//#define LOG_NDEBUG 0

//include necessary header files here
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "hwc.h"
#include "eventprocess/hwc_event_thread.h"
#include "eventprocess/sync_opt.h"
#include <utils/Trace.h>

extern DisplayOpr de2DisplayOpr;
static DisplayOpr* mDisplayOpr;
static char mDump[4096];
static event_thread_t* mEventThread = NULL;
static hwc2_function_pointer_t mVsyncFunc;
static hwc2_function_pointer_t mHotPlugFunc;
static hwc2_function_pointer_t mRefreshFunc;
#define MAXNUM 2//FIX
static Display mDisplay[MAXNUM];
int g_test_sync;

//#define DEBUG_FUNCTION_CALL ALOGV("%s", __FUNCTION__)
#define DEBUG_FUNCTION_CALL ;

DisplayConfig *find_config(hwc2_config_t config)
{
    return mDisplay[config].displayConfigList;
}

hwc2_config_t find_config(DisplayConfig *config)
{
    for(hwc2_config_t i = 0; i < MAXNUM; i++) {
        if (config == mDisplay[i].displayConfigList) {
            return i;
        }
    }
    return 0;
}

/* hwc_device_getCapabilities(.., outCount, outCapabilities)
 * Provides a list of capabilities(described in the definition of
 * hwc2_capability_t above) supported by this device.This list must
 * not change after the device has been loaded.
 *
 * Parameters:
 *  outCounts - if outCapablities was NULL, the number of capabilities
 *      which would have been returned; if outCapabilities was not NULL,
 *      the number of capabilities returned,which must not exceed the
 *      value stored in outCount prior to the call
 *  outCapabilities - a list of capabilities supported by this device; may
 *      be NULL, in which case this function must write into outCount the
 *      number of capabilities which would have been written into
 *      outCapabilities.
 */
void hwc_device_getCapabilities(struct hwc2_device* device, uint32_t* outCount,
    int32_t* /*hwc2_capability_t*/ outCapabilities){
    DEBUG_FUNCTION_CALL;
    if(outCapabilities == NULL){
        *outCount = 0;
    }else{
        *outCapabilities = HWC2_CAPABILITY_INVALID;
    }
}

int32_t hwc_create_virtual_display(hwc2_device_t* device, uint32_t width, uint32_t height,
    int32_t* format, hwc2_display_t* outDisplay){
    DEBUG_FUNCTION_CALL;
    ALOGE("ERROR %s: do not support virtual display", __FUNCTION__);
    return HWC2_ERROR_NO_RESOURCES;
}

int32_t hwc_destroy_virtual_display(hwc2_device_t* device, hwc2_display_t display){
    DEBUG_FUNCTION_CALL;
    return HWC2_ERROR_NONE;
}

void hwc_dump(hwc2_device_t* device, uint32_t* outSize, char* outBuffer){
    DEBUG_FUNCTION_CALL;

    if(outBuffer == NULL){
        mDisplayOpr->dump(&mDisplay[0], outSize, mDump);
    }else{
        memcpy(outBuffer, mDump, *outSize);
    }
}

uint32_t hwc_get_max_virtual_display_count(hwc2_device_t* device){
    DEBUG_FUNCTION_CALL;
    return 0;
}

//For test
void showlayers(Display* dp){
    struct listnode* list = &(dp->layerSortedByZorder);
    struct listnode *node;
    Layer *ly;
    list_for_each(node, list) {
        ly = node_to_item(node, Layer, node);
        if(ly != NULL){
            showLayer(ly);
        }
    }
    ALOGD("End of show layers.");
}

bool findDisplay(hwc2_display_t display){
    for(int i = 0; i < MAXNUM; i++){
        if(&(mDisplay[i]) == ((Display*)display)) {
            return true;
        }
    }
    return false;
}

int32_t hwc_register_callback(hwc2_device_t* device, int32_t descriptor,
    hwc2_callback_data_t callbackData, hwc2_function_pointer_t pointer){
    DEBUG_FUNCTION_CALL;
    switch(descriptor){
        case HWC2_CALLBACK_HOTPLUG:
        case HWC2_CALLBACK_REFRESH:
        case HWC2_CALLBACK_VSYNC:
            mEventThread->register_event_callback(mEventThread, descriptor,
                                                    callbackData, pointer);
            break;
        default:
            ALOGE("ERROR %s: bad parameter", __FUNCTION__);
            return HWC2_ERROR_BAD_PARAMETER;
    }
    return HWC2_ERROR_NONE;
}

int32_t hwc_accept_display_changes(hwc2_device_t* device, hwc2_display_t display){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)) return HWC2_ERROR_BAD_DISPLAY;
    Display* dp = (Display*)display;

    if(dp->displayState.compositionState != VALIDATEDISPLAY){
        ALOGE("ERROR %s: validate Display has not been called yet", __FUNCTION__);
        return HWC2_ERROR_NOT_VALIDATED;//validateDisplay has not been called.
    }else{
        dp->displayState.needReCompose = false;
        return HWC2_ERROR_NONE;
    }

}

