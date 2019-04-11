//#define LOG_NDEBUG 0

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "../hwc.h"
#include <hardware/hardware.h>
#include <hardware/hwcomposer2.h>
#include <cutils/log.h>
#include <system/graphics.h>
#include <cutils/list.h>
#include <stdlib.h>
#include <linux/ion.h>
#include <ion/ion.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <EGL/egl.h>
#include <hardware/hal_public.h>
#include <pthread.h>
#include <hardware/sunxi_display2.h>
#include <hardware/sunxi_metadata_def.h>
#include <sys/resource.h>
#include <utils/Trace.h>
#include <cutils/properties.h>
#include <ui/GraphicBuffer.h>
#include "../dev_composer2.h"

#define ION_IOC_SUNXI_PHYS_ADDR 7
#define HAL_PIXEL_FORMAT_AW_NV12 0x101
#define HAL_PIXEL_FORMAT_BGRX_8888 0x1ff
#define YV12_ALIGN 16
#define ROTATE_ALIGN 32
#define ALIGN(x,a)      (((x) + (a) - 1L) & ~((a) - 1L))

#define PIPE_NUM 4
#define DE_NUM 2
#define LAYER_BY_PIPE 4
//#define FORCE_GPU
//#define AFBC_DEBUG
#define VI_NUM 1

extern DisplayOpr de2DisplayOpr;

typedef struct {
    ion_user_handle_t handle;
    unsigned int phys_addr;
    unsigned int size;
}sunxi_phys_data;


typedef enum DeType {
    VIDEO,
    UI,
} DeType;

typedef struct Spipe {
    bool reserved;
    bool used;
    int layNum;
    DeType type;
    Layer *lay[LAYER_BY_PIPE];
    float scaleW[LAYER_BY_PIPE];
    float scaleH[LAYER_BY_PIPE];
    struct listnode areaHead;
    struct listnode sPNode;
} Spipe;

typedef struct AreaList {
    struct hwc_rect area;
    struct listnode node;
} AreaList;

typedef struct HardWareVar {
    /* w,h changed when switching display mode */
    int width;
    int height;
    bool vsyncEn;
    disp_tv_mode mode;
} HardWareVar;

typedef struct HardWareFix {
    /* w,h were setted when starting system */
    int width;
    int height;
    int vsyncPeriod;
    unsigned int dpiX;
    unsigned int dpiY;
    int ionFd;
    int dispFd;
    int fbFd;
    int trFd;
    int tdFd;
    int pipeNum;
    unsigned char persentWT;
    unsigned char persentHT;
    unsigned char persentW;//default is 100
    unsigned char persentH;//default is 100
} HardWareFix;

typedef struct Pipe {
    int spipe;
    disp_layer_config2 layInfo[LAYER_BY_PIPE];
} Pipe;

typedef struct LayerWork{
    disp_layer_config2 *config;
    int num;
    int32_t *acqFence;
    int layerSize;
    int releaseFenceFd;
    struct listnode node;
} layerWork;

typedef struct hwThread {
    int start;
    pthread_t pid;
    struct listnode workHead;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} hwThread;

/* init pipe need init pipe layer. layer_id, channel_id. */
typedef struct HwSource2 {
    HardWareVar var;
    HardWareFix fix;
    Pipe pipe[PIPE_NUM];
    Spipe sPipe[PIPE_NUM];
    struct listnode sPipeHead;
    struct listnode layHead;
    struct hwThread threadHead;
} HwSource2;

HwSource2 hwSource2[DE_NUM];

static int check_sunxi_metadata_info(Display *display, private_handle_t *handle)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    HardWareFix *fix = &hw->fix;

    if (NULL != handle) {
#if GRALLOC_SUNXI_METADATA_BUF
        private_handle_t *hdl = (private_handle_t *)handle;
        int ret = 0;

        ion_user_handle_t ion_handle;
        unsigned char *map_ptr = NULL;
        int map_fd = -1;
        unsigned int flag = 0;

        if (0 == hdl->ion_metadata_size) {
            ALOGW("ion_metadata_size is 0");
            return 0;
        }

        ret = ion_import(fix->ionFd, hdl->metadata_fd, &ion_handle);
        if (ret) {
            ALOGE("ion import failed, ret=%d, metadata_fd=%d!",
                ret, hdl->metadata_fd);
            return 0;
        }
        ret = ion_map(fix->ionFd, ion_handle, hdl->ion_metadata_size,
            PROT_READ, MAP_SHARED, 0, &map_ptr, &map_fd);
        if (ret) {
            ALOGE("ion_map failed, ret=%d\n", ret);
            if (0 <= map_fd)
                close(map_fd);
            ion_free(fix->ionFd, ion_handle);
            return 0;
        }

        struct sunxi_metadata *ptr = (struct sunxi_metadata *)map_ptr;
        flag = ptr->flag;

        struct afbc_header *p = &(ptr->afbc_head);
        ALOGV("&&&&& afbc header:");
        ALOGV("%u,%u,%u,%u;\n"
            "%u,%u,%u,%u;\n"
            "%u,%u,%u,%u;\n"
            "%u,%u,%u,%u\n"
            "%u,%u,%u @@@.",
            p->signature, p->filehdr_size, p->version, p->body_size,
            p->ncomponents, p->header_layout, p->yuv_transform, p->block_split,
            p->inputbits[0], p->inputbits[1], p->inputbits[2], p->inputbits[3],
            p->block_width, p->block_height, p->width, p->height,
            p->left_crop, p->top_crop, p->block_layout);

        ret = munmap(map_ptr, hdl->ion_metadata_size);
        if (0 != ret) {
            ALOGE("munmap sunxi metadata failed ! ret=%d", ret);
        }
        close(map_fd);
        ion_free(fix->ionFd, ion_handle);
        return flag;
#endif
    } else {
        ALOGE("[%s] handle is NULL !!!", __FUNCTION__);
    }
    return 0;
}


static bool layerIsBlended(Layer *lay)
{
    return (lay->blendMode != HWC2_BLEND_MODE_INVALID
            || lay->blendMode != HWC2_BLEND_MODE_NONE);
}

static bool layerIsTransform(Layer *lay)
{
    if (lay->transform > 0) {
        return true;
    }

    return false;
}

static bool layerIsPremult(Layer *lay)
{
    return (lay->blendMode == HWC2_BLEND_MODE_PREMULTIPLIED);
}

static bool isVideoLay(struct Layer *lay)
{
    private_handle_t *handle = NULL;

    handle = (private_handle_t *)lay->buffer;

    if (handle == NULL) {
        return 0;
    }
    if (lay->compositionType == HWC2_COMPOSITION_SOLID_COLOR) {
        /* Buffer handle is null */
        return 0;
    }

    switch (handle->format) {

    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
    case HAL_PIXEL_FORMAT_AW_NV12:
        return 1;
    default:
        return 0;
    }
}

static bool isSameForamt(struct Layer *lay1, struct Layer *lay2)
{/* note!!! */
    private_handle_t *handle1 = NULL;
    private_handle_t *handle2 = NULL;
    handle1 = (private_handle_t *)lay1->buffer;
    handle2 = (private_handle_t *)lay2->buffer;

    if (isVideoLay(lay1) != isVideoLay(lay2)) {
        return false;
    }
    if (isVideoLay(lay1)) {
        /* Video pipe need the same yuv format,
         * Rgb format no need the same.
        */
    if (handle1->format != handle2->format) {
            return false;
        }
    }

    return true;
}

static int isAfbcBuf(private_handle_t *handle)
{
    int metadata_fd;
    unsigned int flag;

    if (NULL != handle) {
#if GRALLOC_SUNXI_METADATA_BUF
        metadata_fd = handle->metadata_fd;
        flag = handle->ion_metadata_flag;
        if ((0 <= metadata_fd) &&
                (flag & SUNXI_METADATA_FLAG_AFBC_HEADER)) {
            ALOGV("metadata_fd = %d, metadata_flag = %d !\n",metadata_fd, flag);
            return 1;
        }
#endif
    }
    return 0;
}

static bool needVideoPipe(Display *display)
{
    struct listnode *node;
    struct Layer *lay;
    int viLay = 0;
    private_handle_t *handle = NULL;

    list_for_each(node, &display->layerSortedByZorder) {
        lay = node_to_item(node, Layer, node);
        if (isVideoLay(lay)) {
            return true;
        }
    }
    return false;
}

static void initHwPipe(HwSource2 *hw)
{
    int i = 0, j = 0;

    /* init pipe, channel id, layer id */
    memset(&hw->pipe[0], 0, PIPE_NUM * sizeof(Pipe));

    for (i = 0; i < PIPE_NUM; i++) {
        hw->pipe[i].spipe = -1;
        for (j = 0; j < LAYER_BY_PIPE; j++) {
            hw->pipe[i].layInfo[j].channel = i;
            hw->pipe[i].layInfo[j].layer_id = j;
        }
    }
}

static void initHwSpipe(HwSource2 *hw)
{
    int i = 0;

    memset(&hw->sPipe[0], 0, PIPE_NUM * sizeof(Spipe));
    for (i = 0; i < PIPE_NUM; i++) {
        if (i < VI_NUM) {
            hw->sPipe[i].type = VIDEO;
        } else {
            hw->sPipe[i].type = UI;
        }
        list_init(&hw->sPipe[i].areaHead);
        list_init(&hw->sPipe[i].sPNode);
    }

    list_init(&hw->sPipeHead);
    list_init(&hw->layHead);
}

