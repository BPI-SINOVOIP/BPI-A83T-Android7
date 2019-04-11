#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include "AtwLog.h"
#include <hardware/sunxi_display2.h>
#include "hwtimewarp.h"

#include <EGL/egl.h>

#include "hwtw_sunxi_ion.h"

#define HW_ALIGN 64  // gralloc 分配的 buffer 是 64bytes 对齐的。 这是 GPU 方面的规定。
#define ALIGN(x,a)   (((x) + (a) - 1L) & ~((a) - 1L))

#define MAX(x,y) ((x)>(y) ? (x):(y))
#define MIN(x,y) ((x)>(y) ? (y):(x))

using namespace android;

struct hwtimewarp_context_t {
    hwtimewarp_device_1_t device;
    int         sunxiDispFd;    // display driver device fd
    int         ionFd;          // ion device fd
    AtwData_t   atwData;        // display 当前显示帧
    DisplayInfo_t dispInfo; // display 的信息
};

static int hwtimewarp_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static struct hw_module_methods_t hwtimewarp_module_methods = {
    .open = hwtimewarp_device_open
};

hwtimewarp_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = 1,
        .hal_api_version = 0,
        .id = HWTIMEWARP_HARDWARE_MODULE_ID,
        .name = "allwinner hwtimewarp module",
        .author = "softwinner",
        .methods = &hwtimewarp_module_methods,
    }
};

static int hwtimewarp_device_getDisplayInfo(struct hwtimewarp_device_1 *dev, DisplayInfo_t *dispInfo)
{
    struct hwtimewarp_context_t *ctx = (struct hwtimewarp_context_t *)dev;
    if(dispInfo != nullptr)
    {
        *dispInfo = ctx->dispInfo;
    }
    return 0;
}

static int hwtimewarp_device_getDisplayVsync(struct hwtimewarp_device_1 *dev, HardwareVsync_t *hwVsync)
{
    struct hwtimewarp_context_t *ctx = (struct hwtimewarp_context_t *)dev;

    UN_USED(ctx);
    UN_USED(dev);
    UN_USED(hwVsync);
    return 0;
}

static int hwtimewarp_device_getDisplayTiming(struct hwtimewarp_device_1 *dev, DisplayTconTiming_t *dispTconTiming)
{
    struct hwtimewarp_context_t *ctx = (struct hwtimewarp_context_t *)dev;

    UN_USED(ctx);
    UN_USED(dev);
    UN_USED(dispTconTiming);
    return 0;
}

static int hwtimewarp_device_enable(struct hwtimewarp_device_1 *dev, bool enable)
{
    struct hwtimewarp_context_t *ctx = (struct hwtimewarp_context_t *)dev;

    // enalbe or disable vsync.
    unsigned long arg[3] = {0};
    arg[1] = enable == true ? 1 : 0;
    int dispFd = ctx->sunxiDispFd;
    int ret = ioctl(dispFd, DISP_VSYNC_EVENT_EN, &arg[0]);
    if(ret != 0)
    {
        _LOGE("ioctl %s vsync return %d(%s)", enable ? "enable" : "disable", ret, strerror(errno));
        return -1;
    }
    return 0;
}

static void showLayInfo(disp_layer_config2 *config)
{
    /* for test. */
    static char buf[256] = {0};
    int count = 0;

    _LOGV("%s: One frame Hardware data>>>", __func__);
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
        config->info.fb.size[1].width, config->info.fb.size[1].height,config->info.fb.size[2].width, config->info.fb.size[2].height);
    count += sprintf(buf + count, "crop[%4d,%4d,%4d,%4d] ", (unsigned int)(config->info.fb.crop.x>>32), (unsigned int)(config->info.fb.crop.y>>32),
        (unsigned int)(config->info.fb.crop.width>>32), (unsigned int)(config->info.fb.crop.height>>32));
    count += sprintf(buf + count, "ali0[%d] ali1[%d] ali[%d]", config->info.fb.align[0], config->info.fb.align[1]
                    , config->info.fb.align[2]);
    count += sprintf(buf + count, "frame[%4d,%4d,%4d,%4d] ", config->info.screen_win.x, config->info.screen_win.y, config->info.screen_win.width, config->info.screen_win.height);
    count += sprintf(buf + count, "addr[%4d] ", config->info.fb.fd);
    count += sprintf(buf + count, "flags[0x%8x] trd[%1d,%1d]\n", config->info.fb.flags, config->info.b_trd_out, config->info.out_trd_mode);
    _LOGV("%s", buf);
    _LOGV("%s:                        >>>", __func__);
    memset(buf, 0, 256);
}

