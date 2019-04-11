#include "ControllerCursor.h"
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <sys/mman.h>
#include <SkBitmap.h>
#include <SkStream.h>
#include <SkImageDecoder.h>
#include "ControllerCursor.h"
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <sys/mman.h>
#include <SkBitmap.h>
#include <SkStream.h>
#include <SkImageDecoder.h>

#define UNUSED(a) do {__typeof__ (&a) __attribute__ ((unused)) __tmp = &a; } while(0)
#define USE_PRORGAM_CACHE

ControllerCursor::ControllerCursor(float r, float l, float h, float v){
    mRadius = r;
    mLength = l;
    mHorizontal = h;
    mVertical = v;
    mIndex = NULL;
    mPosition = NULL;
    mIndexData = NULL;
    mVertexData = NULL;
    mVcount = 0;
    mIndexCount = 0;
    mUV[0].x = 0;
    mUV[0].y = 0;
    mColor[0].x = 0;
    mColor[0].y = 0;
    mColor[0].z = 0;
    mColor[0].w = 0;
}

ControllerCursor::~ControllerCursor(){
    delete mPosition;
    delete mIndex;
}

static const char mFragmentShader[] =
    "void main(){\n"
    "  gl_FragColor = vec4(1,0,0,1);\n"
    "}\n";

static const char mVertexShader[] =
    "uniform mat4 uMVPMatrix;\n"
    "attribute vec4 aPosition;\n"
    "void main(){\n"
    "  gl_Position = uMVPMatrix * aPosition;\n"
    "}\n";

void ControllerCursor::create()
{
    ALOGD("ControllerCursor::create()");
    BuildTesselatedCylinder(mRadius, mLength, mHorizontal, mVertical, 1.0f, 1.0f); //fix
    mVcount = (mHorizontal + 1) * (mVertical + 1);
    createProgram(mVertexShader, mFragmentShader);
}

void ControllerCursor::BuildTesselatedCylinder(const float radius, const float height, const int horizontal, const int vertical, const float uScale, const float vScale)
{
    mPosition = new vf3_t[( horizontal + 1 ) * ( vertical + 1 )]{};
    for (int y = 0; y <= vertical; ++y) {
        const float yf = (float) y / (float) vertical;
        for ( int x = 0; x <= horizontal; ++x )
        {
            const float xf = (float) x / (float) horizontal;
            const int index = y * ( horizontal + 1 ) + x;
            mPosition[index].x = cosf( M_PI * 2 * xf ) * radius;//fix
            mPosition[index].y = sinf( M_PI * 2 * xf ) * radius;
            mPosition[index].z = -yf * height;//-height + yf * 2 * height
            mUV[index].x = xf * uScale;
            mUV[index].y = ( 1.0f - yf ) * vScale;
            for (int i = 0; i < 4; ++i)
            {
                //mColor[index][i] = 1.0f;
            }
            // fade to transparent on the outside
            if ( y == 0 || y == vertical )
            {
                //mColor[index][3] = 0.0f;
            }
        }
    }
    mIndex = new TriangleIndex[(horizontal * vertical * 6)]{};
    int index = 0;
    for (int y = 0; y < vertical; y++)
    {
        for (int x = 0; x < horizontal; x++)
        {
            mIndex[index + 0] = y * (horizontal + 1) + x;
            mIndex[index + 1] = y * (horizontal + 1) + x + 1;
            mIndex[index + 2] = (y + 1) * (horizontal + 1) + x;
            mIndex[index + 3] = (y + 1) * (horizontal + 1) + x;
            mIndex[index + 4] = y * (horizontal + 1) + x + 1;
            mIndex[index + 5] = (y + 1) * (horizontal + 1) + x + 1;
            index += 6;
        }
    }
    mIndexCount = horizontal * vertical * 6;
}

float *ControllerCursor::getPos(){
    return (float *)mPosition;
}