static void resetHwSpipe(HwSource2 *hw)
{
    struct listnode *node;
    int i = 0;

    /* free area list. */
    for (i = 0; i < PIPE_NUM; i++) {
        while (!list_empty(&hw->sPipe[i].areaHead)) {
            node = list_head(&hw->sPipe[i].areaHead);
            list_remove(node);
            free(node_to_item(node, struct AreaList, node));
        }
    }

    /* free spipe list. */
    while (!list_empty(&hw->sPipeHead)) {
        node = list_head(&hw->sPipeHead);
        list_remove(node);
        list_init(node);
    }
    list_init(&hw->sPipeHead);

    /* destory back layer list. */
    while (!list_empty(&hw->layHead)) {
        node = list_head(&hw->layHead);
        list_remove(node);
        free(node_to_item(node, Layer, node));
    }
    list_init(&hw->layHead);

    /* clean data. */
    memset(&hw->sPipe[0], 0, PIPE_NUM * sizeof(Spipe));
    for (i = 0; i < PIPE_NUM; i++) {
        if (i < VI_NUM) {
            hw->sPipe[i].type = VIDEO;
        } else {
            hw->sPipe[i].type = UI;
        }
        list_init(&hw->sPipe[i].areaHead);
        list_init(&hw->sPipe[i].sPNode);
    }
}

static Spipe *getUiSpipe(HwSource2 *hw)
{
    int i = 0;
    Spipe *pi = NULL;

    for (i = 0; i < hw->fix.pipeNum; i++) {
        if (hw->sPipe[i].used || hw->sPipe[i].reserved) {
            continue;
        }
        if (hw->sPipe[i].type == UI) {
            pi = &hw->sPipe[i];
            pi->used = true;
            list_add_tail(&hw->sPipeHead, &pi->sPNode);
            break;
        }
    }
    return pi;
}

static Spipe *getViSpipe(HwSource2 *hw)
{
    int i = 0;
    Spipe *pi = NULL;

    for (i = 0; i < hw->fix.pipeNum; i++) {
        if (hw->sPipe[i].used || hw->sPipe[i].reserved) {
            continue;
        }
        if (hw->sPipe[i].type == VIDEO) {
            pi = &hw->sPipe[i];
            pi->used = true;
            list_add_tail(&hw->sPipeHead, &pi->sPNode);
            break;
        }
    }
    return pi;
}

static void showSpipeInfo(HwSource2 *hw)
{
    int i = 0;
    for (i = 0; i < PIPE_NUM; i++) {
        ALOGV("Dbg:# spipe=%d, used=%d, reserved=%d, type=%d, layernum=%d!\n"
            ,i, hw->sPipe[i].used, hw->sPipe[i].reserved,
            hw->sPipe[i].type, hw->sPipe[i].layNum);
    }
}

static Spipe *getAnySpipe(HwSource2 *hw)
{
    int i = 0;
    Spipe *pi = NULL;

    for (i = 0; i < hw->fix.pipeNum; i++) {
        if (!hw->sPipe[i].used && !hw->sPipe[i].reserved) {
            pi = &hw->sPipe[i];
            pi->used = true;
            list_add_tail(&hw->sPipeHead, &pi->sPNode);
            break;
        }
    }

    if (pi == NULL) {
            ALOGV("getAnySpipe failed!\n");
    }
    return pi;
}

static Spipe *getSpipe(HwSource2 *hw, bool haveVideo)
{
    if (haveVideo) {
        return getUiSpipe(hw);
    } else {
        return getAnySpipe(hw);
    }
}

static Spipe *getResSpipe(HwSource2 *hw)
{
    int i = 0;
    Spipe *pi = NULL;

    for (i = 0; i < hw->fix.pipeNum; i++) {
        if (hw->sPipe[i].reserved) {
            pi = &hw->sPipe[i];
            if (list_empty(&pi->sPNode)) {
                /* add list only once. */
                list_add_tail(&hw->sPipeHead, &pi->sPNode);
            }
            break;
        }
    }
    if (pi == NULL) {
        ALOGE("There is no reserved pipe, something was wrong!\n");
    }
    return pi;
}

static int reserveSpipe(HwSource2 *hw)
{
    int i = 0;
    Spipe *pi = NULL;


    for (i = 0; i < hw->fix.pipeNum; i++) {
        if (hw->sPipe[i].reserved) {
            ALOGE("Have already reserved pipe, something was wrong!\n");
            return -1;
        }
    }

    for (i = 0; i < hw->fix.pipeNum; i++) {
        /* can only reserve ui pipe for gpu */
        if (!hw->sPipe[i].used && hw->sPipe[i].type == UI) {
            pi = &hw->sPipe[i];
            hw->sPipe[i].reserved = true;
            break;
        }
    }
    if (pi == NULL) {
        ALOGE("No more pipe to reserve, something was wrong!\n");
        return -1;
    }

    return 0;
}

static bool isLayerOverlap(hwc_rect *areaList, hwc_rect *area)
{
    hwc_rect *left_area;
    hwc_rect *right_area;
    hwc_rect *top_area;
    hwc_rect *bottom_area;

    if (area->left > areaList->left) {
        right_area = area;
        left_area = areaList;
    } else {
        right_area = areaList;
        left_area = area;
    }

    /* x not overlap, two area could not be overlay,
     * if equal,is also not overlap.
    */
    if (right_area->left >= left_area->right) {
        return false;
    }

    if (area->top > areaList->top) {
        top_area = area;
        bottom_area = areaList;
    } else {
        top_area = areaList;
        bottom_area = area;
    }

    /* y not overlap, two area could not be overlay */
    if (top_area->top >= bottom_area->bottom)
        return false;

    return true;
}

static bool checkOverlap(Layer *lay, Spipe *pipe)
{
    /* judge current pipe list.*/
    /* judge last pipe list. */
    bool state = false;
    int i;
    struct listnode *node;
    struct AreaList *area;

    if (list_empty(&pipe->areaHead)) {
        return false;
    }

    list_for_each(node, &pipe->areaHead) {
       area = node_to_item(node, AreaList, node);
       if (isLayerOverlap(&area->area, &lay->frame))
           return true;
    }

    return false;
}

static bool checkFormatByPipe(Layer *lay, Spipe *pipe)
{
    int i;
    bool state;

    for (i = 0; i < LAYER_BY_PIPE; i++) {
        if (pipe->lay[i]) {
            if (!isSameForamt(lay, pipe->lay[i]))
                return false;
        }
    }
    return true;
}

static bool isSpipeFull(Spipe *pipe)
{
    int i;

    for (i = 0; i < LAYER_BY_PIPE; i++) {
        if (pipe->lay[i] == NULL) {
            return false;
        }
    }
    return true;
}

static bool checkScale(Layer *lay, Spipe *pipe)
{
    /* judge if the pipe support this scale factor.
     * judge the pipe have capability of scaler.
    */
    float sw, sh, dw, dh;
    float factorW, factorH;
    int i;

    sw = (float)(lay->frame.right - lay->frame.left);
    sh = (float)(lay->frame.bottom - lay->frame.top);

    dw = lay->crop.right - lay->crop.left;
    dh = lay->crop.bottom - lay->crop.top;

    factorW = dw / sw;
    factorH = dh / sh;

    for (i = 0; i < LAYER_BY_PIPE; i++) {
        if (pipe->scaleW[i] == 0 || pipe->scaleH[i] == 0) {
            /* scaler is init status, no scaler. */
            continue;
        }

        dw = factorW - pipe->scaleW[i];
        dh = factorH - pipe->scaleH[i];
        if ((dw > -0.009 && dw < 0.009)
            && (dh > -0.009 && dh < 0.009)) {
            continue;
        } else {
            return false;
        }
    }

    return true;
}

static bool isScaleUI(Layer *lay)
{
    float sw, sh, dw, dh;
    float W, H;
    sw = (float)(lay->frame.right - lay->frame.left);
    sh = (float)(lay->frame.bottom - lay->frame.top);
    dw = lay->crop.right - lay->crop.left;
    dh = lay->crop.bottom - lay->crop.top;
    W = dw - sw;
    H = dh - sh;
    if (isVideoLay(lay)) {
        return false;
    }
    if ( W > -0.009 && W < 0.009 && H > -0.009 && H < 0.009) {
        return false;
    }
    return true;
}

static bool checkPipeIsFull(Spipe *pipe)
{
    if (pipe->layNum > LAYER_BY_PIPE) {
        return true;
    }
    return false;
}

/* TODO: opipe for up. */
static int checkInsert(Layer *lay, Spipe *pipe, Spipe *opipe)
{
    /* 1. judge is the proper format.
     * 2. judge isoverlap.
     * 3. judge last pipe isoverlap.
     * 3. judge is ablending?
     * 4. return the insert position.
    */
    int i;

    if (pipe == NULL) {
        /* pipe is null. */
        return -1;
    }

    if(lay->skipFlag) {
        return -1;
    }

    if (lay->compositionType == HWC2_COMPOSITION_SOLID_COLOR) {
        return -1;
    }

    if (layerIsTransform(lay)) {
        return -1;
    }

    private_handle_t *handle = (private_handle_t *)lay->buffer;
    if (handle) {
        if (!(handle->flags & SUNXI_MEM_CONTIGUOUS)) {
            ALOGV("%s: LINE:%d try filed, Not SUNXI_MEM_CONTIGUOUS!\n", __func__, __LINE__);
            return -1;
        }
    }

    if (checkOverlap(lay, pipe) && layerIsBlended(lay)) {
        return -1;
    }

    if (checkPipeIsFull(pipe)) {
        return -1;
    }

    if (!checkFormatByPipe(lay, pipe)) {
        return -1;
    }

    if (!checkScale(lay, pipe)) {
        return -1;
    }

    if (isScaleUI(lay)) {
        return -1;
    }

    for (i = 0; i < LAYER_BY_PIPE; i++) {
        if(pipe->lay[i] == NULL) {
            /* return insert layer position.
             * if is full, return -1.
             */
            return i;
        }
    }
    /* pipe is full. */
    return -1;
}

static int checkReverseInsert(Layer *lay, Spipe *pipe)
{
    /* 1. judge is the proper format.
     * 2. judge isoverlap?
     * 3. judge is ablending?
     * 4. return the insert position.
    */
    int i;

    if (pipe->reserved) {
        /* reserved pipe. */
        if (layerIsBlended(lay)
            || !checkFormatByPipe(lay, pipe)
            || !checkScale(lay, pipe)) {
            /* do not need judge overlap. */
            return -1;
        }
        for (i = LAYER_BY_PIPE - 1; i >= 0 ; i--) {
            if(pipe->lay[i] == NULL) {
                /* return insert layer position. */
                return i;
            }
        }
    } else {
        /* normal pipe. */
        if (!checkFormatByPipe(lay, pipe)) {
            return -1;
        }

        if (!checkScale(lay, pipe)) {
            return -1;
        }

        if (checkOverlap(lay, pipe) && layerIsBlended(lay)) {
            return -1;
        }

        for (i = 0; i < LAYER_BY_PIPE; i++) {
            if(pipe->lay[i] == NULL) {
                /* return insert layer position. */
                return i;
            }
        }
    }

    /* pipe is full. */
    return -1;
}