int32_t hwc_create_layer(hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t* outLayer){

    DEBUG_FUNCTION_CALL;

    if(!findDisplay(display)){
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Layer* layer = (Layer*)malloc(sizeof(Layer));

    memset(layer, 0, sizeof(Layer));
    layer->releaseFence = -1;
    layer->acquireFence = -1;
    if(layer == NULL){
        ALOGE("ERROR %s:not enought memory to allow!", __FUNCTION__);
        return HWC2_ERROR_NO_RESOURCES;
    }
    *outLayer = (hwc2_layer_t)layer;
    ALOGV("%s : layer=%p", __FUNCTION__, layer);
    list_init(&(layer->node));
    return HWC2_ERROR_NONE;
}
extern void testHotPlug();

int32_t hwc_destroy_layer(hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*)layer;
    Display *dp = (Display *) display;

    ALOGV("ID=%d, hwc_destroy_layer: layer=%p", dp->displayId, layer);
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    //testHotPlug();
    if(removeLayer(ly)){
        return HWC2_ERROR_NONE;
    }else{
        ALOGE("ERROR %s:bad Layer", __FUNCTION__);
        return HWC2_ERROR_BAD_LAYER;
    }
}

int32_t hwc_get_active_config(hwc2_device_t* device, hwc2_display_t display,
    hwc2_config_t* outConfig){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*) display;

    //*outConfig = (hwc2_config_t)(&(dp->displayConfigList[dp->activeConfigId]));
    *outConfig = (hwc2_config_t)((find_config(&dp->displayConfigList[dp->activeConfigId])));
    return HWC2_ERROR_NONE;
}

int32_t hwc_get_changed_composition_types(hwc2_device_t* device, hwc2_display_t display,
    uint32_t* outNumElements, hwc2_layer_t* outLayers, int32_t* outTypes){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*)display;
    if(dp->displayState.compositionState != VALIDATEDISPLAY) {
        ALOGE("ERROR %s:not validated", __FUNCTION__);
        return HWC2_ERROR_NOT_VALIDATED;
    }

    int num = 0;
    struct listnode *list = &(dp->layerSortedByZorder);
    struct listnode *node;
    Layer *ly;

    if(outLayers == NULL | outTypes == NULL){
        list_for_each(node, list) {
            ly = node_to_item(node, Layer, node);
            if (ly != NULL
                && ly->typeChange
                && ly->compositionType != HWC2_COMPOSITION_CLIENT_TARGET){
                num++;
            }
        }
        *outNumElements = num;
        return HWC2_ERROR_NONE;
    }else{
        list_for_each(node, list) {
            ly = node_to_item(node, struct Layer, node);
            if(ly != NULL && ly->typeChange && ly->compositionType != HWC2_COMPOSITION_CLIENT_TARGET){
                *outLayers = (hwc2_layer_t)ly;
                outLayers++;
                *outTypes = ly->compositionType;
                outTypes++;
                ALOGV("%s: layer=%p, type=%d", __FUNCTION__, ly, ly->compositionType);
            }
        }
        return HWC2_ERROR_NONE;
    }
}

//FixMe: not all the dataspace is support by de.
int32_t hwc_get_client_target_support(hwc2_device_t* device, hwc2_display_t display,
    uint32_t width, uint32_t height, int32_t format, int32_t dataspace){
    DEBUG_FUNCTION_CALL;
    ALOGV("%s display=%p, width=%d, height=%d, format=%d, dataspace=%x",
        __FUNCTION__, display, width, height, format, dataspace);
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    if(format > HAL_PIXEL_FORMAT_BGRA_8888) {
        ALOGE("ERROR %s:unsupported", __FUNCTION__);
        return HWC2_ERROR_UNSUPPORTED;
    }
    return HWC2_ERROR_NONE;
}

//FixMe: androidN do not use this API，and the <system/graphics.h> don't have the struct android_color_mode_t
int32_t hwc_get_color_modes(hwc2_device_t* device, hwc2_display_t display, uint32_t* outNumModes,
    int32_t* outModes){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    if (!outModes){
        *outNumModes = 1;
        return HWC2_ERROR_NONE;
    }else{
        *outModes = 1;

    }
    return HWC2_ERROR_NONE;
}

int32_t hwc_get_display_attribute(hwc2_device_t* device, hwc2_display_t display,
    hwc2_config_t config, int32_t attribute, int32_t* outValue){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*) display;

    for(int i = 0; i < dp->configNumber; i++){
        //if(&(dp->displayConfigList[i]) == ((DisplayConfig*)config)){
        if(&(dp->displayConfigList[i]) == ((DisplayConfig*)find_config(config))){
            //DisplayConfig* cfg = (DisplayConfig*)config;
            DisplayConfig* cfg = (DisplayConfig*)find_config(config);
            switch (attribute) {
                case HWC2_ATTRIBUTE_WIDTH:
                    *outValue = cfg->width;
                    ALOGV("%s:width=%d", __FUNCTION__, cfg->width);
                    break;
                case HWC2_ATTRIBUTE_HEIGHT:
                    ALOGV("%s:height=%d", __FUNCTION__, cfg->height);
                    *outValue = cfg->height;
                    break;
                case HWC2_ATTRIBUTE_VSYNC_PERIOD:
                    ALOGV("%s:vsyncPeriod=%d", __FUNCTION__, cfg->vsyncPeriod);
                    *outValue = cfg->vsyncPeriod;
                    break;
                case HWC2_ATTRIBUTE_DPI_X:
                    ALOGV("%s:dpiX=%d", __FUNCTION__, cfg->dpiX);
                    *outValue = cfg->dpiX;
                    break;
                case HWC2_ATTRIBUTE_DPI_Y:
                    ALOGV("%s:dpiY=%d", __FUNCTION__, cfg->dpiY);
                    *outValue = cfg->dpiY;
                    break;
                default:
                    return HWC2_ERROR_BAD_CONFIG;
            }
            return HWC2_ERROR_NONE;
        }
    }
    ALOGE("ERROR %s:bad config", __FUNCTION__);
    return HWC2_ERROR_BAD_CONFIG;

}