void ControllerCursor::drawSelf(mat4 mvp1, mat4 mvp2, GLuint mVpWidth, GLuint mVpHeight)
{
    UNUSED(mvp1);
    UNUSED(mvp2);
    UNUSED(mVpWidth);
    UNUSED(mVpHeight);

#ifndef USE_PRORGAM_CACHE
    GLenum error;

    glUseProgram(mProgram);
    glVertexAttribPointer(mPositionHandle, 3, GL_FLOAT, false,
        3 * sizeof(float), (float *)mPosition);

    error = glGetError();
    if (error != GL_NO_ERROR) {
        ALOGE("GL error 0x%04x, %d", int(error), __LINE__);
    }
    glEnableVertexAttribArray(mPositionHandle);

    error = glGetError();
    if (error != GL_NO_ERROR) {
        ALOGE("GL error 0x%04x, %d", int(error), __LINE__);
    }
    glUniformMatrix4fv(mMVPMatrixHandle, 1, false, mvp.asArray());
    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, mIndex);
    ALOGV("x=%f", mvp[0][0]);
    //glDrawArrays(6, 0, 303);    //draw mesh
#else
    glViewport(0, 0, mVpWidth, mVpHeight/2);
    mState.setColor(0.4431f, 0.6392f, 0.8863f, 0.8f);
    mState.setProjectionMatrix(mvp1);  //load the mvp matrix to mstate
    ProgramCache::getInstance().useProgram(mState);   //use the mstate to create program
    glVertexAttribPointer(Program::position,    //load vertex
            3,
            GL_FLOAT, GL_FALSE,
            3 * sizeof(float),
            mPosition);
    //glDrawArrays(GL_TRIANGLES, 0, mVcount);    //draw mesh
    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT,  mIndex);

    glViewport(0, mVpHeight/2, mVpWidth, mVpHeight/2);

    glViewport(0, mVpHeight/2, mVpWidth, mVpHeight/2);
    mState.setColor(0.4431f, 0.6392f, 0.8863f, 0.8f);

    //left
    mState.setProjectionMatrix(mvp2);    //load the mvp matrix to mstate

    ProgramCache::getInstance().useProgram(mState);   //use the mstate to create program

    glVertexAttribPointer(Program::position,    //load vertex
            3,
            GL_FLOAT, GL_FALSE,
            3 * sizeof(float),
            mPosition);

    //glDrawArrays(GL_TRIANGLES, 0, 303);    //draw mesh
    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, mIndex);
#endif
}

GLuint ControllerCursor::buildShader(const char* source, GLenum type){
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE){
        GLchar log[512];
        glGetShaderInfoLog(shader, sizeof(log), 0, log);
        ALOGE("Sphere::buildShader Error while compiling shader: \n%s\n%s", source, log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void ControllerCursor::createProgram(const char* vertex, const char* fragment){
    GLuint vertexId = buildShader(vertex, GL_VERTEX_SHADER);
    GLuint fragmentId = buildShader(fragment, GL_FRAGMENT_SHADER);
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexId);
    glAttachShader(programId, fragmentId);
    glLinkProgram(programId);

    GLint status;
    glGetProgramiv(programId, GL_LINK_STATUS, &status);
    if(status != GL_TRUE){
        ALOGE("Sphere::createProgram Error while linking shaders:");
        GLint infoLen = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1){
            GLchar log[infoLen];
            glGetProgramInfoLog(programId, infoLen, 0, &log[0]);
            ALOGE("Sphere::createProgram %s", log);
        }
        glDetachShader(programId, vertexId);
        glDetachShader(programId, fragmentId);
        glDeleteShader(vertexId);
        glDeleteShader(fragmentId);
        glDeleteProgram(programId);
    }else{
        mProgram = programId;
        mMVPMatrixHandle = glGetUniformLocation(programId, "uMVPMatrix");
        if(mMVPMatrixHandle < 0){
            ALOGE("GazeCursor::createProgram Error not find 'uMVPMatrix' in shader");
        }
        mPositionHandle = glGetAttribLocation(programId, "aPosition");
        if(mPositionHandle < 0){
            ALOGE("GazeCursor::createProgram Error not find 'aPosition' in shader");
        }
        //mTextureHandle = glGetAttribLocation(programId, "aTextureCoord");
        //if(mTextureHandle < 0){
        //  ALOGE("GazeCursor::createProgram Error not find 'aTextureCoord' in shader");
        //}
    }
}



