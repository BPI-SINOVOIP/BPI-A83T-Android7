#ifndef _HWC_H_
#define _HWC_H_
#include <hardware/hardware.h>
#include <hardware/hwcomposer2.h>
#include <cutils/log.h>
#include <system/graphics.h>
#include <cutils/list.h>
#include <stdlib.h>
#include <sync/sync.h>
#include <sw_sync.h>
#include <linux/ion.h>
#include <utils/String8.h>
#include <utils/String16.h>

/************* Layer And Its operate API ****************/
#define HWC2_COMPOSITION_CLIENT_TARGET 0xFF
typedef struct Layer{
    int32_t compositionType;
    int32_t skipFlag;
    int32_t releaseFence;
    int32_t acquireFence;
    hwc_region_t damageRegion;
    buffer_handle_t buffer;
    int32_t blendMode;
    hwc_color_t color;
    int32_t dataspace;
    hwc_rect_t frame;
    float planeAlpha;
    const native_handle_t* stream;    //this value is setted when the layer is a sideband stream layer
    hwc_frect_t crop;
    int32_t transform;
    hwc_region_t visibleRegion;
    int32_t zorder;
    bool typeChange;
    struct listnode node;
} Layer;

extern void clearListExcepFbTarget(struct listnode *list);
extern void createList(struct listnode *list);
extern void clearList(struct listnode *list);
extern int sizeList(struct listnode *list);
extern bool isEmptyList(struct listnode *list);
extern void insertLayerByZorder(Layer *element, struct listnode *list);
extern bool removeLayer(Layer *element);    //return true when success
extern void showLayer(struct Layer *lay);
/****************************************************************/

/********************Display ************************************/
enum CompositionState {
    INITSTATE,
    VALIDATEDISPLAY,
    ACCEPT_DISPLAY_CHANGES
};

typedef struct DisplayState{
    /*decide the display whether need to re-compose the layers.
     *Set True when these：
     * 1. create layer or distroy layer
     * 2. the geometry attibutes of the layer is changed
     * 3. the plane alpha value is changed
     *
     *Set False when these:
     * 1.in init
     * 2.after the method "presentDisplay" is called
     *
     *When true in one frame,it should try to assign layers.
     *When false in one frame, it do not need to assign layers and the layers' compoistion type keep same as befor frame.
     */
    bool needReCompose;
    /*decide the composing state in every frame
     *When "validate_display" is called, state change to VALIDATEDDISPLAY
     *When "accept display change" or "get changed composition types" is called,
     * and the state is not VALIDATEDISPLAY, the called should return false.
     * or return trye when VALIDATEDISPLAY state.
     *When "present display" is called, state change to INITSTATE
     */
    enum CompositionState compositionState;

}DisplayState;

typedef struct DisplayConfig{
    int width;
    int height;
    int vsyncPeriod;
    int dpiX;
    int dpiY;
}DisplayConfig;

typedef struct Display{
    int displayId;
    char* displayName;
    DisplayConfig *displayConfigList;
    hwc2_config_t configId;
    int configNumber;
    int activeConfigId;
    bool active;
    int32_t colorTransformHint;     //we(DE)do not support color transform,when this value is not 0, should force the layers of this display to be client compose.
    DisplayState displayState;
    unsigned frameCount;
    unsigned preFrameCount;
    pthread_mutex_t mutex;
    int syncTimeLine;
    struct listnode layerSortedByZorder;
}Display;

typedef struct DisplayOpr{
    /*Try to assign layer
     *display: the display target, it contains the layerlist
     *return 1 if any layer's composition type is changed. or 0 if every layer's composition type are not changed.
     */
    int32_t (*tryToAssignLayer)(Display* display);
    /*Display this frame for the display
     *display: the display target, it contains the layerlist
     *return 1 if success
     */
    int32_t (*presentDisplay)(Display* display, int32_t * retireFence);
    /*Dump the message of the Hardware.
     * It will be called when "dumpsys SurfaceFlinger"
     */
    void (*dump)(Display* display, uint32_t* outSize, char* outBuffer);

    /* Init the private data of this display
    * return 0 if success
     */
    int32_t (*init)(Display* display);

    int32_t (*deInit)(Display* display);

    /* Sets the power mode of  the given display.
    * All displays must support HWC2_POWER_MODE_ON and HWC2_POWER_MODE_OFF. Whether a display
    * supports HWC2_POWER_MODE_DOZE or HWC2_POWER_MODE_DOZE_SUSPEND may be queried using
    * getDozeSupport.
    * return 0 if success
    */
    int32_t (*setPowerMode)(Display* display, int32_t mode);

    /*Enables or disables the vsync signal for the given display.
    * return 0 for success setting, or not 0 when the enabled parameter was an invalid value
    */
    int32_t (*setVsyncEnabled)(Display* display, int32_t enabled);
    int32_t (*switchDevice)(Display *display, int type, int mode);
}DisplayOpr;
/*****************************************************************/
#endif