static void creatArea(struct listnode *node, hwc_rect_t *area)
{
    AreaList *al;
    al = (AreaList *)malloc(sizeof(AreaList));
    al->area.top = area->top;
    al->area.left = area->left;
    al->area.right = area->right;
    al->area.bottom = area->bottom;
    list_init(&al->node);
    list_add_tail(node, &al->node);
}

static void calcScale(int pos, Spipe *pipe)
{
    float sw, sh, dw, dh;

    sw = (float)(pipe->lay[pos]->frame.right - pipe->lay[pos]->frame.left);
    sh = (float)(pipe->lay[pos]->frame.bottom - pipe->lay[pos]->frame.top);

    dw = pipe->lay[pos]->crop.right - pipe->lay[pos]->crop.left;
    dh = pipe->lay[pos]->crop.bottom - pipe->lay[pos]->crop.top;

    pipe->scaleW[pos] = dw / sw;
    pipe->scaleW[pos] = dh / sh;
}

/* insert to current pipe now
 * TODO: insert to last pipe in the future.
*/
static bool insertLayToSpipe(Layer *lay, Spipe *pipe)
{
    /* 1. add lay.
     * 2. add area to list.
     * 3. up insert to another pipe.(need a pipe list)
     */
    int i;
    AreaList *al;
    Spipe *opipe = NULL;

    /* TODO: check overlap first in the future...to judge up.
     * checkOverlap(lay, pipe);checkFormatByPipe(lay, pipe)
    */

    i = checkInsert(lay, pipe, opipe);
    if (i < 0) {
        return false;
    }

    pipe->lay[i] = lay;
    calcScale(i, pipe);
    creatArea(&pipe->areaHead, &lay->frame);
    if (pipe->layNum >= LAYER_BY_PIPE) {
        ALOGE("%s: insert wrong, pipe is full, something was wrong.\n", __func__);
        return false;
    }
    pipe->layNum++;

    return true;
}

static bool reverseInsertLayToSpipe(Layer *lay, Spipe *pipe)
{
    /* 1. add lay.
     * 2. add area to list.
     */
    int i;
    AreaList *al;

    i = checkReverseInsert(lay, pipe);
    if (i < 0) {
        return false;
    }

    pipe->lay[i] = lay;
    calcScale(i, pipe);
    creatArea(&pipe->areaHead, &lay->frame);

    return true;
}

static int isFbTarget(int type)
{
    return (type == HWC2_COMPOSITION_CLIENT_TARGET);
}

static int isTypeChange(Layer *lay, int type)
{
    return (lay->compositionType != type);
}

static void insertFbToSpipe(Display *display, Spipe *pipe)
{
    /* 1. find fbtarget.
     * 2. insert to current spipe's first layer.
     */
    struct listnode *node;
    struct Layer *lay;

    list_for_each(node, &display->layerSortedByZorder) {
        lay = node_to_item(node, Layer, node);
        /* find fbtarget buffer */
        if (lay->compositionType == HWC2_COMPOSITION_CLIENT_TARGET) {
            break;
        }
    }

    pipe->lay[0] = lay;
    /* need calc factor? */
    /* need turn to device client? */
    creatArea(&pipe->areaHead, &lay->frame);
    pipe->layNum++;
}

static void createLaylist(Display *display)
{
    Layer *lay, *layTemp;
    struct listnode *head;
    struct listnode *node;
    HwSource2 *hw = &hwSource2[display->displayId];

    list_for_each(node, &display->layerSortedByZorder) {
        lay = node_to_item(node, Layer, node);
        layTemp = (Layer *)malloc(sizeof(Layer));
        memset(layTemp, 0, sizeof(Layer));
        memcpy(layTemp, lay, sizeof(Layer));
        list_init(&layTemp->node);
        list_add_tail(&hw->layHead, &layTemp->node);
    }
}

static void changeCompositionType(Layer *lay, int type)
{
    if (lay->compositionType == HWC2_COMPOSITION_CLIENT_TARGET) {
        /* Nothing todo with Fbtarget. */
        return;
    }

    lay->typeChange = isTypeChange(lay, type);
    lay->compositionType = type;
}

static char get3dMode(HwSource2 *hw)
{
    char mode = 0;
    int tdFd = 0;

    if (hw->fix.tdFd >= 0) {
        lseek(hw->fix.tdFd, 0, SEEK_SET);
        char data = 0;
        ssize_t ret = 0;
        ret = read(hw->fix.tdFd, &data, 1);
        if (ret < 0) {
            ALOGE("###read operate_3d_mode fail, err=%d!", errno);
        }
        mode = data - '0';
        return mode;
    } else {
            ALOGE("###open operate_3d_mode fail,can't read.");
    }
    return 0;
}

int tryToAssign(Display *display, bool tryFlag)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    struct listnode *node;
    struct listnode *list;
    struct Layer *lay;
    struct Spipe *pipe = NULL;
    bool haveVi = needVideoPipe(display);
    float mBrandwidth = 0;
    static int framecount = 0;

    if (tryFlag) {
        createLaylist(display);
        list = &hw->layHead;
    } else {
        list = &display->layerSortedByZorder;
    }

    if(display->colorTransformHint) {
        // GPU process.
        return -2;
    }

    if (tryFlag) {
        list_for_each(node, list) {
            lay = node_to_item(node, Layer, node);
            if (lay->compositionType == HWC2_COMPOSITION_CLIENT_TARGET) {
                continue;
            }
            if (isVideoLay(lay)) {
                mBrandwidth = mBrandwidth + (float)(lay->crop.right - lay->crop.left) * (float)(lay->crop.bottom - lay->crop.top) * 1.5 * 60 / 1024 / 1024;
            } else {
                mBrandwidth = mBrandwidth + (float)(lay->crop.right - lay->crop.left) * (float)(lay->crop.bottom - lay->crop.top) * 4 * 60 / 1024 / 1024;
            }
            if (mBrandwidth >2200.0) {
                // the future 60 frames use gpu assagin.
                framecount = 60;
                return -2;
            }
        }
        if ((framecount--) > 0) {
            return -2;
        }
        framecount = 0;
    }

    list_for_each(node, list) {
        lay = node_to_item(node, Layer, node);
        if (lay->compositionType == HWC2_COMPOSITION_CLIENT_TARGET) {
            continue;
        }
        if (!insertLayToSpipe(lay, pipe)) {
            /* current pipe can not insert. */
            if (lay->compositionType == HWC2_COMPOSITION_SOLID_COLOR) {
                ALOGV("%s: LINE:%d try filed, solid color!\n", __func__, __LINE__);
                return -1;
            }

            if (layerIsTransform(lay)) {
                ALOGV("%s: LINE:%d try filed, transform!\n", __func__, __LINE__);
                return -1;
            }

            if (isScaleUI(lay)) {
                ALOGV("%s: LINE:%d try filed, scale ui!\n", __func__, __LINE__);
                return -1;
            }

            if(lay->skipFlag) {
                ALOGV("%s: LINE:%d try filed, skip!\n", __func__, __LINE__);
                return -1;
            }

            private_handle_t *handle = (private_handle_t *)lay->buffer;
            if (handle) {
                if (!(handle->flags & SUNXI_MEM_CONTIGUOUS)) {
                ALOGV("%s: LINE:%d try filed, Not SUNXI_MEM_CONTIGUOUS!\n", __func__, __LINE__);
                return -1;
                }
            }

            if (isVideoLay(lay)) {
                /* video layer*/
                pipe = getViSpipe(hw);
                if (pipe == NULL) {
                    /* have no more pipe. */
                    ALOGV("%s: LINE:%d try filed, have no more video pipe!\n", __func__, __LINE__);
                    return -1;
                } else {
                    insertLayToSpipe(lay, pipe);
                    changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
                }
            } else {
                /* ui layer*/
                pipe = getSpipe(hw, haveVi);
                if (pipe == NULL) {
                    /* have no more pipe. */
                    ALOGV("%s: LINE:%d try filed, have no more ui pipe, haveVi=%d!\n",
                            __func__, __LINE__, haveVi);
                    return -1;
                } else {
                    insertLayToSpipe(lay, pipe);
                    changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
                }
            }
        } else {
            /* insert ok, use hardware. */
            changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
        }
    }
    return 0;
}

static bool setAssign(HwSource2 *hw, struct Layer *lay, struct Spipe *pipe)
{
    bool mergeLayer = false;

    if (pipe == NULL) {
        /* try to insert to reserve pipe. */
        if(reverseInsertLayToSpipe(lay, getResSpipe(hw))) {
            /* insert reserve pipe success. */
            changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
        } else {
            /* use gpu. */
            changeCompositionType(lay, HWC2_COMPOSITION_CLIENT);
            mergeLayer = true;
        }
    } else {
        /* insert to new pipe. */
        reverseInsertLayToSpipe(lay, pipe);/* insert to new pipe, no need judge. */
        changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
    }
    return mergeLayer;
}

