#ifndef _VRDESKTOP_CONTROLLERCURSOR
#define _VRDESKTOP_CONTROLLERCURSOR

#include <ui/mat4.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../Program.h"
#include "../ProgramCache.h"
#include "../Description.h"
#include "../Mesh.h"
#include "../Texture.h"

using namespace android;

typedef struct vf4 {
    float x;
    float y;
    float z;
    float w;
} vf4_t;

typedef struct vf3 {
    float x;
    float y;
    float z;
} vf3_t;

typedef struct vf2 {
    float x;
    float y;
} vf2_t;

typedef unsigned short TriangleIndex;


class ControllerCursor{
public:
    ControllerCursor(float r, float l, float h, float v);
    ~ControllerCursor();

    void create();
    void drawSelf(mat4 mvp1, mat4 mvp2, GLuint mVpWidth, GLuint mVpHeight);
    float *getPos();
private:
    //private method
    Description mState;
    TriangleIndex *mIndex;
    vf3_t *mPosition;
    vf2_t mUV[42];
    vf4_t mColor[42];
    GLint mProgram;
    unsigned short* mIndexData;
    int mIndexCount;
    //private variable
    GLint mVertexbuffer;
    float* mVertexData;
    float mRadius;
    float mLength;
    float mHorizontal;
    float mVertical;
    float mVcount;
    GLint mPositionHandle;
    GLint mMVPMatrixHandle;
    void initTexture();
    void BuildRect();

//  void initTexture(const char *path, GLuint *texture);
    void BuildTesselatedCylinder(const float radius, const float height,
                const int horizontal, const int vertical,
                const float uScale, const float vScale);
    void BuildUnitCubeLines();

    float toRadians(float degree);
    void initVertexData(float r);
    GLuint buildShader(const char* source, GLenum type);
    void createProgram(const char* vertex, const char* fragment);
};

#endif