static int buildAtwLayer(const struct hwtimewarp_context_t *ctx, const AtwData_t &data, disp_layer_config2 *config)
{
    ATRACE_NAME("buildAtwLayer");

    int ionFd = ctx->ionFd;
    int atw_mode = (int)ctx->dispInfo.atw_mode;
    int viewport_w = ctx->dispInfo.viewport_w;
    int viewport_h = ctx->dispInfo.viewport_h;

    int disp_w, disp_h;
    disp_atw_mode disp_mode;
    switch(atw_mode)
    {
        case 0:
        {
            // 双屏左右
            disp_w = viewport_w / 2;
            disp_h = viewport_h;
            disp_mode = NORMAL_MODE;
            break;
        }
        case 1:
        {
            // 单屏左右
            disp_w = viewport_w;
            disp_h = viewport_h;
            disp_mode = LEFT_RIGHT_MODE;
            break;
        }
        case 2:
        {
            // 单屏上下.
            // 例如 1080x1920，  viewport_w = 1920, viewport_h = 1080, 实际 display 的宽高应该为 disp_w = 1080, disp_h = 1920
            disp_w = viewport_h;
            disp_h = viewport_w;
            disp_mode = UP_DOWN_MODE;
            break;
        }
        default:
        {
            _FATAL("unknown atw mode=%d", atw_mode);
        }
    }

    private_handle_t *priv_hnd_pixels = (private_handle_t *)data.hnd_pixel;
    private_handle_t *priv_hnd_coefficents = (private_handle_t *)data.hnd_coeffcient;

    if(private_handle_t::validate(priv_hnd_pixels) != 0)
    {
        _LOGE("hnd(%p): magic=%d, numFds=%d, numInts=%d, version=%d",
                priv_hnd_pixels,
                priv_hnd_pixels->magic,
                priv_hnd_pixels->numFds,
                priv_hnd_pixels->numInts,
                priv_hnd_pixels->version);
        _FATAL("hnd(%p) is not valid.",priv_hnd_pixels);
    }

    auto width = priv_hnd_pixels->width;
    auto height = priv_hnd_pixels->height;
    auto format = priv_hnd_pixels->format;
    auto pixels_fd = priv_hnd_pixels->share_fd;
    auto coeffcients_fd = priv_hnd_coefficents != nullptr ? priv_hnd_coefficents->share_fd : 0;

    memset(config, 0, sizeof(struct disp_layer_config2));
    config->enable = true;
    config->channel = 0;
    config->layer_id = 0;

    // setup normal layer info
    disp_layer_info2 *info = &config->info;
    info->mode = LAYER_MODE_BUFFER;
    info->zorder = 16;
    info->alpha_mode = 1;
    info->alpha_value = 0xff;//TODO: fix

    info->screen_win.x = 0;
    info->screen_win.y = 0;
    info->screen_win.width = disp_w;
    info->screen_win.height = disp_h;

    info->fb.fd = pixels_fd;
    info->fb.size[0].width = width;
    info->fb.size[0].height = height;
    info->fb.align[0] = HW_ALIGN;
    switch (format) {
    case HAL_PIXEL_FORMAT_RGBA_8888:
        info->fb.format = DISP_FORMAT_ABGR_8888;
        break;
    case HAL_PIXEL_FORMAT_RGBX_8888:
        info->fb.format = DISP_FORMAT_XBGR_8888;
        break;
    case HAL_PIXEL_FORMAT_BGRA_8888:
        info->fb.format = DISP_FORMAT_ARGB_8888;
        break;
    case HAL_PIXEL_FORMAT_RGB_888:
        info->fb.format = DISP_FORMAT_BGR_888;
        break;
    default:
        // do not support 565.
        _LOGE("DO not support format 0x%x in %s", format, __FUNCTION__);
        return -1;
    }

    info->fb.crop.x = ((long long)(0) << 32);
    info->fb.crop.y = ((long long)(0) << 32);
    info->fb.crop.width = ((long long)(width) << 32);
    info->fb.crop.height = ((long long)(height) << 32);

    if(coeffcients_fd != 0)
    {
        // set up atw layer info
        info->atw.used = 1;
        info->atw.mode = disp_mode;
        info->atw.b_row = 40;
        info->atw.b_col = 40;
        info->atw.cof_fd = coeffcients_fd;
        //_LOGV("set atw");
    }
    else
    {
        // disable atw.
        info->atw.used = 0;
        //_LOGV("diable atw");
    }
    return 0;
}

