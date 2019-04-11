#pragma once

#include <system/window.h> // for buffer_handle_t

#include "AW_Math.h"

#pragma pack(4) // explicit tell compiler we want 4bytes alignment.

typedef struct
{
  buffer_handle_t  hnd_pixel;
  buffer_handle_t  hnd_coeffcient;
}AtwData_t;

typedef enum
{
    EyeTargetLeft = 0,
    EyeTargetRight
}Eye_t;

typedef struct
{
    long long timestamps[2];
    avrQuatf   poses[2];
}WarpData_t;

typedef struct
{
    long long base;
    long long period;
}ServiceTimingData_t;

typedef struct
{
    uint32_t eye;
    uint32_t hint; // whether or not enable color abbreation correction.
    allwinner::Matrix4f mats[2];
}WarpInput_t;

typedef struct
{
    float deltaVsync[2];
    float predictionPoints[2][2];
}swapProgram_t;

typedef enum
{
    DispGlobalShutter = 0, // shader will use another one
    DispRollingShutter
}DispShutterType_t;// 屏幕本身的属性，这将影响到 warp shader 的选择。

typedef enum
{
  /*
      双目视觉观察坐标系: 显示输出屏幕的左上角为 (0,0)， 右下角为 (1,1)
  */
  Normal_Dual_Screen = 0,// eyeview: scan from lefttop to rightbottom
  Left_Right_Single_Screen,
  Up_Bottom_Single_Screen
}DispAtwMode_t;

typedef struct
{
  DispAtwMode_t atw_mode;

  // 双目视觉观察系下，屏幕的宽和高。
  int viewport_w;
  int viewport_h;

  // 提供给应用的 render target 的像素宽高。
  int rendertarget_w;
  int rendertarget_h;

  // 屏幕的实际参数。
  int scn_num;
  int scn_width;
  int scn_height;
  int disableAtw;
}DisplayInfo_t;

typedef struct HardwareVsync
{
    uint64_t vsyncCount;
    uint64_t vsyncBaseNano;
    uint64_t vsyncPeriodsNano;
}HardwareVsync_t;

typedef struct
{
  int64_t timestamp;
  int32_t tconline;

  int32_t tconlineRange[2];// 1080x1920 屏幕，实际扫描行数为 1980 行， 即 [0,1980).
  int32_t vsyncOffset;     // tcon 扫描第一行buffer时(即 vsync 时刻点)， tconline 的 offset
}DisplayTconTiming_t;


#define ATW_DEVICE_MAX_EYE_BUFFER_WIDTH  (1280)
#define ATW_DEVICE_MAX_EYE_BUFFER_HEIGHT (1280)

#define ATW_CONFIG_FORCE_RENDER_LEFT_EYE  (1)  // 用于控制 atw warp {left_eye,right_eye} 的顺序. 这个由屏的贴合方向来决定。

#define ATW_SERVICE_DEBUG_SYS_ATTRIBUTE   "debug.atw.showfps"  // 用于控制是否打印帧率

#define ATW_SERVICE_DEBUG_DYNAMIC_RENDER_FPS   "debug.atw.renderfps"  // 仅限内部使用， 用于控制app渲染帧率。

#define ATW_SERVICE_DEBUG_ENABLE_REPROJECTION  "debug.atw.reprojection" // 仅限内部使用，用于控制是否开启插帧模式。

#pragma pack() // reset to compiler default aligment