int32_t hwc_get_display_configs(hwc2_device_t* device, hwc2_display_t display,
    uint32_t* outNumConfigs, hwc2_config_t* outConfigs){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*) display;

    if(outConfigs == NULL){
        *outNumConfigs = dp->configNumber;
    }else{
        for(int i = 0; i < *outNumConfigs; i++){
            *outConfigs = find_config(&(dp->displayConfigList[i]));
            //*outConfigs = (hwc2_config_t)(&(dp->displayConfigList[i]));
            outConfigs++;
        }
    }
    return HWC2_ERROR_NONE;
}

int32_t hwc_get_display_name(hwc2_device_t* device, hwc2_display_t display, uint32_t* outSize,
    char* outName){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*) display;

    if(outName == NULL){
        *outSize = strlen(dp->displayName);
    }else{
        strncpy(outName, dp->displayName, *outSize);
    }
    return HWC2_ERROR_NONE;
}

int32_t hwc_get_display_requests(hwc2_device_t* device, hwc2_display_t display,
    int32_t* outDisplayRequests, uint32_t* outNumElements, hwc2_layer_t* outLayers,
    int32_t* outLayerRequests){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)) {
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*)display;

    if(dp->displayState.compositionState != VALIDATEDISPLAY){
        ALOGE("ERROR %s:not validated", __FUNCTION__);
        return HWC2_ERROR_NOT_VALIDATED;
    }
    *outDisplayRequests = HWC2_DISPLAY_REQUEST_FLIP_CLIENT_TARGET;
    struct listnode *list = &(dp->layerSortedByZorder);
    struct listnode *node;
    Layer *ly;
    int num = 0;
    if(outLayers == NULL || outLayerRequests == NULL){
        /*
        list_for_each(node, list) {
            ly = node_to_item(node, Layer, node);
            if (ly != NULL && ly->compositionType == HWC2_COMPOSITION_CLIENT){
                num++;
            }
        }
        *outNumElements = num;
        */
        *outNumElements = 0;
        return HWC2_ERROR_NONE;
    }else{
        /*
        list_for_each(node, list) {
            ly = node_to_item(node, struct Layer, node);
            if(ly->compositionType == HWC2_COMPOSITION_CLIENT_TARGET){
                continue;
            }
            *outLayers = (hwc2_layer_t)ly;
            outLayers++;
            *outLayerRequests = HWC2_LAYER_REQUEST_CLEAR_CLIENT_TARGET;
            outLayerRequests++;
        }
        */

        return HWC2_ERROR_NONE;
    }
}

int32_t hwc_get_display_type(hwc2_device_t* device, hwc2_display_t display, int32_t* outType){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    *outType = HWC2_DISPLAY_TYPE_PHYSICAL;
    return HWC2_ERROR_NONE;
}

int32_t hwc_get_doze_support(hwc2_device_t* device, hwc2_display_t display, int32_t* outSupport){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    *outSupport = 0;
    return HWC2_ERROR_NONE;
}

int32_t hwc_get_hdr_capabilities(hwc2_device_t* device, hwc2_display_t display,
    uint32_t* outNumTypes,int32_t* outTypes, float* outMaxLuminance,
    float* outMaxAverageLuminace, float* outMinLuminance){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    if(outTypes == NULL){
        *outNumTypes = 0;
    }
    return HWC2_ERROR_NONE;
}