static int layerAssign(Display *display)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    struct listnode *node;
    struct Layer *lay;
    struct Spipe *pipe = NULL;
    int lcnt = 0, layNum = 0, rlcnt = 0;
    bool haveVi = needVideoPipe(display);
    bool mergeLayer = false;

    layNum = sizeList(&display->layerSortedByZorder);
    list_for_each(node, &display->layerSortedByZorder) {
        lay = node_to_item(node, Layer, node);
        if (lay->compositionType == HWC2_COMPOSITION_CLIENT_TARGET) {
            continue;
        }
        if (!insertLayToSpipe(lay, pipe)) {
            /* current pipe can not insert. */
            private_handle_t *handle = (private_handle_t *)lay->buffer;
            bool flag = false;
            if (handle) {
                if (!(handle->flags & SUNXI_MEM_CONTIGUOUS)) {
                    flag = true;
                }
            }

            if (lay->compositionType == HWC2_COMPOSITION_SOLID_COLOR
                || layerIsTransform(lay) || lay->skipFlag || isScaleUI(lay) || flag) {
                changeCompositionType(lay, HWC2_COMPOSITION_CLIENT);
                lcnt++; //fix
                break;
            }

            if (isVideoLay(lay)) {
                /* video layer*/
                pipe = getViSpipe(hw);
                if (pipe == NULL) {
                    changeCompositionType(lay, HWC2_COMPOSITION_CLIENT);
                    lcnt++; //fix
                    break;
                } else {
                    insertLayToSpipe(lay, pipe);
                    changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
                    lcnt++;
                }
            } else {
                /* ui layer*/
                pipe = getSpipe(hw, haveVi);
                if (pipe == NULL) {
                    changeCompositionType(lay, HWC2_COMPOSITION_CLIENT);
                    lcnt++; //fix
                    break;
                } else {
                    insertLayToSpipe(lay, pipe);
                    changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
                    lcnt++;
                }
            }
        } else {
            /* insert ok. */
            changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
            lcnt++;
        }
    }

#ifndef REVERSE_ASSIGN
    /* method 1. lazzy assign. the rest layers use gpu */
    /* all reset layers assign to fbtarget. */
    mergeLayer = true;

#endif
/***************************************************/
    /* method 2. reverse assign. */
    /* assign fbtarget first. */
    //showSpipeInfo(hw);
    pipe = getResSpipe(hw);
    insertFbToSpipe(display, pipe);

    rlcnt = layNum - lcnt - 1; //fix: layNum include fbtarget.
    if (rlcnt == 0) {
        ALOGV("%s: LINE:%d default assign ok!\n", __func__, __LINE__);
        return 0;
    }

    pipe = NULL;
    list_for_each_reverse(node, &display->layerSortedByZorder) {
            lay = node_to_item(node, Layer, node);
            if (lay->compositionType == HWC2_COMPOSITION_CLIENT_TARGET) {
                continue;
            }

            if (mergeLayer) {
                /* merge the reset layers to fbtarget. */
                changeCompositionType(lay, HWC2_COMPOSITION_CLIENT);
                goto goon;
            }
            if (pipe) {
                /* already get one pipe, reverse insert. */
                if (reverseInsertLayToSpipe(lay, pipe)) {
                    changeCompositionType(lay, HWC2_COMPOSITION_DEVICE);
                    goto goon;
                }
            }
            if (isVideoLay(lay)) {
                /* video layer*/
                pipe = getViSpipe(hw);
                mergeLayer = setAssign(hw, lay, pipe);
            } else {
                /* ui layer*/
                /* this haveVi may retry again...*/
                pipe = getSpipe(hw, haveVi);
                mergeLayer = setAssign(hw, lay, pipe);
            }
goon:
            rlcnt--;
            if (!rlcnt) {
                /* the rest layers assign finish. */
                break;
            }
    }
    //showSpipeInfo(hw);
    return 0;

}

static void forceGPUAssign(Display *display)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    struct listnode *node;
    struct Layer *lay;
    struct Layer *fbLay;
    struct Spipe *pipe = NULL;
    int i = 0;

    list_for_each(node, &display->layerSortedByZorder) {
        lay = node_to_item(node, Layer, node);
        //showLayer(lay);
        if (lay->compositionType != HWC2_COMPOSITION_CLIENT_TARGET) {
            changeCompositionType(lay, HWC2_COMPOSITION_CLIENT);
            i++;
            continue;
        }
        fbLay = lay;
    }
    if (i > 0) {
        /* there are more than one layer, include FBTarget layer. */
        ALOGV("%s: have more than one layers.\n", __func__);
        pipe = getSpipe(hw, true);
        if (pipe == NULL) {
             ALOGE("%s: get pipe failed.\n", __func__);
        }
        insertLayToSpipe(fbLay, pipe);
    }
}

static int defaultAssign(Display *display)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    int ret = 0;
    //showSpipeInfo(hw);

    //ALOGD("%d: before assign size=%d\n",
    //      display->displayId, sizeList(&display->layerSortedByZorder));

    ret = tryToAssign(display, true);
    if (ret == -1) {
        //showSpipeInfo(hw);
        /* try failed, reserve one pipe. */
        resetHwSpipe(hw);
        if (reserveSpipe(hw)) {
            ALOGE("%s: reserveSpipe failed.\n", __func__);
        }

        /* add spipe print info. */
        if (layerAssign(display)) {
             ALOGE("%s: lazzy assign failed, something was wrong.\n", __func__);
        }
        return 1;
    } else if (ret == -2) {
        /* too many layers */
        resetHwSpipe(hw);
        forceGPUAssign(display);
        return 1;
    } else {
        /* try success. */
        resetHwSpipe(hw);
        tryToAssign(display, false);
        return 0;
    }

}

unsigned int ionGetAddr(HwSource2 *hw, int sharefd)
{
    int ret = -1;
    struct ion_custom_data custom_data;
    sunxi_phys_data phys_data;
    ion_handle_data freedata;
    struct ion_fd_data data ;

    data.fd = sharefd;
    ret = ioctl(hw->fix.ionFd, ION_IOC_IMPORT, &data);
    if (ret < 0) {
        ALOGE("%s: ION_IOC_IMPORT failed(ret=%d)", __func__, ret);
        return 0;
    }
    custom_data.cmd = ION_IOC_SUNXI_PHYS_ADDR;
    phys_data.handle = data.handle;
    custom_data.arg = (unsigned long)&phys_data;
    ret = ioctl(hw->fix.ionFd, ION_IOC_CUSTOM, &custom_data);
    if(ret < 0) {
        ALOGE("%s: ION_IOC_CUSTOM failed(ret=%d)", __func__, ret);
        return 0;
    }
    freedata.handle = data.handle;
    ret = ioctl(hw->fix.ionFd, ION_IOC_FREE, &freedata);
    if(ret < 0) {
        ALOGE("%s: ION_IOC_FREE failed(ret=%d)", __func__, ret);
        return 0;
    }
    return phys_data.phys_addr;
}

static void showLayInfo(disp_layer_config2 *config)
{
    /* for test. */
    static char buf[256] = {0};
    int count = 0;
    char property[PROPERTY_VALUE_MAX];
    int fpsFlag = 0;

    count = 0;
    count += sprintf(buf + count, " %5s ", (config->info.mode == LAYER_MODE_BUFFER)? "BUF":"COLOR");
    count += sprintf(buf + count, " %8s ", (config->enable==1)?"enable":"disable");
    count += sprintf(buf + count, "ch[%1d] ", config->channel);
    count += sprintf(buf + count, "lyr[%1d] ", config->layer_id);
    count += sprintf(buf + count, "z[%1d] ", config->info.zorder);
    count += sprintf(buf + count, "prem[%1s] ", (config->info.fb.pre_multiply)? "Y":"N");
    count += sprintf(buf + count, "a[%5s %3d] ", (config->info.alpha_mode)? "globl":"pixel", config->info.alpha_value);
    count += sprintf(buf + count, "fmt[%3d] ", config->info.fb.format);
    count += sprintf(buf + count, "fb[%4d,%4d;%4d,%4d;%4d,%4d] ", config->info.fb.size[0].width, config->info.fb.size[0].height,
        config->info.fb.size[0].width, config->info.fb.size[0].height,config->info.fb.size[0].width, config->info.fb.size[0].height);
    count += sprintf(buf + count, "crop[%4d,%4d,%4d,%4d] ", (unsigned int)(config->info.fb.crop.x>>32), (unsigned int)(config->info.fb.crop.y>>32),
        (unsigned int)(config->info.fb.crop.width>>32), (unsigned int)(config->info.fb.crop.height>>32));
    count += sprintf(buf + count, "ali0[%d] ali1[%d] ali[%d]", config->info.fb.align[0], config->info.fb.align[1]
                    , config->info.fb.align[2]);
    count += sprintf(buf + count, "frame[%4d,%4d,%4d,%4d] ", config->info.screen_win.x, config->info.screen_win.y, config->info.screen_win.width, config->info.screen_win.height);
    //count += sprintf(buf + count, "addr[%8llx,%8llx,%8llx] ", config->info.fb.addr[0], config->info.fb.addr[1], config->info.fb.addr[2]);
    count += sprintf(buf + count, "flags[0x%8x] trd[%1d,%1d]\n", config->info.fb.flags, config->info.b_trd_out, config->info.out_trd_mode);

    if (property_get("debug.hwc.showfps", property, NULL) >= 0) {
        fpsFlag = atoi(property);
    }
    if (fpsFlag == 2) {
        ALOGD("%s", buf);
        memset(buf, 0, 256);
    }
}

static int siwtchDevice(Display *display, int mode)
{
    int ret = 0;
    unsigned long arg[4] = {0};
    HwSource2 *hw = &hwSource2[display->displayId];

    arg[0] = display->displayId;
    arg[1] = DISP_OUTPUT_TYPE_HDMI;
    arg[2] = mode;
    ioctl(hw->fix.dispFd, DISP_DEVICE_SWITCH, (unsigned long)arg);
    return 0;
}