static int commitAtwLayer(const struct hwtimewarp_context_t *ctx, struct disp_layer_config2 *layerCommitted)
{
    ATRACE_NAME("commitAtwLayer");

    // 参考 hwc2/de2/DisplayOpr.cpp setLayConfig
    int ret = 0;
    unsigned long arg[4] = {0};
    int dispFd = ctx->sunxiDispFd;

    /* open protect. */
    arg[0] = 0; // display 0
    arg[1] = 1;
    ret = ioctl(dispFd, DISP_SHADOW_PROTECT, (unsigned long)arg);
    if(ret != 0)
    {
        _LOGE("%d err: DISP_SHADOW_PROTECT failed", __LINE__);
        return -1;
    }

    arg[0] = 0;
    arg[1] = (unsigned long)(layerCommitted);
    arg[2] = 1;
    ret = ioctl(dispFd, DISP_LAYER_SET_CONFIG2, (unsigned long)arg);
    if(ret != 0)
    {
        _LOGE("%d err: DISP_LAYER_SET_CONFIG2 failed", __LINE__);
        return -1;
    }

    /* close protect. */
    arg[0] = 0;
    arg[1] = 0;
    ret = ioctl(dispFd, DISP_SHADOW_PROTECT, (unsigned long)arg);
    if(ret != 0)
    {
        _LOGE("%d err: DISP_SHADOW_PROTECT failed", __LINE__);
        return -1;
    }

    return 0;
}

static int clearAllLayers(const struct hwtimewarp_context_t *ctx)
{
    ATRACE_NAME("clearAllLayers");
    int ret = 0;
    unsigned long arg[4] = {0};
    int dispFd = ctx->sunxiDispFd;

    const int dispIndex = 0;
    const int channelCountsPerDisp = 2;
    const int layersCountPerChannel = 2;
    struct disp_layer_config2 layersSet[channelCountsPerDisp][layersCountPerChannel];
    memset(layersSet, 0, sizeof(layersSet));
    for(int i=0; i<channelCountsPerDisp; i++)
    {
        for(int j=0; j<layersCountPerChannel; j++)
        {
            layersSet[i][j].enable = false;
            layersSet[i][j].channel = i;
            layersSet[i][j].layer_id = j;
        }
    }

    arg[0] = dispIndex;
    arg[1] = 1;
    ret = ioctl(dispFd, DISP_SHADOW_PROTECT, (unsigned long)arg);
    if(ret != 0)
    {
        _LOGE("%d err: DISP_SHADOW_PROTECT failed", __LINE__);
        return -1;
    }

    arg[0] = dispIndex;
    arg[1] = (unsigned long)(&layersSet[0][0]);
    arg[2] = channelCountsPerDisp*layersCountPerChannel;
    ret = ioctl(dispFd, DISP_LAYER_SET_CONFIG2, (unsigned long)arg);
    if(ret != 0)
    {
        _LOGE("%d err: DISP_LAYER_SET_CONFIG2 failed", __LINE__);
        return -1;
    }

    arg[0] = dispIndex;
    arg[1] = 0;
    ret = ioctl(dispFd, DISP_SHADOW_PROTECT, (unsigned long)arg);
    if(ret != 0)
    {
        _LOGE("%d err: DISP_SHADOW_PROTECT failed", __LINE__);
        return -1;
    }
    return 0;
}

static int hwtimewarp_device_setAtwLayer(struct hwtimewarp_device_1 *dev, AtwData_t *info)
{
    ATRACE_NAME("dev_setAtwLayer");

    struct hwtimewarp_context_t *ctx = (struct hwtimewarp_context_t *)dev;
    struct disp_layer_config2 layerSet;

    if(info->hnd_coeffcient == nullptr)
    {
        _LOGV("clear all layers.");
        return clearAllLayers(ctx);
    }

    if(0 != buildAtwLayer(ctx, *info, &layerSet))
    {
        _FATAL("build atw layer failed");
        return -1;
    }

    if(0 != commitAtwLayer(ctx, &layerSet))
    {
        _FATAL("commit atw layer failed");
        return -1;
    }
    return 0;
}

static int hwtimewarp_device_close(struct hw_device_t *dev)
{
    struct hwtimewarp_context_t* ctx = (struct hwtimewarp_context_t*)dev;
    if (ctx) {
        close(ctx->sunxiDispFd);
        free(ctx);
    }
    return 0;
}

