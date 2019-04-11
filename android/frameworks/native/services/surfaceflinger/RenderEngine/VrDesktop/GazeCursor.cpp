#include "GazeCursor.h"
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <sys/mman.h>
#include <SkBitmap.h>
#include <SkStream.h>
#include <SkImageDecoder.h>

#define GAZE_CURSOR_FILE "/system/media/gaze_cursor.png"

GazeCursor::GazeCursor(float r){
    mRadius = r;
    mVertexData = NULL;
    mTexcoordData = NULL;
    mIndexData = NULL;
    mIndexCount = 0;
}

GazeCursor::~GazeCursor(){
    glDeleteTextures(1, &mTexture);
}

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

void GazeCursor::create(){
    ALOGD("GazeCursor::create()");
    createProgram(mVertexShader, mFragmentShader);
    initVertexData(mRadius);
    initTexture();
}

void GazeCursor::drawSelf(mat4 mvp){
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
    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, mIndexData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GazeCursor::initTexture(){
    //load texture from /system/media/gaze_cursor.png
    int fd;;
    struct stat s;
    void *data;
    fd = open(GAZE_CURSOR_FILE, O_RDONLY);
    if(fd < 0){
        ALOGE("GazeCursor::initTexture cannot open '%s'", GAZE_CURSOR_FILE);
        return;
    }
    if(fstat(fd, &s) < 0){
        ALOGE("GazeCursor::initTexture fstat failed!");
        close(fd);
        return;
    }
    data = mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(data == MAP_FAILED){
        ALOGE("GazeCursor::initTexture mmap failed!");
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

void GazeCursor::initVertexData(float r){
    mVertexData = new float[4 * 3]{
        -r,  r, 0,
        -r, -r, 0,
         r, -r, 0,
         r,  r, 0
    };

    mTexcoordData = new float[4 * 2]{
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };

    mIndexData = new unsigned short[6]{
        0, 1, 2,
        0, 2, 3
    };

    mIndexCount = 6;
}

GLuint GazeCursor::buildShader(const char* source, GLenum type){
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

void GazeCursor::createProgram(const char* vertex, const char* fragment){
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
        mTextureHandle = glGetAttribLocation(programId, "aTextureCoord");
        if(mTextureHandle < 0){
            ALOGE("GazeCursor::createProgram Error not find 'aTextureCoord' in shader");
        }
    }
}