static void operate3dMode(Display *display, disp_layer_info2 *info, int isVideo)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    enum display_3d_mode tdMode = (enum display_3d_mode)get3dMode(hw);
    static int switchDev = 0;

    if (display->displayId <= 0) {
        /* not support. */
        return;
    }

    if (switchDev && tdMode < 3) {
        if (display->displayId > 0) {
            /* switch to normal mode. */
            siwtchDevice(display, DISP_TV_MOD_1080P_24HZ);
            switchDev = 0;
            return;
        }
    }
    if (!isVideo) {
        return;
    }
    if (tdMode >= 3) {
        if (!switchDev) {
            siwtchDevice(display, DISP_TV_MOD_1080P_24HZ_3D_FP);
            switchDev = 1;
        }
    }

    switch (tdMode) {
    case DISPLAY_2D_ORIGINAL:
        break;
    case DISPLAY_2D_LEFT:
        info->b_trd_out = 1;
        info->out_trd_mode = DISP_3D_OUT_MODE_FP;
        break;
    case DISPLAY_2D_TOP:
        info->b_trd_out = 1;
        info->out_trd_mode = DISP_3D_OUT_MODE_FP;
        break;
    case DISPLAY_3D_LEFT_RIGHT_HDMI:
        info->b_trd_out = 1;
        info->out_trd_mode = DISP_3D_OUT_MODE_FP;
        info->screen_win.x = 0;
        info->screen_win.y = 0;
        info->screen_win.width = 1920;
        info->screen_win.height = 1080;
        info->fb.flags = DISP_BF_STEREO_SSH;
        break;
    case DISPLAY_3D_TOP_BOTTOM_HDMI:
        info->b_trd_out = 1;
        info->out_trd_mode = DISP_3D_OUT_MODE_FP;
        info->screen_win.x = 0;
        info->screen_win.y = 0;
        info->screen_win.width = 1920;
        info->screen_win.height = 1080;
        info->fb.flags = DISP_BF_STEREO_TB;
        break;
    default:
        break;
    }
}

static int covLayToInfo(Display *display, Layer *lay, disp_layer_config2 *config, int zorder)
{
    float wFactor = 1;
    float hFactor = 1;
    int pNum, bpp, swapUV, is3D;
    int stride = 0, i = 0;
    int pwscale[3], phscale[3];
    disp_layer_info2 *info = &config->info;
    HwSource2 *hw = &hwSource2[display->displayId];

    if (lay->buffer == NULL)
        ALOGE("%s: hw=%p layer->buffer is NULL\n", __FUNCTION__, hw);

    private_handle_t *handle = (private_handle_t *)lay->buffer;//fix private_handle_t

    if (!handle) {
        ALOGE("Img handle is NULL", (private_handle_t *)handle);
        return -1;
    }

    info->fb.fd = handle->share_fd;

    if (!info->fb.fd) {
        ALOGE("%s: LINE:%d fb.fd err.", __func__, __LINE__);
        goto err;
    }

#if GRALLOC_SUNXI_METADATA_BUF
    if (isAfbcBuf(handle)) {
#ifdef AFBC_DEBUG
    check_sunxi_metadata_info(display, handle);
#endif
    info->fb.metadata_fd = handle->metadata_fd;
    info->fb.metadata_size = handle->ion_metadata_size;
    info->fb.metadata_flag = handle->ion_metadata_flag;
    info->fb.fbd_en = 1;
    ALOGV("%s: fb.metadata: fd=%d, size=%d, flag = %d \n",
            __func__, info->fb.metadata_fd, info->fb.metadata_size, info->fb.metadata_flag);
    }
#endif

    config->enable = 1;
    info->zorder = zorder;
    info->fb.align[0] = SUNXI_YUV_ALIGN;

    if (layerIsBlended(lay)) {
        info->alpha_mode = 2;
        info->alpha_value = 0xff;//(unsigned char)(lay->planeAlpha * 255l); //TODO:fix
        int test = 0;
        float a = 255;
        test = (int)(lay->planeAlpha * a);
        ALOGV("%s: planeAlpha=%f, alpha_value=%u, test = %u\n",
            __func__, lay->planeAlpha, (unsigned int)(lay->planeAlpha * (float)255), test);
    } else {
        info->alpha_mode = 1;
        info->alpha_value = 0xff;//TODO: fix
        ALOGV("%s: layerIsnotBlended.info->alpha_value=%d\n", __func__, info->alpha_value);
    }
    if (layerIsPremult(lay)) {
        info->fb.pre_multiply = 1;
    }

    pNum = 1;
    swapUV = 0;
    bpp = 32;
    pwscale[0] = 1;
    phscale[0] = 1;

    switch (handle->format) {
    case HAL_PIXEL_FORMAT_RGBA_8888:
        info->fb.format = DISP_FORMAT_ABGR_8888;
        break;
    case HAL_PIXEL_FORMAT_RGBX_8888:
        info->fb.format = DISP_FORMAT_XBGR_8888;
        break;
    case HAL_PIXEL_FORMAT_BGRA_8888:
        info->fb.format = DISP_FORMAT_ARGB_8888;
        break;
    case HAL_PIXEL_FORMAT_BGRX_8888:
        info->fb.format = DISP_FORMAT_XRGB_8888;
        break;
    case HAL_PIXEL_FORMAT_RGB_888:
        info->fb.format = DISP_FORMAT_BGR_888;
        bpp = 24;
        break;
    case HAL_PIXEL_FORMAT_RGB_565:
        info->fb.align[0] = 32;
        info->fb.format = DISP_FORMAT_RGB_565;
        bpp = 16;
        break;
    case HAL_PIXEL_FORMAT_YV12:
        info->fb.format = DISP_FORMAT_YUV420_P;
        info->fb.align[0] = YV12_ALIGN;
        info->fb.align[1] = YV12_ALIGN / 2;
        info->fb.align[2] = YV12_ALIGN / 2;
        pwscale[1] = 2;
        pwscale[2] = 2;
        phscale[1] = 2;
        phscale[2] = 2;
        bpp = 12;
        pNum = 3;
        swapUV = 1;
        break;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        info->fb.format = DISP_FORMAT_YUV420_SP_VUVU;
        info->fb.align[0] = YV12_ALIGN;
        info->fb.align[1] = YV12_ALIGN / 2;
        bpp = 12;
        pNum = 2;
        pwscale[0] = 1;
        pwscale[1] = 2;
        phscale[0] = 1;
        phscale[1] = 2;
        break;
    case HAL_PIXEL_FORMAT_AW_NV12:
        info->fb.format = DISP_FORMAT_YUV420_SP_UVUV;
        info->fb.align[0] = YV12_ALIGN;
        info->fb.align[1] = YV12_ALIGN / 2;
        pNum = 2;
        bpp = 12;
        pwscale[1] = 2;
        phscale[1] = 2;
        break;
    default:
        ALOGE("DO not support format 0x%x in %s", handle->format, __FUNCTION__);
        goto err;
    }

    stride = ALIGN(handle->width, info->fb.align[0]);

    while (i < pNum) {
        info->fb.size[i].width =  ALIGN(stride / pwscale[i], info->fb.align[i]);

        info->fb.size[i].height = handle->height / phscale[i];
        i++;
    }

    /* config coordinate. */
    //showLayer(lay);
    info->fb.crop.x = ((long long)(lay->crop.left < 0 ? 0 : lay->crop.left) << 32);
    info->fb.crop.y = ((long long)(lay->crop.top < 0 ? 0 : lay->crop.top) << 32);
    info->fb.crop.width = ((long long)(lay->crop.right - lay->crop.left) << 32);
    info->fb.crop.height = ((long long)(lay->crop.bottom - lay->crop.top) << 32);

    info->screen_win.x = lay->frame.left < 0 ? 0 : lay->frame.left;
    info->screen_win.y = lay->frame.top < 0 ? 0 : lay->frame.top;
    info->screen_win.width = lay->frame.right - lay->frame.left;
    info->screen_win.height = lay->frame.bottom - lay->frame.top;

    operate3dMode(display, info, isVideoLay(lay));
    //showLayInfo(config);
    return 0;
err:
    return -1;

 return 0;
}

static void setupLay(Display *display, Spipe *spipe, Pipe *pipe, int pos)
{
    int i;
    int curLay = 0, zorder = 0;

    if (spipe == NULL) {
        // no spipe to config.
        return;
    }

    for (i = 0; i < LAYER_BY_PIPE; i++) {
        if (spipe->lay[i] == NULL) {
            continue;
        }
        zorder = pos * LAYER_BY_PIPE + curLay;
        covLayToInfo(display, spipe->lay[i], &pipe->layInfo[curLay], zorder);
        curLay++;
    }
}

static Spipe *findSpipeByListid(HwSource2 *hw, int pos)
{
    int i = 0;
    Spipe *pi;
    struct listnode *node;

    if (pos == -1) {
        /* there is useless pos */
        return NULL;
    }

    list_for_each(node, &hw->sPipeHead) {
        pi = node_to_item(node, Spipe, sPNode);
        if (i == pos) {
            //ALOGV("%s:Spipe was found, id is %d!\n", __func__, pos);
            return pi;
        }
        i++;
    }
    //ALOGV("%s:Spipe not found, something was wrong!\n", __func__);

    return NULL;
}

static void setupPipe(Display *display)
{

    int i = 0;
    int pos = 0;
    Spipe *sp = NULL;
    HwSource2 *hw = &hwSource2[display->displayId];

    for (i = 0; i < hw->fix.pipeNum; i++) {
        pos = hw->pipe[i].spipe;
        sp = findSpipeByListid(hw, pos);
        if (sp) {
            /* find one */
            //ALOGV("%s:find sp,pos = %d setupLay\n", __func__, pos);
            setupLay(display, sp, &hw->pipe[i], pos);
        }
    }
}