int32_t hwc_get_release_fences(hwc2_device_t* device, hwc2_display_t display,
    uint32_t* outNumElements, hwc2_layer_t* outLayers, int32_t* outFences){
    DEBUG_FUNCTION_CALL;
    Display* dp = (Display*)display;
    struct listnode *list = &(dp->layerSortedByZorder);
    struct listnode *node;
    Layer *ly;
    hwc2_layer_t* retLy = outLayers;
    int32_t* retFence = outFences;

    if(!findDisplay(display)){
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if(outLayers == NULL || outFences == NULL){
        int hasFbLy = 0;
        list_for_each(node, list) {
            ly = node_to_item(node, struct Layer, node);
            if(ly->compositionType == HWC2_COMPOSITION_CLIENT_TARGET){
                hasFbLy = 1;
                break;
            }
        }
        *outNumElements = sizeList(list) - hasFbLy;
        ALOGV("%s : ID=%d, outNumElements=%d", __FUNCTION__, dp->displayId, *outNumElements);
    }else{

        list_for_each(node, list) {
            ly = node_to_item(node, struct Layer, node);
            if(ly->compositionType == HWC2_COMPOSITION_CLIENT_TARGET){
                continue;
            }
            *outLayers = (hwc2_layer_t)ly;
            outLayers++;

            if (ly->releaseFence >= 0) {
                *outFences = dup(ly->releaseFence);
                close(ly->releaseFence);
                ly->releaseFence = -1;
            } else {
                *outFences = -1;
            }
            outFences++;
            ALOGV("%s : ID=%d, layer=%p, releaseFence=%d", __FUNCTION__, dp->displayId, ly, ly->releaseFence);
        }
    }
    return HWC2_ERROR_NONE;
}

bool waitGPUWorkFinish(int32_t acquirefence, int timeout){
    ATRACE_CALL();
    return sync_wait(acquirefence, timeout);
}

int32_t hwc_present_display(hwc2_device_t* device, hwc2_display_t display,
    int32_t* outRetireFence){
    DEBUG_FUNCTION_CALL;

    if(!findDisplay(display)){
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    static unsigned presentCnt = 0;

    Display* dp = (Display*) display;
    struct listnode *list = &(dp->layerSortedByZorder);
    struct listnode *node;
    Layer *ly;

    //wait Buffer Write finish
#if 0
    list_for_each(node, list) {
        ly = node_to_item(node, struct Layer, node);
        if(ly->compositionType == HWC2_COMPOSITION_DEVICE ||
            ly->compositionType == HWC2_COMPOSITION_CLIENT_TARGET){
            if(ly->acquireFence >= 0 && !waitGPUWorkFinish(ly->acquireFence, 3000)){
                continue;
            }else if(ly->acquireFence >= 0){
                ALOGE("WARNING %s : layer %p wait acquirefence %d timeout", __FUNCTION__, ly,
                        ly->acquireFence);
                continue;
            }
        }
    }

#endif
    mDisplayOpr->presentDisplay(dp, outRetireFence);

#if 0
    list_for_each(node, list){
        ly = node_to_item(node, struct Layer, node);
        if((ly->acquireFence >= 0)
            /*&& ((ly->compositionType == HWC2_COMPOSITION_DEVICE) ||
                 (ly->compositionType == HWC2_COMPOSITION_CLIENT_TARGET)*/){
            close(ly->acquireFence);
            ly->acquireFence = -1;
        }
    }
#endif

#if 0
    pthread_mutex_lock(&dp->mutex);
    ALOGV("timeline=%d, frameCount=%d", dp->syncTimeLine, dp->frameCount);
    list_for_each(node, list){
        ly = node_to_item(node, struct Layer, node);
        if(ly->compositionType == HWC2_COMPOSITION_DEVICE){
            ly->releaseFence = sw_sync_fence_create(dp->syncTimeLine, "hwc2", dp->frameCount + 1);
            if (ly->releaseFence < 0 ) {
                ALOGV("releaseFence create failed!\n");
                ly->releaseFence = -1;
            }
        }else{
            ly->releaseFence = -1;
        }
    }
#endif

#if 0
    if (dp->lastCount > 0) {
        sw_sync_timeline_inc(dp->syncTimeLine, dp->lastCount);
    }

    if (dp->frameCount - (int)presentCnt > 0) {
        //ALOGD("vsync%d callback ok . cnt = %d, vc= %d\n",vsync, dp->frameCount, device->presentCnt);
        dp->lastCount = dp->frameCount - (int)presentCnt;
        //ALOGD("LASTCOUNT = %d", dp->lastCount);
    } else {
        dp->lastCount = 0;
    }
    presentCnt = dp->frameCount;

    dp->frameCount = (dp->frameCount) + 1;
    ALOGV("vsync %s: framecnt = %d, cur-pre= %d", __FUNCTION__, dp->frameCount, dp->frameCount - dp->preFrameCount);
    dp->preFrameCount = dp->frameCount;
#endif
    pthread_mutex_unlock(&dp->mutex);
    dp->displayState.compositionState = INITSTATE;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_active_config(hwc2_device_t* device, hwc2_display_t display,
    hwc2_config_t config){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*) display;
    DisplayConfig* cfg = find_config(config);
    //DisplayConfig* cfg = (DisplayConfig*) config;

    for(int i = 0; i < dp->configNumber; i++){
        if(cfg == &(dp->displayConfigList[i])){
            ALOGV("%s: set active config %p, configid=%d", __FUNCTION__, cfg, i);
            dp->activeConfigId = i;
            return HWC2_ERROR_NONE;
        }
    }
    ALOGE("ERROR %s:bad config", __FUNCTION__);
    return HWC2_ERROR_BAD_CONFIG;
}

int32_t hwc_set_client_target(hwc2_device_t* device, hwc2_display_t display,
    buffer_handle_t target, int32_t acquireFence, int32_t dataspace, hwc_region_t damage){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*)display;
    struct listnode *list = &(dp->layerSortedByZorder);
    struct listnode *node;
    Layer *ly;

    list_for_each(node, list) {
        ly = node_to_item(node, struct Layer, node);
        if(ly->compositionType == HWC2_COMPOSITION_CLIENT_TARGET){
           // showLayer(ly);
            ly->buffer = target;
            ly->acquireFence = acquireFence;
            ly->dataspace = dataspace;
            ly->damageRegion = damage;
            ALOGV("%s : update FBLayer=%p, acquireFence=%ld, dataspace%d, buffer=%p, target=%p",
                __FUNCTION__, ly, acquireFence, dataspace, ly->buffer, target);
            return HWC2_ERROR_NONE;
        }
    }
    //if there is not client target in the layerlist,create it and insert it to the list
    Layer *layer = (Layer*)malloc(sizeof(Layer));
    if (layer == NULL) {
        ALOGE("create layer failed!");
        return -1;
    }
    memset(layer, 0, sizeof(Layer));
    layer->compositionType = HWC2_COMPOSITION_CLIENT_TARGET;
    layer->acquireFence = acquireFence;
    layer->releaseFence = -1;
    layer->buffer = target;
    layer->crop.left = 0;
    layer->crop.right = dp->displayConfigList[dp->activeConfigId].width;
    layer->crop.top = 0;
    layer->crop.bottom = dp->displayConfigList[dp->activeConfigId].height;
    layer->damageRegion = damage;
    layer->dataspace = dataspace;
    layer->frame.left = 0;
    layer->frame.right = dp->displayConfigList[dp->activeConfigId].width;
    layer->frame.top = 0;
    layer->frame.bottom = dp->displayConfigList[dp->activeConfigId].height;
    layer->transform = 0;
    layer->typeChange = false;
    layer->zorder = -1;
    list_init(&(layer->node));
    insertLayerByZorder(layer, list);

    ALOGV("%s : create FbLayer=%p, acquirefence=%ld, dataspace=%d, buffer=%p, target=%p",
        __FUNCTION__, layer, acquireFence, dataspace, layer->buffer, target);
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_color_mode(hwc2_device_t* device, hwc2_display_t display, int32_t mode){
    //Fix Me:cannot find struct android_color_mode_t in <system/graphics.h>
    DEBUG_FUNCTION_CALL;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_color_transform(hwc2_device_t* device, hwc2_display_t display,
    const float* matrix, int32_t hint){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("ERROR %s:bad display", __FUNCTION__);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*)display;

    dp->colorTransformHint = hint;
    return HWC2_ERROR_NONE;

}

int32_t hwc_set_output_buffer(hwc2_device_t* device, hwc2_display_t display,
    buffer_handle_t buffer, int32_t releaseFence){
    DEBUG_FUNCTION_CALL;
    ALOGE("%s : we do not support virtual display yet,should not be called", __FUNCTION__);
    return HWC2_ERROR_UNSUPPORTED;
}

int32_t hwc_set_power_mode(hwc2_device_t* device, hwc2_display_t display, int32_t mode){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("%s : bad display %p", __FUNCTION__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    //ToDo:release all fence and do not accept frame.

    //call blank/unblank api of driver.
    Display* dp = (Display*) display;
    if(!mDisplayOpr->setPowerMode(dp, mode)){
        return HWC2_ERROR_NONE;
    }
    return HWC2_ERROR_UNSUPPORTED;
}

int32_t hwc_set_vsync_enabled(hwc2_device_t* device, hwc2_display_t display, int32_t enabled){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("%s : bad display %p", __FUNCTION__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    //for test
    ALOGV("hwc_set_vsync_enabled %d", enabled);
    mEventThread->set_vsync_enabled(mEventThread, display, (hwc2_vsync_t)enabled);
    if(!mDisplayOpr->setVsyncEnabled((Display*)display, enabled)){
        return HWC2_ERROR_NONE;
    }
    return HWC2_ERROR_BAD_PARAMETER;
}

int32_t hwc_validate_display(hwc2_device_t* device, hwc2_display_t display,
    uint32_t* outNumTypes, uint32_t* outNumRequests){
    DEBUG_FUNCTION_CALL;
    if(!findDisplay(display)){
        ALOGE("%s : bad display %p", __FUNCTION__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    Display* dp = (Display*) display;
    dp->displayState.compositionState = VALIDATEDISPLAY;
    struct listnode* list = &(dp->layerSortedByZorder);
    struct listnode *node;
    Layer *ly;
    uint32_t numTypes = 0;
    *outNumRequests = 0;
    //test
    if (!g_test_sync)
            g_test_sync++;

    ALOGV("ID=%d, Before assign, size=%d", dp->displayId, sizeList(&(dp->layerSortedByZorder)));
    //showlayers(dp);
    if(dp->displayState.needReCompose || true){ //fix
        if(mDisplayOpr->tryToAssignLayer(dp)){
            ALOGV("After assign");
            //showlayers(dp);
            list = &(dp->layerSortedByZorder);
            list_for_each(node, list) {
                ly = node_to_item(node, Layer, node);
                if(ly != NULL && ly->typeChange && ly->compositionType != HWC2_COMPOSITION_CLIENT_TARGET){
                    numTypes++;
                }
            }
            *outNumTypes = numTypes;
            ALOGV("%s: %d layer changes.\n",__FUNCTION__, numTypes);
            return HWC2_ERROR_HAS_CHANGES;
        }else{
            return HWC2_ERROR_NONE;
        }
    }else{
        return HWC2_ERROR_NONE;
    }
}

int32_t hwc_set_cursor_position(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, int32_t x, int32_t y){
    DEBUG_FUNCTION_CALL;
    ALOGE("%s : (Warning) we do not support cursor layer alone.", __FUNCTION__);
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_buffer(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, buffer_handle_t buffer, int32_t acquireFence){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*)layer;
    Display* dp = (Display*)display;


    ly->buffer = buffer;
    ly->acquireFence = acquireFence;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_surface_damage(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, hwc_region_t damage){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->damageRegion = damage;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_blend_mode(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, int32_t mode){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->blendMode = mode;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_color(hwc2_display_t* device, hwc2_display_t display, hwc2_layer_t layer,
    hwc_color_t color){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->color = color;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_composition_type(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, int32_t type){
    Layer* ly = (Layer*) layer;

    switch (type) {
    case HWC2_COMPOSITION_CLIENT:
        ly->skipFlag = 1;
        break;
    default:
        ly->skipFlag = 0;
        break;
    }
    ly->compositionType = type;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_dataspace(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, int32_t dataspace){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->dataspace = dataspace;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_display_frame(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, hwc_rect_t frame){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->frame = frame;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_plane_alpha(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, float alpha){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->planeAlpha = alpha;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_sideband_stream(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, const native_handle_t* stream){
    DEBUG_FUNCTION_CALL;
    ALOGV("%s : (Warning)do not support sideband stream layer yet!", __FUNCTION__);
    Layer* ly = (Layer*) layer;

    ly->stream = stream;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_source_crop(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, hwc_frect_t crop){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->crop = crop;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_transform(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, int32_t transform){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->transform = transform;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_visible_region(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, hwc_region_t visible){
    DEBUG_FUNCTION_CALL;
    Layer* ly = (Layer*) layer;

    ly->visibleRegion = visible;
    return HWC2_ERROR_NONE;
}

int32_t hwc_set_layer_z_order(hwc2_device_t* device, hwc2_display_t display,
    hwc2_layer_t layer, uint32_t z){
    Layer* ly = (Layer*) layer;

    ly->zorder = z;
    Display* dp = (Display*) display;
    struct listnode* list = &(dp->layerSortedByZorder);

    insertLayerByZorder(ly, list);
    ALOGV("ID=%d, INSERT lay =%p", dp->displayId, ly);
    return HWC2_ERROR_NONE;
}

/* hwc_device_getFunction(..., descriptor)
 * Returns a function pointer which implements the requested description
 *
 * Parameters:
 *  descriptor - the function to return
 * Returns either a function pointer implementing the requested descriptor
 *  or NULL if the described function is not supported by this device.
 */
template <typename PFN, typename T>
static hwc2_function_pointer_t asFP(T function){
    return reinterpret_cast<hwc2_function_pointer_t>(function);
}

hwc2_function_pointer_t hwc_device_getFunction(struct hwc2_device* device,
    int32_t /*hwc2_function_descriptor_t*/ descriptor){
    ALOGV("%s: descriptor=%d", __FUNCTION__, descriptor);
    switch(descriptor){
    case HWC2_FUNCTION_ACCEPT_DISPLAY_CHANGES:
        return asFP<HWC2_PFN_ACCEPT_DISPLAY_CHANGES>(
            hwc_accept_display_changes);
    case HWC2_FUNCTION_CREATE_LAYER:
        return asFP<HWC2_PFN_CREATE_LAYER>(
            hwc_create_layer);
    case HWC2_FUNCTION_CREATE_VIRTUAL_DISPLAY:
        return asFP<HWC2_PFN_CREATE_VIRTUAL_DISPLAY>(
            hwc_create_virtual_display);
    case HWC2_FUNCTION_DESTROY_LAYER:
        return asFP<HWC2_PFN_DESTROY_LAYER>(
            hwc_destroy_layer);
    case HWC2_FUNCTION_DESTROY_VIRTUAL_DISPLAY:
        return asFP<HWC2_PFN_DESTROY_VIRTUAL_DISPLAY>(
            hwc_destroy_virtual_display);
    case HWC2_FUNCTION_DUMP:
        return asFP<HWC2_PFN_DUMP>(hwc_dump);
    case HWC2_FUNCTION_GET_ACTIVE_CONFIG:
        return asFP<HWC2_PFN_GET_ACTIVE_CONFIG>(
            hwc_get_active_config);
    case HWC2_FUNCTION_GET_CHANGED_COMPOSITION_TYPES:
        return asFP<HWC2_PFN_GET_CHANGED_COMPOSITION_TYPES>(
            hwc_get_changed_composition_types);
    case HWC2_FUNCTION_GET_CLIENT_TARGET_SUPPORT:
        return asFP<HWC2_PFN_GET_CLIENT_TARGET_SUPPORT>(
            hwc_get_client_target_support);
    case HWC2_FUNCTION_GET_COLOR_MODES:
        return asFP<HWC2_PFN_GET_COLOR_MODES>(
            hwc_get_color_modes);
    case HWC2_FUNCTION_GET_DISPLAY_ATTRIBUTE:
        return asFP<HWC2_PFN_GET_DISPLAY_ATTRIBUTE>(
            hwc_get_display_attribute);
    case HWC2_FUNCTION_GET_DISPLAY_CONFIGS:
        return asFP<HWC2_PFN_GET_DISPLAY_CONFIGS>(
            hwc_get_display_configs);
    case HWC2_FUNCTION_GET_DISPLAY_NAME:
        return asFP<HWC2_PFN_GET_DISPLAY_NAME>(
            hwc_get_display_name);
    case HWC2_FUNCTION_GET_DISPLAY_REQUESTS:
        return asFP<HWC2_PFN_GET_DISPLAY_REQUESTS>(
            hwc_get_display_requests);
    case HWC2_FUNCTION_GET_DISPLAY_TYPE:
        return asFP<HWC2_PFN_GET_DISPLAY_TYPE>(
            hwc_get_display_type);
    case HWC2_FUNCTION_GET_DOZE_SUPPORT:
        return asFP<HWC2_PFN_GET_DOZE_SUPPORT>(
            hwc_get_doze_support);
    case HWC2_FUNCTION_GET_HDR_CAPABILITIES:
        return asFP<HWC2_PFN_GET_HDR_CAPABILITIES>(
            hwc_get_hdr_capabilities);
    case HWC2_FUNCTION_GET_MAX_VIRTUAL_DISPLAY_COUNT:
        return asFP<HWC2_PFN_GET_MAX_VIRTUAL_DISPLAY_COUNT>(
            hwc_get_max_virtual_display_count);
    case HWC2_FUNCTION_GET_RELEASE_FENCES:
        return asFP<HWC2_PFN_GET_RELEASE_FENCES>(
            hwc_get_release_fences);
    case HWC2_FUNCTION_PRESENT_DISPLAY:
        return asFP<HWC2_PFN_PRESENT_DISPLAY>(
            hwc_present_display);
    case HWC2_FUNCTION_REGISTER_CALLBACK:
        return asFP<HWC2_PFN_REGISTER_CALLBACK>(
            hwc_register_callback);
    case HWC2_FUNCTION_SET_ACTIVE_CONFIG:
        return asFP<HWC2_PFN_SET_ACTIVE_CONFIG>(
            hwc_set_active_config);
    case HWC2_FUNCTION_SET_CLIENT_TARGET:
        return asFP<HWC2_PFN_SET_CLIENT_TARGET>(
            hwc_set_client_target);
    case HWC2_FUNCTION_SET_COLOR_MODE:
        return asFP<HWC2_PFN_SET_COLOR_MODE>(
            hwc_set_color_mode);
    case HWC2_FUNCTION_SET_COLOR_TRANSFORM:
        return asFP<HWC2_PFN_SET_COLOR_TRANSFORM>(
            hwc_set_color_transform);
    case HWC2_FUNCTION_SET_CURSOR_POSITION:
        return asFP<HWC2_PFN_SET_CURSOR_POSITION>(
            hwc_set_cursor_position);
    case HWC2_FUNCTION_SET_LAYER_BLEND_MODE:
        return asFP<HWC2_PFN_SET_LAYER_BLEND_MODE>(
            hwc_set_layer_blend_mode);
    case HWC2_FUNCTION_SET_LAYER_BUFFER:
        return asFP<HWC2_PFN_SET_LAYER_BUFFER>(
            hwc_set_layer_buffer);
    case HWC2_FUNCTION_SET_LAYER_COLOR:
        return asFP<HWC2_PFN_SET_LAYER_COLOR>(
            hwc_set_layer_color);
    case HWC2_FUNCTION_SET_LAYER_COMPOSITION_TYPE:
        return asFP<HWC2_PFN_SET_LAYER_COMPOSITION_TYPE>(
            hwc_set_layer_composition_type);
    case HWC2_FUNCTION_SET_LAYER_DATASPACE:
        return asFP<HWC2_PFN_SET_LAYER_DATASPACE>(
            hwc_set_layer_dataspace);
    case HWC2_FUNCTION_SET_LAYER_DISPLAY_FRAME:
        return asFP<HWC2_PFN_SET_LAYER_DISPLAY_FRAME>(
            hwc_set_layer_display_frame);
    case HWC2_FUNCTION_SET_LAYER_PLANE_ALPHA:
        return asFP<HWC2_PFN_SET_LAYER_PLANE_ALPHA>(
            hwc_set_layer_plane_alpha);
    case HWC2_FUNCTION_SET_LAYER_SIDEBAND_STREAM:
        return asFP<HWC2_PFN_SET_LAYER_SIDEBAND_STREAM>(
            hwc_set_layer_sideband_stream);
    case HWC2_FUNCTION_SET_LAYER_SOURCE_CROP:
        return asFP<HWC2_PFN_SET_LAYER_SOURCE_CROP>(
            hwc_set_layer_source_crop);
    case HWC2_FUNCTION_SET_LAYER_SURFACE_DAMAGE:
        return asFP<HWC2_PFN_SET_LAYER_SURFACE_DAMAGE>(
            hwc_set_layer_surface_damage);
    case HWC2_FUNCTION_SET_LAYER_TRANSFORM:
        return asFP<HWC2_PFN_SET_LAYER_TRANSFORM>(
            hwc_set_layer_transform);
    case HWC2_FUNCTION_SET_LAYER_VISIBLE_REGION:
        return asFP<HWC2_PFN_SET_LAYER_VISIBLE_REGION>(
            hwc_set_layer_visible_region);
    case HWC2_FUNCTION_SET_LAYER_Z_ORDER:
        return asFP<HWC2_PFN_SET_LAYER_Z_ORDER>(
            hwc_set_layer_z_order);
    case HWC2_FUNCTION_SET_OUTPUT_BUFFER:
        return asFP<HWC2_PFN_SET_OUTPUT_BUFFER>(
            hwc_set_output_buffer);
    case HWC2_FUNCTION_SET_POWER_MODE:
        return asFP<HWC2_PFN_SET_POWER_MODE>(
            hwc_set_power_mode);
    case HWC2_FUNCTION_SET_VSYNC_ENABLED:
        return asFP<HWC2_PFN_SET_VSYNC_ENABLED>(
            hwc_set_vsync_enabled);
    case HWC2_FUNCTION_VALIDATE_DISPLAY:
        return asFP<HWC2_PFN_VALIDATE_DISPLAY>(
            hwc_validate_display);
    }
    return NULL;
}

int hwc_device_close(struct hw_device_t* device){
    return 0;
}

void deviceManger(hwc2_display_t display, bool hotplug)
{
    Display* dp = (Display*)display;
    int mode;
    unsigned long arg[4] = {0};

    if(!findDisplay(display)){
        ALOGE("%s : bad display %p", __FUNCTION__, display);
        return;
    }

    if (hotplug) {
        mDisplayOpr->switchDevice(dp, 4, 10);
        if (!mDisplay[dp->displayId].active) {
            /* first init, create timeline. */
            mDisplay[dp->displayId].syncTimeLine = sw_sync_timeline_create();
            mDisplay[dp->displayId].active = 1;
        }
        ALOGV("syncTimeLine=%d, err=%d", mDisplay[dp->displayId].syncTimeLine, errno);
        mDisplay[dp->displayId].activeConfigId = 0;
        mDisplayOpr->init(&mDisplay[dp->displayId]);
    } else {
        mDisplayOpr->switchDevice(dp, 0, 0);
        mDisplayOpr->deInit(dp);
    }
}

static int hwc_device_open(const struct hw_module_t* module, const char* id,
    struct hw_device_t** device){
    hwc2_device_t* hwcDevice;
    hw_device_t* hwDevice;
    int err = 0;

    if(strcmp(id, HWC_HARDWARE_COMPOSER)){
        return -EINVAL;
    }
    hwcDevice = (hwc2_device_t*)malloc(sizeof(hwc2_device_t));
    if(!hwcDevice){
        ALOGE("%s: Failed to allocate memory", __func__);
        return -ENOMEM;
    }
    memset(hwcDevice, 0, sizeof(hwc2_device_t));
    hwDevice = (hw_device_t*)hwcDevice;
    //0.Init the hwcomposer API
    hwcDevice->common.tag = HARDWARE_DEVICE_TAG;
    hwcDevice->common.version = HWC_DEVICE_API_VERSION_2_0;
    hwcDevice->common.module = const_cast<hw_module_t*>(module);
    hwcDevice->common.close = hwc_device_close;
    hwcDevice->getCapabilities = hwc_device_getCapabilities;
    hwcDevice->getFunction = hwc_device_getFunction;
    *device = hwDevice;
    //1.Init the data about the Hardware related and display data.
#if(SW_CHIP_PLATFORM == SW_KYLIN)
    mDisplayOpr= &de2DisplayOpr;
#else
#error "please select a platform\n"
#endif
    for(int i = 0; i < MAXNUM; i++){
        memset(&mDisplay[i], 0, sizeof(Display));
        createList(&(mDisplay[i].layerSortedByZorder));
        mDisplay[i].displayId = i;
        mDisplay[i].configNumber = 1;
        mDisplay[i].displayConfigList = (DisplayConfig*) malloc(mDisplay[i].configNumber *sizeof(DisplayConfig));
        mDisplay[i].configId = i;
        mDisplay[i].frameCount = 0;
        mDisplay[i].preFrameCount = 0;
        pthread_mutex_init(&mDisplay[i].mutex, 0);
        //test
        if (i < 1) {
            /* init main device first. */
            mDisplay[i].syncTimeLine = sw_sync_timeline_create();
            ALOGV("syncTimeLine=%d, err=%d", mDisplay[i].syncTimeLine, errno);
            mDisplay[i].activeConfigId = 0;
            mDisplayOpr->init(&mDisplay[i]);
        }
    }

    //2.init the event processing module
    mEventThread = event_thread_create();
    mEventThread->register_hotplug_callback(mEventThread,
        reinterpret_cast<hwc2_function_pointer_t>(deviceManger));

    mEventThread->start(mEventThread, "event process module", HAL_PRIORITY_URGENT_DISPLAY);
    for(int i = 0; i < MAXNUM; i++){
        //ForTMP:the hotplug message should send by display HAL in the future.
        if(mDisplay[i].displayId == 0){
            mEventThread->set_hotplug_attribute(mEventThread, (hwc2_display_t)(&mDisplay[i]),
                HOTPLUG_TYPE_NONE, NULL);
        }else if(mDisplay[i].displayId > 0){
            mEventThread->set_hotplug_attribute(mEventThread, (hwc2_display_t)(&mDisplay[i]),
                HOTPLUG_TYPE_SWITCH_EVENT, "hdmi");
        }

        ALOGD("init display[%d] = %p", i, &mDisplay[i]);
    }
    //3.init fence opr module
    mEventThread->register_global_vsync_callback(mEventThread,
        reinterpret_cast<hwc2_function_pointer_t>(sync_opt_vsync_handler));

    return err;
}

//define the module methods
static struct hw_module_methods_t hwc_module_methods = {
    .open = hwc_device_open,
};

//define the entry point of the module
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = HWC_HARDWARE_MODULE_ID,
    .name = "Allwinner Hwcomposer Module",
    .author = "chenjd/zq",
    .methods = &hwc_module_methods,
};