static int initDeviceDisplayInfo(int dispFd, struct hwtimewarp_context_t *dev)
{
    DisplayInfo_t info;
    unsigned int scn_w;
    unsigned int scn_h;
    unsigned int scn_num = 1;// TODO: 获取屏幕数目。
    DispAtwMode_t mode = Up_Bottom_Single_Screen; // TODO: 从哪儿获取这个信息 ?
    unsigned long arg[3];
    arg[0] = 0;
    scn_w = ioctl(dispFd, DISP_GET_SCN_WIDTH, &arg[0]);
    scn_h = ioctl(dispFd, DISP_GET_SCN_HEIGHT, &arg[0]);
    //scn_num = ioctl(dispFd, DISP_GET_SCN_NUM, &arg[0]):
    //scn_mode = ioctl(dispFd, DISP_GET_SCN_ATW_MODE, &arg[0]);

    if(scn_num != 1)
    {// 1280*720*2, 960*960*2, 1024*1024*x, 1200*1200*2, 1280*1280*2, 1440*1440*2, 1536*1536*2
        _LOGE("currently we only support single screen");
        return -1;
    }
    else
    {// 1280*720,1920*1080, 2560*1440
        if( (scn_w > scn_h) && (scn_w > ATW_DEVICE_MAX_EYE_BUFFER_WIDTH))
        {
            // atw device 的最大行扫描宽度为 1280 像素, 最大行扫描行数为 1280*2
            // scn_w > scn_h 意味着这是一个物理横屏 land screen, atw 不能支持宽度>1280的行扫描buffer.
            _LOGE("atw device can not accept such screen resolution. scn_w=%d, scn_h=%d.", scn_w, scn_h);
            info.disableAtw = 1; // only do distortion correction.
        }
        else
        {
            info.disableAtw = 0;
        }

        info.scn_num = 1;
        info.scn_width = scn_w;
        info.scn_height = scn_h;
        info.atw_mode = mode;

        // viewport 以虚拟现实设备用户的观察下，规定水平方向为 viewport_w, 垂直方向为 viewport_h，此时设备的宽高。
        // 我们这里假设虚拟现实设备的观察坐标系下，用户一定看向的是一个宽>高的屏幕。
        info.viewport_w = MAX(scn_w, scn_h);
        info.viewport_h = MIN(scn_w, scn_h);

#if 1
        // 计算系统允许的最大 rendertarget 的宽高。
        int eye_w = info.viewport_w / 2;
        int eye_h = info.viewport_h;
        eye_w = MIN(eye_w, ATW_DEVICE_MAX_EYE_BUFFER_WIDTH);
        eye_h = MIN(eye_h, ATW_DEVICE_MAX_EYE_BUFFER_HEIGHT);
        info.rendertarget_w = eye_w * 2;
        info.rendertarget_h = eye_h;
#else
        // 用于测试 2K 屏的性能。
        info.rendertarget_w = ATW_DEVICE_MAX_EYE_BUFFER_WIDTH * 2;
        info.rendertarget_h = ATW_DEVICE_MAX_EYE_BUFFER_HEIGHT;
#endif
    }

    _LOGV("init screeninfo: atwDisabled=%d, scn_num=%d, scn_w=%d, scn_h=%d, mode=%d, vp_w=%d, vp_h=%d, rendertarget_w=%d, rendertarget_h=%d",
            info.disableAtw,
            info.scn_num,
            info.scn_width, info.scn_height,
            (int)info.atw_mode,
            info.viewport_w, info.viewport_h,
            info.rendertarget_w, info.rendertarget_h);
    dev->dispInfo = info;
    return 0;
}

static int hwtimewarp_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int status = -EINVAL;
    if (!strcmp(name, HWTIMEWARP_HARDWARE_WARP)) {
        struct hwtimewarp_context_t *dev;
        dev = (hwtimewarp_context_t*)malloc(sizeof(*dev));

        // initialize our state here
        memset(dev, 0, sizeof(*dev));

        // 打开 display device
        int dispFd = open("/dev/disp", O_RDWR);
        if( dispFd <= 0 )
        {
            _FATAL("open /dev/disp failed. errno[%d]:%s", errno, strerror(errno));
        }
        if(initDeviceDisplayInfo(dispFd, dev) != 0)// get screen info from display driver.
        {
            _FATAL("get screen info from dislay driver failed");
        }
        dev->sunxiDispFd = dispFd;
        _LOGV("open /dev/disp success");

        int ionFd = open("/dev/ion",O_RDWR);
        if(ionFd <= 0)
        {
            _FATAL("open /dev/ion failed. errno[%d]:%s", errno, strerror(errno));
        }
        dev->ionFd = ionFd;
        _LOGV("open /dev/ion success");

        // initialize the procs
        dev->device.common.tag      = HARDWARE_DEVICE_TAG;
        dev->device.common.version  = HARDWARE_MODULE_API_VERSION(0, 1);
        dev->device.common.module   = const_cast<hw_module_t*>(module);
        dev->device.common.close    = hwtimewarp_device_close;

        dev->device.enable              = hwtimewarp_device_enable;
        dev->device.setAtwLayer         = hwtimewarp_device_setAtwLayer;
        dev->device.getDisplayInfo      = hwtimewarp_device_getDisplayInfo;
        dev->device.getDisplayVsync     = hwtimewarp_device_getDisplayVsync;
        dev->device.getDisplayTiming    = hwtimewarp_device_getDisplayTiming;

        *device = &dev->device.common;
        status  = 0;
    }
    return status;
}