static int storeLayConfig(Display *display, disp_layer_config2 **config, int num, int releasefencefd)
{
    disp_layer_config2 *configs;
    disp_layer_config2 *configDst;
    disp_layer_config2 *configSrc = NULL;
    LayerWork *layerWork = NULL;
    HwSource2 *hw = &hwSource2[display->displayId];
    Layer *ly;
    int i = 0;
    struct listnode *node;

    configs = (disp_layer_config2 *)malloc(num * sizeof(struct disp_layer_config2));
    configDst = configs;
    if (configs == NULL) {
        goto err;
    }
    memset(configs, 0, num * sizeof(struct disp_layer_config2));

    for (i = 0; i < num; i++) {
        configSrc = config[i];
        memcpy(configDst, configSrc, sizeof(struct disp_layer_config2));
        configDst++;
    }

    layerWork = (LayerWork *)calloc(1, sizeof(LayerWork));
    layerWork->num = num;
    layerWork->config = configs;
    layerWork->releaseFenceFd = releasefencefd;
    layerWork->layerSize = sizeList(&display->layerSortedByZorder);
    layerWork->acqFence = (int32_t *)calloc(layerWork->layerSize, sizeof(int32_t));
    for (i = 0; i < layerWork->layerSize; i++) {
        layerWork->acqFence[i] = -1;
    }
    i = 0;

    list_for_each(node, &display->layerSortedByZorder) {
        ly = node_to_item(node, struct Layer, node);
        if (ly->acquireFence >= 0) {
            layerWork->acqFence[i] = dup(ly->acquireFence);
            close(ly->acquireFence);
            i++;
        }
    }
    list_init(&layerWork->node);
    pthread_mutex_lock(&hw->threadHead.mutex);
    list_add_tail(&hw->threadHead.workHead, &layerWork->node);
    pthread_cond_signal(&hw->threadHead.cond);
    pthread_mutex_unlock(&hw->threadHead.mutex);

    return 0;
err2:
    free(configs);
err:
    return -1;
}

static void setupReleaseFence(Display *display, int releasefencefd)
{
    struct listnode *node;
    Layer *ly;

    list_for_each(node, &display->layerSortedByZorder) {
            ly = node_to_item(node, struct Layer, node);
            if (ly->compositionType == HWC2_COMPOSITION_DEVICE
                && releasefencefd >= 0)
                ly->releaseFence = dup(releasefencefd);
            else {
                ly->releaseFence = -1;
            }
    }
    /*if (releasefencefd >= 0) {
        close(releasefencefd);
    }
    */
}

static int setLayConfig(LayerWork *layerWork, Display *display)
{
    unsigned long arg[4] = {0};
    HwSource2 *hw = &hwSource2[display->displayId];
    int ret = 0;
    disp_layer_config2 *config = layerWork->config;
    ATRACE_CALL();

    for (int i = 0; i < layerWork->num; i++) {
        showLayInfo(config);
        config++;
    }

    char value[2];
    property_get("service.atw.enable", value, "0");
    if(atoi(value)){
        //don't send layers to de.
        return 0;
    }

    /* open protect. */
    arg[0] = display->displayId;
    arg[1] = 1;
    ret = ioctl(hw->fix.dispFd, DISP_SHADOW_PROTECT, (unsigned long)arg);
    if (ret) {
        ALOGE("%d err: DISP_SHADOW_PROTECT failed", __LINE__);
        goto err;
    }

    arg[0] = display->displayId;
#if 0
    arg[1] = (unsigned long)(layerWork->config);
    arg[2] = layerWork->num;
    ret = ioctl(hw->fix.dispFd, DISP_LAYER_SET_CONFIG2, (unsigned long)arg);

    if (ret) {
       ALOGE("%d err: DISP_LAYER_SET_CONFIG failed", __LINE__);
       goto err;
    }
#else
    arg[1] = (unsigned long)(layerWork->config);
    arg[2] = (unsigned long)(display->displayId ? 8 : 16);
    ret = ioctl(hw->fix.dispFd, DISP_LAYER_SET_CONFIG2, (unsigned long)arg);

    arg[1] = HWC_SUBMIT_LAYER_FENCE;
    arg[2] = HWC_SYNC_TIMELINE_DEFAULT;
    arg[3] = layerWork->releaseFenceFd;
    ret = ioctl(hw->fix.dispFd, DISP_HWC_COMMIT, (unsigned long)arg);
    if (ret) {
        ALOGE("fence commit err.");
    }
    if (layerWork->releaseFenceFd>= 0) {
        close(layerWork->releaseFenceFd);
    }
    #if 0
    if (!ret) {
        int fencefd = -1;
        int releasefencefd = -1;
        arg[1] = HWC_COMMIT_FRAME_NUM_INC;
        arg[2] = HWC_SYNC_TIMELINE_DEFAULT;
        arg[3] = 1;
        releasefencefd = ioctl(hw->fix.dispFd, DISP_HWC_COMMIT, (unsigned long)arg);
        setupReleaseFence(display, releasefencefd);
    } else {
        ALOGE("DISP_LAYER_SET_CONFIG failed !");
    }
    #endif
#endif


    /* close protect. */
    arg[0] = display->displayId;
    arg[1] = 0;

    ret = ioctl(hw->fix.dispFd, DISP_SHADOW_PROTECT, (unsigned long)arg);
    if (ret) {
        ALOGE("%d err: DISP_SHADOW_PROTECT failed", __LINE__);
        goto err;
    }
err:
    return ret;
}

static int setLayConfig(Display *display, disp_layer_config2 **config, int num, int32_t * retireFence)
{
    int ret = 0, i = 0;
    HwSource2 *hw = &hwSource2[display->displayId];
    unsigned long arg[4] = {0};
    disp_layer_config2 configStore[PIPE_NUM * LAYER_BY_PIPE];
    disp_layer_config2 *configSrc = NULL;
    disp_layer_config2 *configDst = NULL;
    configDst = (disp_layer_config2 *)&configStore[0];

    memset(configDst, 0, PIPE_NUM * LAYER_BY_PIPE * sizeof(disp_layer_config2));
    for (i = 0; i < hw->fix.pipeNum * LAYER_BY_PIPE; i++) {
        configSrc = config[i];
        memcpy(configDst, configSrc, sizeof(disp_layer_config2));
        showLayInfo(configDst);
        configDst++;
    }

    /* open protect. */
    arg[0] = display->displayId;
    arg[1] = 1;
    ret = ioctl(hw->fix.dispFd, DISP_SHADOW_PROTECT, (unsigned long)arg);
    if (ret) {
        ALOGE("%d err: DISP_SHADOW_PROTECT failed", __LINE__);
        goto err;
    }

    arg[0] = display->displayId;
    arg[1] = (unsigned long)(&configStore[0]);
    if (display->displayId == 0) {
        arg[2] = 16;
    } else {
        arg[2] = 8;
    }

    ret = ioctl(hw->fix.dispFd, DISP_LAYER_SET_CONFIG2, (unsigned long)arg);
    if (!ret) {
        int fencefd = -1;
        int releasefencefd = -1;
        arg[1] = HWC_COMMIT_FRAME_NUM_INC;
        arg[2] = HWC_SYNC_TIMELINE_DEFAULT;
        arg[3] = 1;
        releasefencefd = ioctl(hw->fix.dispFd, DISP_HWC_COMMIT, (unsigned long)arg);
        *retireFence = dup(releasefencefd);
        setupReleaseFence(display, releasefencefd);
    } else {
        ALOGE("DISP_LAYER_SET_CONFIG failed !");
    }

    /* close protect. */
    arg[0] = display->displayId;
    arg[1] = 0;

    ret = ioctl(hw->fix.dispFd, DISP_SHADOW_PROTECT, (unsigned long)arg);
    if (ret) {
        ALOGE("%d err: DISP_SHADOW_PROTECT failed", __LINE__);
        goto err;
    }

err:
    return ret;
}

static void clearAcquireFence(Display *display)
{
    struct listnode *node;
    Layer *ly;

    list_for_each(node, &display->layerSortedByZorder) {
        ly = node_to_item(node, struct Layer, node);
        if (ly->acquireFence > 0) {
            ly->acquireFence = -1;
        }
    }
}

static int sendLayConfig(Display *display, int32_t * retireFence)
{
    int num = 0, i = 0, j = 0;
    int ret = 0;
    unsigned long arg[4] = {0};
    HwSource2 *hw = &hwSource2[display->displayId];
    disp_layer_config2 *config[PIPE_NUM * LAYER_BY_PIPE] = {NULL};

    for (i = 0; i < hw->fix.pipeNum; i++) {
        for (j = 0; j < LAYER_BY_PIPE; j++) {
            config[num] = &hw->pipe[i].layInfo[j];
            num++;
        }
    }

    int fencefd = -1;
    int releasefencefd = -1;
    arg[1] = HWC_COMMIT_FRAME_NUM_INC;
    arg[2] = HWC_SYNC_TIMELINE_DEFAULT;
    arg[3] = 1;
    releasefencefd = ioctl(hw->fix.dispFd, DISP_HWC_COMMIT, (unsigned long)arg);
    *retireFence = dup(releasefencefd);
    setupReleaseFence(display, releasefencefd);
    ret = storeLayConfig(display, config, num, releasefencefd);
    clearAcquireFence(display);
    return ret;
    //return setLayConfig(display, config, num, retireFence);
}

static int setupLayTopipe(Display *display, int32_t * retireFence)
{
    struct listnode *node;
    Spipe *pi;
    HwSource2 *hw = &hwSource2[display->displayId];
    int i = 0;
    int vcount = 0, ucount = 0;

    list_for_each(node, &hw->sPipeHead) {
        /* contect spipe and pipi. */
        pi = node_to_item(node, Spipe, sPNode);
        if (pi->type == VIDEO) {
            if (vcount < VI_NUM) {
                hw->pipe[vcount].spipe = i;
                //ALOGV("%d sPipenodeCnt is video=%d =============\n", __LINE__, i);
            }
            vcount++;
        } else {
            hw->pipe[ucount + VI_NUM].spipe = i;
            //ALOGV("%d sPipenodeCnt is ui=%d =============\n", __LINE__, i);
            ucount++;
        }
        i++;
    }

    setupPipe(display);
    return sendLayConfig(display, retireFence);
}

static void calcFactor(HwSource2 *hw, float *wf, float *hf)
{

    float wFactor = (float)hw->fix.persentW / 100;
    float Hfactor = (float)hw->fix.persentH / 100;

    if(hw->fix.width && hw->fix.height)
    {
        wFactor = (float)hw->var.width / hw->fix.width * hw->fix.persentW / 100;
        Hfactor = (float)hw->var.height/ hw->fix.height * hw->fix.persentH / 100;
    }

    *wf = wFactor;
    *hf = Hfactor;
}

