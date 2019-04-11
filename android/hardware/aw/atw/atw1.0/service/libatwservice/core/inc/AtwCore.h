#ifndef _ATW_CORE_H_
#define _ATW_CORE_H_
#include "NE10_macros.h"
#include "NE10_math.h"

#ifdef __cplusplus
extern "C" {
#endif

// matrix is col-major, like opengles.
typedef ne10_vec4f_t vec4;
typedef ne10_vec2f_t vec2;
typedef ne10_mat4x4f_t mat4;

typedef struct
{
    vec2 v[4];
}Quad_t;
struct F32Coefficent
{
    float data[8];
};
struct I32Coefficent
{
    int R[8];
    int G[8];
    int B[8];
    int K[8];
};

typedef struct DispConfig
{
    int eyew;
    int eyeh;
    int shift;
    float factor;
}DispConfig_t;

typedef enum
{
    EYEBUFFER_SCAN_ALONGSIDE_WITH_X_AXIS = 0,
    EYEBUFFER_SCAN_ALONGSIDE_WITH_Y_AXIS
}EyeBufferScanOrder_t;

struct AtwCalculatorNeon;
typedef int (* pFuncInitAtwCalculatorNeon)(struct AtwCalculatorNeon *dev, EyeBufferScanOrder_t order);
typedef int (* pFuncDestroyAtwCalculatorNeon)(struct AtwCalculatorNeon *dev);
typedef int (* pFuncDoTimewarp)(struct AtwCalculatorNeon *dev, mat4 *m, int eye, int32_t *coefficents);
typedef struct AtwCalculatorNeon
{
    DispConfig_t *config;
    int count;
    int tx;
    int ty;

    vec4* src[4]; // once
    vec4* tmp[4];
    vec4* proj[2];
    vec2* transformed[2];

    Quad_t *normQuads; // once

    float *scales;
    struct F32Coefficent *fResults;

    //functions.
    pFuncInitAtwCalculatorNeon Init;
    pFuncDestroyAtwCalculatorNeon Destroy;
    pFuncDoTimewarp DoTimeWarp;
}AtwCalculatorNeon_t;

extern void createAtwCalculatorNeon(struct AtwCalculatorNeon **ppDev, int eyew, int eyeh, int dispw, int disph);
extern void destroyAtwCalculatorNeon(struct AtwCalculatorNeon *pDev);

#ifdef __cplusplus
}
#endif

#endif //_ATW_CORE_H_
