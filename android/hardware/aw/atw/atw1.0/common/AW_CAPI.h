#ifndef _AW_CAPI_H_
#define _AW_CAPI_H_

typedef struct avrSizei_
{
    int w, h;
} avrSizei;

// 3D
typedef struct avrQuatf_
{
    float x, y, z, w;
} avrQuatf;
typedef struct avrVector2f_
{
    float x, y;
} avrVector2f;
typedef struct avrVector3f_
{
    float x, y, z;
} avrVector3f;
typedef struct avrMatrix4f_
{
    float M[4][4];
} avrMatrix4f;
// Position and orientation together.
typedef struct avrPosef_
{
    avrQuatf     Orientation;
    avrVector3f  Position;
} avrPosef;

// Full pose (rigid body) configuration with first and second derivatives.
typedef struct avrPoseStatef_
{
    avrPosef     Pose;
    avrVector3f  AngularVelocity;
    avrVector3f  LinearVelocity;
    avrVector3f  AngularAcceleration;
    avrVector3f  LinearAcceleration;
    double       TimeInSeconds;         // Absolute time of this state sample.
} avrPoseStatef;

#endif //_AW_CAPI_H_