static bool layerIsScaled(Display *display, Layer *layer)
{
    float wFactor = 1;
    float hFactor = 1;
    int w = 0, h = 0;

    HwSource2 *hw = &hwSource2[display->displayId];

    calcFactor(hw, &wFactor, &hFactor);

    w = layer->crop.right - layer->crop.left;
    h = layer->crop.bottom - layer->crop.top;

    if (layer->transform & HWC_TRANSFORM_ROT_90)
    {
        int tmp = w;
        w = h;
        h = tmp;
    }

    return (((layer->frame.right - layer->frame.left) * wFactor != w)
        || ((layer->frame.bottom - layer->frame.top) * hFactor != h));
}

static void initHw2(HwSource2 *hw)
{
    memset(hw, 0, sizeof(*hw));
}

static void resetVar(HwSource2 *hw)
{
    int i = 0;
    HardWareVar *var = &hw->var;

    if (!var) {
        ALOGE("var is null!\n");
        return;
    }

    memset(var, 0, sizeof(*var));
}

static bool isLayerListTypeChange(Display* display)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    struct listnode *node;
    struct listnode *list;
    struct Layer *lay;

    list = &display->layerSortedByZorder;
    list_for_each(node, list) {
        lay = node_to_item(node, Layer, node);
        if (lay->compositionType == HWC2_COMPOSITION_CLIENT_TARGET) {
            continue;
        }
        if (lay->typeChange) {
            return true;
        }
    }

    return false;
}

int32_t de2TryToAssignLayer(Display* display)
{
    bool needSoftComp;
    int tryFlag = 0;
    struct listnode *node;
    struct Layer *lay;
    int ret = 1;

    if (!display) {
        ALOGE("%s NULL pointer\n", __func__);
        return 0;
    }
    HwSource2 *hw = &hwSource2[display->displayId];

    initHwPipe(hw);
    resetHwSpipe(hw);

#ifdef FORCE_GPU
    forceGPUAssign(display);
#else
    defaultAssign(display);
    ret = isLayerListTypeChange(display);
#endif
    return ret;
}

/* judge if list have gles buffer */
static bool isUseFbtarget(Display *display)
{
    struct listnode *node;
    struct Layer *lay;

    list_for_each(node, &display->layerSortedByZorder) {
            lay = node_to_item(node, Layer, node);
            if (lay->compositionType == HWC2_COMPOSITION_CLIENT) {
                return true;
            }
    }

    return false;
}

int32_t de2PresentDisplay(Display *display, int32_t * retireFence)
{
    ATRACE_CALL();
    HwSource2 *hw = &hwSource2[display->displayId];
    return setupLayTopipe(display, retireFence);
}

//TODO
void de2Dump(Display *display, uint32_t* outSize, char* outBuffer)
{
    int count = 0;
    struct listnode *node;
    struct Layer *lay;
    HwSource2 *hw = &hwSource2[display->displayId];

    if (outBuffer == NULL) {
        ALOGE("%s: err, outbuffer is NULL!\n", __func__);
        return;
    }

    list_for_each(node, &display->layerSortedByZorder) {
        lay = node_to_item(node, Layer, node);
        count += sprintf(outBuffer + count, " layer[%p] ", lay);
        count += sprintf(outBuffer + count, " handle[%p] ", lay->buffer);
        count += sprintf(outBuffer + count, " acquirefence[%d] ", lay->acquireFence);
        count += sprintf(outBuffer + count, " releaseFence[%d] ", lay->releaseFence);
        count += sprintf(outBuffer + count, " composition type: [%d] ", lay->compositionType);
        count += sprintf(outBuffer + count, " mode[%d] ", lay->blendMode);
        count += sprintf(outBuffer + count, " dspace[%d] ", lay->dataspace);
        count += sprintf(outBuffer + count, " tr[%d] ", lay->transform);
        count += sprintf(outBuffer + count, " frame[%4d,%4d,%4d,%4d] ", lay->frame.left,
                lay->frame.top, lay->frame.right, lay->frame.bottom);
        count += sprintf(outBuffer + count, " planeAlpha[%4f] ", lay->planeAlpha);
        count += sprintf(outBuffer + count, " crop[%4f, %4f, %4f, %4f] ", lay->crop.left,
                lay->crop.top, lay->crop.right, lay->crop.bottom);
        //count += sprintf(outBuffer + count, " vi[%1d] ", lay->visibleRegion);
        count += sprintf(outBuffer + count, " z[%1d] ", lay->zorder);
        count += sprintf(outBuffer + count, " changed[%1d] \n", lay->typeChange);
    }

    *outSize = count;
}

static int initHdmi(Display *display, struct fb_var_screeninfo *info)
{
    HwSource2 *hw = &hwSource2[display->displayId];

    hw->fix.dpiX = 213000;
    hw->fix.dpiY = 213000;
    hw->fix.vsyncPeriod = 1000000000 / 24;
    /* ======fix========= */
    hw->fix.persentHT = 100;
    hw->fix.persentWT = 100;
    hw->fix.persentH = 100;
    hw->fix.persentW = 100;
    /* ================== */
    hw->fix.width = 1920;
    hw->fix.height = 1080;
    hw->var.width = 1920;
    hw->var.height = 1080;
    hw->var.vsyncEn = 1;
    return 0;
}

static int de2SwitchDevice(Display *display, int type, int mode)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    unsigned long arg[4] = {0};
    HardWareFix *fix = &hw->fix;
    HardWareVar *var = &hw->var;
    HwSource2 *hw0 = &hwSource2[0];


    ALOGV("switch device %d.", display->displayId);
    /* TODO: open too much */
    if (!fix->dispFd) {
        /* display device does not open */
        fix->dispFd = hw0->fix.dispFd;//open("/dev/disp", O_RDWR);
        if (!fix->dispFd) {
            ALOGE("failed open disp device.");
        }
        fix->fbFd = hw0->fix.fbFd;//open("/dev/graphics/fb0", O_RDWR);
        if (!fix->fbFd)
            ALOGE("failed open fb0 device.");

        fix->ionFd = hw0->fix.ionFd;//open("/dev/ion",O_RDWR);
        if (!fix->ionFd)
            ALOGE("failed open ion device.");

        fix->trFd = hw0->fix.trFd;//open("/dev/transform",O_RDWR);
        if(!fix->trFd) {
            ALOGE("Failed to open transform device");
        }
    }

    arg[0] = display->displayId; //use de0 as hdmi
    arg[1] = type;
    arg[2] = mode;

    ALOGV("%s: type = %d. mode = %d, id = %d", __FUNCTION__, type, mode, display->displayId);
    if (ioctl(fix->dispFd, DISP_DEVICE_SWITCH, (unsigned long)arg) == -1) {
        ALOGE("switch device failed!\n");
    }

    arg[1] = type ? 1 : 0;
    if (ioctl(fix->dispFd, DISP_VSYNC_EVENT_EN, (unsigned long)arg) == -1) {
        ALOGE("vysn event control failed!\n");
    }

    return 0;
}


static int initLcd(Display *display, struct fb_var_screeninfo *info)
{
    int refreshRate, xdpi, ydpi, vsync_period;
    HwSource2 *hw = &hwSource2[display->displayId];

    refreshRate = 1000000000000LLU /
                    (uint64_t(info->upper_margin + info->lower_margin + info->vsync_len + info->yres)
                    * ( info->left_margin  + info->right_margin + info->hsync_len + info->xres)
                    * info->pixclock);
    if (refreshRate == 0) {
        ALOGW("invalid refresh rate, assuming 60 Hz");
        refreshRate = 60;
    }
    if (info->width == 0)
        hw->fix.dpiX = 160000;
    else
        hw->fix.dpiX = 1000 * (info->xres * 25.4f) / info->width;

    if(info->height == 0)
        hw->fix.dpiY = 160000;
    else
        hw->fix.dpiY = 1000 * (info->yres * 25.4f) / info->height;

    hw->fix.vsyncPeriod = 1000000000 / refreshRate;
    /* ======fix========= */
    hw->fix.persentHT = 100;
    hw->fix.persentWT = 100;
    hw->fix.persentH = 100;
    hw->fix.persentW = 100;
    hw->fix.width = info->xres;
    hw->fix.height = info->yres;
    hw->var.width = info->xres;
    hw->var.height = info->yres;
    hw->var.vsyncEn = 1;
    return 0;
}

static LayerWork *getHeadWork(HwSource2 *hw)
{
    struct listnode *node;
    LayerWork *layerWork = NULL;

    pthread_mutex_lock(&hw->threadHead.mutex);
    if (!list_empty(&hw->threadHead.workHead)) {
        list_for_each(node, &hw->threadHead.workHead) {
            layerWork = node_to_item(node, LayerWork, node);
            if (layerWork) {
                list_remove(&layerWork->node);
                break;
            }
        }
    }
    pthread_mutex_unlock(&hw->threadHead.mutex);
    return layerWork;
}

static void clearWork(HwSource2 *hw)
{
    struct listnode *node;
    LayerWork *layerWork = NULL;

    pthread_mutex_lock(&hw->threadHead.mutex);
    if (list_empty(&hw->threadHead.workHead)) {
        pthread_mutex_unlock(&hw->threadHead.mutex);
        return;
    }
    list_for_each(node, &hw->threadHead.workHead) {
        layerWork = node_to_item(node, LayerWork, node);
        if (layerWork) {
            ALOGD("Have some other work");
            list_remove(&layerWork->node);
            free(layerWork->config);
            free(layerWork);
            //break;
        }
    }
    pthread_mutex_unlock(&hw->threadHead.mutex);
}

