#include "Sphere.h"
#include <math.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <sys/mman.h>
#include <SkBitmap.h>
#include <SkStream.h>
#include <SkImageDecoder.h>

#define VRDESKTOP_BACKGROUND_FILE "/system/media/vrdesktop_bg.jpg"
Sphere::Sphere(float r){
    mRadius = r;
    mIndexCount = 0;
    mVertexData = NULL;
    mTexcoordData = NULL;
    mIndexData = NULL;
}

Sphere::~Sphere(){
    glDeleteTextures(1, &mTexture);
}
/*
static const char mVertexShader[] =
    "uniform mat4 uMVPMatrix;\n"
    "attribute vec4 aPosition;\n"
    "attribute vec4 aTextureCoord;\n"
    "varying vec2 vTextureCoord;\n"
    "void main(){\n"
    "  gl_Position = uMVPMatrix * aPosition;\n"
    "  vTextureCoord = aTextureCoord.xy;\n"
    "}\n";
    */
static const char mFragmentShader[] =
    "precision mediump float;\n"
    "varying vec2 vTextureCoord;\n"
    "uniform sampler2D sTexture;\n"
    "void main(){\n"
    "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
    "}\n";


static const char mVertexShader[] =
    "uniform mat4 uMVPMatrix;\n"
    "attribute vec4 aPosition;\n"
    "attribute vec4 aTextureCoord;\n"
    "varying vec2 vTextureCoord;\n"
    "void main(){\n"
    "  gl_Position = uMVPMatrix * aPosition;\n"
    "  vTextureCoord = aTextureCoord.xy;\n"
    "}\n";
/*
static const char mFragmentShader[] =
    "precision mediump float;\n"
    "varying vec2 vTextureCoord;\n"
    "uniform sampler2D sTexture;\n"
    "void main(){\n"
    "  gl_FragColor = max(texture2D(sTexture, vTextureCoord), vec4(1.0, 1.0, 1.0, 1.0));\n"
    "}\n";
*/

void Sphere::create(){
    ALOGD("Sphere::create()");
    createProgram(mVertexShader, mFragmentShader);
    initVertexData(mRadius);
    initTexture();
}

void Sphere::drawSelf(mat4 mvp){
    glUseProgram(mProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    glVertexAttribPointer(mPositionHandle, 3, GL_FLOAT, false,
        3 * sizeof(float), mVertexData);
    glEnableVertexAttribArray(mPositionHandle);
    glVertexAttribPointer(mTextureHandle, 2, GL_FLOAT, false,
        2 * sizeof(float), mTexcoordData);
    glEnableVertexAttribArray(mTextureHandle);
    glUniformMatrix4fv(mMVPMatrixHandle, 1, false, mvp.asArray());
    glDrawElements(GL_TRIANGLE_STRIP, mIndexCount, GL_UNSIGNED_SHORT, mIndexData);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Sphere::initTexture(){
    //load texture from /system/media/vrdesktobg.jpg
    int fd;;
    struct stat s;
    void *data;
    fd = open(VRDESKTOP_BACKGROUND_FILE, O_RDONLY);
    if(fd < 0){
        ALOGE("Sphere::initTexture cannot open '%s'", VRDESKTOP_BACKGROUND_FILE);
        return;
    }
    if(fstat(fd, &s) < 0){
        ALOGE("Sphere::initTexture fstat failed!");
        close(fd);
        return;
    }
    data = mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(data == MAP_FAILED){
        ALOGE("Sphere::initTexture mmap failed!");
        close(fd);
        return;
    }
    SkBitmap bitmap;
    SkImageDecoder::DecodeMemory(data, s.st_size,
            &bitmap, kUnknown_SkColorType, SkImageDecoder::kDecodePixels_Mode);
    munmap(data, s.st_size);
    close(fd);

    //gen the texture,and bind to the picture buffer.
    bitmap.lockPixels();
    const int w = bitmap.width();
    const int h = bitmap.height();
    const void* p = bitmap.getPixels();
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    switch(bitmap.colorType()){
        case kAlpha_8_SkColorType:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA,
                GL_UNSIGNED_BYTE, p);
            break;
        case kARGB_4444_SkColorType:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                GL_UNSIGNED_SHORT_4_4_4_4, p);
            break;
        case kN32_SkColorType:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, p);
            break;
        case kRGB_565_SkColorType:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                GL_UNSIGNED_SHORT_5_6_5, p);
            break;
        default:
            ALOGE("Sphere::initTexture Error do not support color type %d", bitmap.colorType());
            break;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float Sphere::toRadians(float degree){
    return ((degree*3.141592) / 180);
}

void Sphere::initVertexData(float r){
    const float angleSpan = 10.0f;

    //generate vertex data
    int bw = (int)(360 / angleSpan);
    int bh = (int)(180 / angleSpan);
    mVertexData = new float[(bw+1)*(bh+1)*3];
    int c = 0;
    for(int vAngle = 90; vAngle >= -90; vAngle = vAngle - angleSpan){
        for(int hAngle = 360; hAngle >= 0; hAngle = hAngle - angleSpan){
            double xozLength = r * cos(toRadians(vAngle));
            float x1 = (float)(xozLength * cos(toRadians(hAngle)));
            float z1 = (float)(xozLength * sin(toRadians(hAngle)));
            float y1 = (float)(r * sin(toRadians(vAngle)));

            mVertexData[c++] = x1;
            mVertexData[c++] = y1;
            mVertexData[c++] = z1;
        }
    }

    //generate texcoor data
    mTexcoordData = new float[(bw+1)*(bh+1)*2];
    float sizew = 1.0f/bw;
    float sizeh = 1.0f/bh;
    c = 0;
    for(int i = 0; i <= bh; i++){
        for(int j = 0; j <= bw; j++){
            float s = j * sizew;
            float t = i * sizeh;
            mTexcoordData[c++] = s;
            mTexcoordData[c++] = t;
        }
    }

    //generate index data
    mIndexData = new unsigned short[(bw+1)*bh*2];
    c = 0;
    int bIncrease = 1;
    for(int i = 0; i < bh; i++){
        if(bIncrease == 1){
            for(int j = 0; j <= bw; j++){
                unsigned short s = i * (bw + 1) + j;
                unsigned short t = (i + 1) * (bw + 1) + j;
                mIndexData[c++] = s;
                mIndexData[c++] = t;
            }
        }else{
            for(int j = bw; j >= 0; j--){
                unsigned short s = i * (bw + 1) + j;
                unsigned short t = (i + 1) * (bw + 1) + j;
                mIndexData[c++] = s;
                mIndexData[c++] = t;
            }
        }
        bIncrease = 1 - bIncrease;
    }
    mIndexCount = (bw+1)*bh*2;
}

GLuint Sphere::buildShader(const char* source, GLenum type){
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

void Sphere::createProgram(const char* vertex, const char* fragment){
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
            ALOGE("Sphere::createProgram Error not find 'uMVPMatrix' in shader");
        }
        mPositionHandle = glGetAttribLocation(programId, "aPosition");
        if(mPositionHandle < 0){
            ALOGE("Sphere::createProgram Error not find 'aPosition' in shader");
        }
        mTextureHandle = glGetAttribLocation(programId, "aTextureCoord");
        if(mTextureHandle < 0){
            ALOGE("Sphere::createProgram Error not find 'aTextureCoord' in shader");
        }
    }
}
