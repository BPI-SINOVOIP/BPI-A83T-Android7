#pragma once

/*
                RightTop
     ................. -
     .               . |
     .               . |
     .               . |
     .       .       . EyeViewH
     .               . |
     .               . |
     .               . |
     ................. -
LeftBottom
*/
typedef enum{
    EyeView_Scan_From_LeftTop_To_RightBottom = 0,
    EyeView_Scan_From_LeftBottom_To_RightTop,
    EyeView_Scan_Unknown // 很难定义物理横屏，并且是单屏的display， 例如 2560x1440， 扫描宽度是 2560 的那种屏幕。
}scanType_t;

typedef struct{
    bool GlobalShutter;//是否是 global shutter
    bool SplitScreen;//是否物理双屏

    int EyeViewW;//佩戴头盔时，单眼看到的屏幕区域的宽，像素为单位
    int EyeViewH;//佩戴头盔时，单眼看到的屏幕区域的高，像素为单位
    scanType_t scanType;// EyeView区域内，Display 的扫描方向。这个属性影响到初始化 distortionBuffer_t。
}displayInfo_t;

//基础的畸变补偿网格信息。包括左右眼的畸变网格，左右眼的正常网格。
typedef struct
{
    float x;
    float y;
}point_t;
typedef struct
{
    int row;
    int col;
    point_t *inMesh;//点的存储顺序默认是 leftBottom 到 rightTop， 先是左眼的 row*col个点，然后是右眼的 row*col个点。
    point_t *ouMesh;//
}distortionMesh_t;

//输出 blks
typedef struct
{
    int row;
    int col;
    point_t *blks;//left and right is the same.
}atwOuBlks_t;

//timewarp, 输入blks
typedef struct
{
    float EyeStart[2][4];
    float EyeStop[2][4];
}atwWarp_t;
typedef struct
{
    int row;
    int col;
    point_t *blks[2];//left and right maybe not same.
}atwInBlks_t;