static void *present_thread_loop(void *ptr)
{
    LayerWork *layerWork = NULL;
    Display *display = (Display *)ptr;
    HwSource2 *hw = &hwSource2[display->displayId];
    int i;

    setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY - 2);
    while(hw->threadHead.start) {
        pthread_mutex_lock(&hw->threadHead.mutex);
        if (list_empty(&hw->threadHead.workHead)) {
            pthread_cond_wait(&hw->threadHead.cond, &hw->threadHead.mutex);
        }
        pthread_mutex_unlock(&hw->threadHead.mutex);

        layerWork = getHeadWork(hw);
        if (layerWork) {
#if 1
            for (i = 0; i < layerWork->layerSize; i++) {
                if(layerWork->acqFence[i] >= 0 && !sync_wait(layerWork->acqFence[i], 3000)) {
                    continue;
                } else if(layerWork->acqFence[i] >= 0) {
                    ALOGE("WARNING %s : layer wait acquirefence %d timeout", __FUNCTION__,
                            layerWork->acqFence[i]);
                    continue;
                }
            }
#endif

            setLayConfig(layerWork, display);
#if 1
            pthread_mutex_lock(&display->mutex);
            display->frameCount = (display->frameCount) + 1;
            ALOGV("vsync %s: framecnt = %d, cur-pre= %d",
                __FUNCTION__, display->frameCount, display->frameCount - display->preFrameCount);
            display->preFrameCount = display->frameCount;
            pthread_mutex_unlock(&display->mutex);
#endif

#if 1
            for (i = 0; i < layerWork->layerSize; i++) {
                if(layerWork->acqFence[i] >= 0) {
                    close(layerWork->acqFence[i]);
                }
            }
#endif
            free(layerWork->config);
            free(layerWork);
        }
    }
    return NULL;
}

int de2Dinit(Display* display)
{
    HwSource2 *hw = &hwSource2[display->displayId];

    //hw->threadHead.start = 0;
    //pthread_join(hw->threadHead.pid, NULL);
    resetHwSpipe(hw);
    clearListExcepFbTarget(&display->layerSortedByZorder);
    clearWork(hw);
    //initHw2(hw);
    return 0;
}

int de2Init(Display* display)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    unsigned long arg[4] = {0};
    HardWareFix *fix = &hw->fix;
    HardWareVar *var = &hw->var;
    struct fb_var_screeninfo info;
    struct disp_output outPut;
    int ret = 0;
    int fd;

    //initHw2(hw);
    //resetVar(hw);
    if (!fix->dispFd) {
        fix->dispFd = open("/dev/disp", O_RDWR);
        if (!fix->dispFd) {
            ALOGE("failed open disp device.");
        }
        fix->fbFd = open("/dev/graphics/fb0", O_RDWR);
        if (!fix->fbFd)
            ALOGE("failed open fb0 device.");

        fix->ionFd = open("/dev/ion",O_RDWR);
        if (!fix->ionFd)
            ALOGE("failed open ion device.");

        fix->trFd = open("/dev/transform",O_RDWR);
        if(!fix->trFd) {
            ALOGE("Failed to open transform device");
        }

        if (ioctl(fix->fbFd, FBIOGET_VSCREENINFO, &info) == -1) {
            ALOGE("FBIOGET_VSCREENINFO ioctl failed: %s", strerror(errno));
            return -1;
        }
    }

    arg[0] = display->displayId;
    arg[1] = (unsigned long)&outPut;
    ret = ioctl(fix->dispFd, DISP_GET_OUTPUT, arg);
    if (ret)
        ALOGE("get output type is NONE!\n");
    else
        ALOGV("get output type is not NONE!\n");

    ALOGV("get type is %d,mode is %d \n", outPut.type, outPut.mode);
    switch (outPut.type) {
    case DISP_OUTPUT_TYPE_LCD:
        ALOGV("err: get output type is LCD!\n");
        initLcd(display, &info);
        break;
    case DISP_OUTPUT_TYPE_HDMI:
        ALOGV("err: get output type is HDMI!\n");
        initHdmi(display, &info);
        break;
    case DISP_OUTPUT_TYPE_TV:
        break;
    case DISP_OUTPUT_TYPE_VGA:
        break;
    default:
        break;
    }
    ALOGV("get device width = %d, height = %d.\n", fix->width, fix->height);

    /* runtime. */
    fd = open("/sys/class/disp/disp/attr/runtime_enable", O_WRONLY);
    if (fd >= 0) {
        char i = '1';
        ssize_t ret = 0;
        ret = write(fd, &i, 1);
        if (ret < 0)
            ALOGE("###write /sys/class/disp/disp/attr/runtime_enable fail!");
        close(fd);
    } else {
        ALOGE("###open /sys/class/disp/disp/attr/runtime_enable fail!");
    }

    fix->tdFd = open("/sys/class/disp/disp/attr/operate_3d_mode", O_RDONLY);
    if (fix->tdFd  < 0) {
        ALOGE("###open /sys/class/disp/disp/attr/operate_3d_mode fail!");
    }

    if (display->displayId == 0) {
        hw->fix.pipeNum = 4;
    } else if (display->displayId == 1) {
        hw->fix.pipeNum = 2;
    }
    /* config list is not ready. */
    int num = display->configNumber;
    display->displayConfigList->width = hw->fix.width;
    display->displayConfigList->height = hw->fix.height;
    display->displayConfigList->dpiX = hw->fix.dpiX;
    display->displayConfigList->dpiY = hw->fix.dpiY;
    display->displayConfigList->vsyncPeriod = hw->fix.vsyncPeriod;
    ALOGD("HW0 = 0x%p", &hwSource2[0]);
    ALOGD("HW1 = 0x%p", &hwSource2[1]);

    initHwSpipe(hw);
    initHwPipe(hw);
    list_init(&hw->threadHead.workHead);

    if (!hw->threadHead.start) {
        pthread_create(&hw->threadHead.pid, NULL, present_thread_loop, display);
        pthread_cond_init(&hw->threadHead.cond, NULL);
        pthread_mutex_init(&hw->threadHead.mutex, 0);
        hw->threadHead.start = 1;
    }

    // for atw coeff.

    // int eyew = 1080;
    // int eyeh = 1920/2;
    // int disp_w = 1080;
    // int disp_h = 1920;

    const int tx = 40;
    const int ty = 40;
    const int NumIntsPerBlock = 4*8; //R|G|B|K, each block has 4*8 int32_t, which equal to 4*8 RGBA8888 pixels.
    const int NumBlocksPerEye = tx*ty;
    const int NumIntsPerEye = NumBlocksPerEye*NumIntsPerBlock;
    // each sgb will need to store tx*ty*2 i32blocks
    int bufferWidth = tx*8;
    int bufferHeight = ty*4*2;
    uint32_t usage = GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_HW_COMPOSER;
    PixelFormat format = PIXEL_FORMAT_RGBA_8888;

    coefficents = new GraphicBuffer(bufferWidth, bufferHeight, format, usage);

    int32_t *bufferTarget = nullptr;
    coefficents->lock(GraphicBuffer::USAGE_SW_WRITE_OFTEN, (void **)&bufferTarget);
    //memset(bufferTarget, 0x55, bufferWidth * bufferHeight * 4);
    LoadDistortionBuffer(bufferTarget, bufferWidth * bufferHeight * 4);

    coefficents->unlock();
    cofficentHandle = coefficents->getNativeBuffer()->handle;
    ALOGD("cofficentHandle = %u", cofficentHandle);

    arg[0] = 0;
    arg[1] = HWC_START;
    arg[2] = START_CLIENT_HWC_2;
    if (ioctl(fix->dispFd, DISP_HWC_COMMIT, (unsigned long)arg)) {
        ALOGE("start devcomposer failed !!!");
    }
    arg[0] = 0;
    arg[1] = HWC_SYNC_TIMELINE_SET_OFFSET;
    arg[2] = HWC_SYNC_TIMELINE_DEFAULT;
    arg[3] = 0;
    if (ioctl(fix->dispFd, DISP_HWC_COMMIT, (unsigned long)arg)) {
        ALOGE("HWC_SYNC_TIMELINE_SET_OFFSET disp0_0 failed !");
    }
    arg[0] = 1;
    arg[1] = HWC_SYNC_TIMELINE_SET_OFFSET;
    arg[2] = HWC_SYNC_TIMELINE_DEFAULT;
    arg[3] = 0;
    if (ioctl(fix->dispFd, DISP_HWC_COMMIT, (unsigned long)arg)) {
        ALOGE("HWC_SYNC_TIMELINE_SET_OFFSET disp1_0 failed !");
    }
    return 0;
}

int32_t de2SetPowerMode(Display* display, int32_t mode)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    unsigned long arg[4] = {0};
    int ret = 0;
    unsigned long vsyncEn = 0;
    arg[0] = display->displayId;

    switch (mode) {
    case HWC2_POWER_MODE_OFF:
        arg[1] = 1;
        vsyncEn = 0;
        break;
    case HWC2_POWER_MODE_DOZE:
    case HWC2_POWER_MODE_DOZE_SUSPEND:
    case HWC2_POWER_MODE_ON:
        arg[1] = 0;
        vsyncEn = 1;
        break;
    }

    if (ioctl(hw->fix.dispFd, DISP_BLANK, arg)) {
        ALOGE("DISP_BLANK ioctl failed: %s", strerror(errno));
        return -1;
    }

    arg[1] = vsyncEn;
    if (ioctl(hw->fix.dispFd, DISP_VSYNC_EVENT_EN, arg)) {
            ALOGE("DISP_CMD_VSYNC_EVENT_EN ioctl failed: %s", strerror(errno));
            return -1;
    }
    hw->var.vsyncEn = !!vsyncEn;

    return 0;
}

int32_t de2SetVsyncEnabled(Display* display, int32_t enabled)
{
    HwSource2 *hw = &hwSource2[display->displayId];
    unsigned long arg[4] = {0};
    arg[0] = display->displayId;
    //arg[1] = (enabled == HWC2_VSYNC_ENABLE)?1:0;
    arg[1] = 1;
    //TODO vsync
    if (ioctl(hw->fix.dispFd, DISP_VSYNC_EVENT_EN, arg)) {
            ALOGE("DISP_CMD_VSYNC_EVENT_EN ioctl failed: %s", strerror(errno));
            return -1;
    }
    hw->var.vsyncEn = enabled;

    return 0;
}

DisplayOpr de2DisplayOpr = {
    .tryToAssignLayer = de2TryToAssignLayer,
    .presentDisplay = de2PresentDisplay,
    .dump = de2Dump,
    .init = de2Init,
    .deInit = de2Dinit,
    .setPowerMode = de2SetPowerMode,
    .setVsyncEnabled = de2SetVsyncEnabled,
    .switchDevice = de2SwitchDevice,
};

