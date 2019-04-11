/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <ui/Rect.h>

#include <utils/String8.h>
#include <utils/Trace.h>

#include <cutils/compiler.h>
#include <gui/ISurfaceComposer.h>
#include <math.h>
#include <cutils/properties.h>

#include "VrDesktopRenderEngine.h"
#include "../Program.h"
#include "../ProgramCache.h"
#include "../Description.h"
#include "../Mesh.h"
#include "../Texture.h"

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

#define FOV "70"
#define METER_TO_PIXEL 1000
#define IPD 0.06f
#define VRDESKTOP_DISTANCE 3.0f
#define CURSOR_LENGTH 1000.0f

#define UNUSED(a) do {__typeof__ (&a) __attribute__ ((unused)) __tmp = &a; } while(0)

//#define CHECK_GL_ERROR() {\
//    GLint error = glGetError();\
//        if (error!=0) ALOGD("glError (0x%x) at:%s %d", error, __FUNCTION__, __LINE__);\
//}

#define CHECK_GL_ERROR()


VrDesktopRenderEngine::VrDesktopRenderEngine() :
        mVpWidth(0), mVpHeight(0), mCursorLength(0){

    mCursorOn = false;
    scaleRatio = 0;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, mMaxViewportDims);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    const uint16_t protTexData[] = { 0 };
    glGenTextures(1, &mProtectedTexName);
    glBindTexture(GL_TEXTURE_2D, mProtectedTexName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0,
            GL_RGB, GL_UNSIGNED_SHORT_5_6_5, protTexData);

    //mColorBlindnessCorrection = M;

    mBackground = new Sphere(50000.0f);
    mBackground->create();
    mGazeCursor = new GazeCursor(20.0f);
    mGazeCursor->create();
    mControllerCursor = new ControllerCursor(2.0f, CURSOR_LENGTH, 100, 2);//fix
    mControllerCursor->create();

    char property[4];
    int fovDegree;
    if (property_get("persist.vr.fov", property, FOV) >= 0) {
        fovDegree = atoi(property);
        mFovScale = tan(fovDegree * M_PI / 360);
        ALOGV("fov degree is %d, scale is %f", fovDegree, mFovScale);
    }else{
        ALOGW("can not find property 'persist.vr.fov', use a defaul fov value");
        mFovScale = 0.8f;
    }
    char orient[4];
    property_get("ro.sys.vr.forcelandscape", orient, "1");
    mDisplayOrient = atoi(orient);
    mVrDesktopEnable = false;

}

VrDesktopRenderEngine::~VrDesktopRenderEngine() {
    delete mBackground;

    int count = mLayerFboMap.size();
    for (int i=0; i<count; i++) {
        LayerFbo* layerFbo = mLayerFboMap.valueAt(i);
        delete layerFbo;
    }
    mLayerFboMap.clear();
}

size_t VrDesktopRenderEngine::getMaxTextureSize() const {
    return mMaxTextureSize;
}

size_t VrDesktopRenderEngine::getMaxViewportDims() const {
    return
        mMaxViewportDims[0] < mMaxViewportDims[1] ?
            mMaxViewportDims[0] : mMaxViewportDims[1];
}

void VrDesktopRenderEngine::setViewportAndProjection(
        size_t vpw, size_t vph, Rect sourceCrop, size_t hwh, bool yswap,
        Transform::orientation_flags rotation) {
    if(mVrDesktopEnable){

        int l = -(sourceCrop.right - sourceCrop.left)/2;
        int r = -l;

        // In GL, (0, 0) is the bottom-left corner, so flip y coordinates
        int t = -(sourceCrop.bottom - sourceCrop.top)/4;
        int b = -t;

        //Should caculate the near value by use the fov.For TMP,the fov is 90 degree.
        int near = ((float)-l)/mFovScale;
/*
        int near = 60;//((float)-l) / mFovScale;
        int l = -near*mFovScale;
        int r = -l;
        int t = l*(sourceCrop.bottom - sourceCrop.top)/(2*(sourceCrop.right - sourceCrop.left));
        int b = -t;
*/
        ALOGV("VrDesktopRenderEngine::setViewportAndProjection vpw=%zu, vph=%zu, hwh=%zu, l=%d", vpw, vph, hwh, (sourceCrop.right - sourceCrop.left)/2);
        mat4 m;
        mSourceCrop = sourceCrop;
        if (yswap) {
            m = mat4::frustum(l/10, r/10, t/10, b/10, near/10, 100000);
        } else {
            m = mat4::frustum(l/10, r/10, t/10, b/10, near/10, 100000);
        }

        // Apply custom rotation to the projection.
        float rot90InRadians = 2.0f * static_cast<float>(M_PI) / 4.0f;
        switch (rotation) {
            case Transform::ROT_0:
                break;
            case Transform::ROT_90:
                m = mat4::rotate(rot90InRadians, vec3(0,0,1)) * m;
                break;
            case Transform::ROT_180:
                m = mat4::rotate(rot90InRadians * 2.0f, vec3(0,0,1)) * m;
                break;
            case Transform::ROT_270:
                m = mat4::rotate(rot90InRadians * 3.0f, vec3(0,0,1)) * m;
                break;
            default:
                break;
        }
        //we will set view port when drawing left and right eye.
        //glViewport(0, 0, vpw, vph);
        mProjectMatrix = m;
        //we caculate the mvp matrix in drawing mesh.
        //mState.setProjectionMatrix(m);
        mVpWidth = vpw;
        mVpHeight = vph;

        ALOGV("SetViewPort mVpWidth=%zu, mVpHeight=%zu, hwh=%zu", vpw, vph, hwh);
        ALOGV("projection l=%d, r=%d, t=%d, b=%d, near%d", l, r, t, b, near);
    }else{
        size_t l = sourceCrop.left;
        size_t r = sourceCrop.right;
        size_t t = hwh - sourceCrop.top;
        size_t b = hwh - sourceCrop.bottom;

        mat4 m;
        if(yswap){
            m = mat4::ortho(l, r, t, b, 0, 1);
        }else{
            m = mat4::ortho(l, r, b, t, 0, 1);
        }

        //Apply custom rotation to the projection.
        float rot90InRadians = 2.0f * static_cast<float>(M_PI) / 4.0f;
        switch (rotation) {
            case Transform::ROT_0:
                break;
            case Transform::ROT_90:
                m = mat4::rotate(rot90InRadians, vec3(0,0,1)) * m;
                break;
            case Transform::ROT_180:
                m = mat4::rotate(rot90InRadians * 2.0f, vec3(0,0,1)) * m;
                break;
            case Transform::ROT_270:
                m = mat4::rotate(rot90InRadians * 3.0f, vec3(0,0,1)) * m;
                break;
            default:
                break;
        }
        glViewport(0, 0, vpw, vph);
        mState.setProjectionMatrix(m);
        mVpWidth = vpw;
        mVpHeight = vph;
    }
}

#ifdef USE_HWC2
void VrDesktopRenderEngine::setupLayerBlending(bool premultipliedAlpha,
        bool opaque, float alpha) {
#else
void VrDesktopRenderEngine::setupLayerBlending(
    bool premultipliedAlpha, bool opaque, int alpha) {
#endif

    mState.setPremultipliedAlpha(premultipliedAlpha);
    mState.setOpaque(opaque);
#ifdef USE_HWC2
    mState.setPlaneAlpha(alpha);

    if (alpha < 1.0f || !opaque) {
#else
    mState.setPlaneAlpha(alpha / 255.0f);

    if (alpha < 0xFF || !opaque) {
#endif
        glEnable(GL_BLEND);
        glBlendFunc(premultipliedAlpha ? GL_ONE : GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}

#ifdef USE_HWC2
void VrDesktopRenderEngine::setupDimLayerBlending(float alpha) {
#else
void VrDesktopRenderEngine::setupDimLayerBlending(int alpha) {
#endif
    mState.setPlaneAlpha(1.0f);
    mState.setPremultipliedAlpha(true);
    mState.setOpaque(false);
#ifdef USE_HWC2
    mState.setColor(0, 0, 0, alpha);
#else
    mState.setColor(0, 0, 0, alpha/255.0f);
#endif
    mState.disableTexture();

#ifdef USE_HWC2
    if (alpha == 1.0f) {
#else
    if (alpha == 0xFF) {
#endif
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void VrDesktopRenderEngine::setupLayerTexturing(const Texture& texture) {
    GLuint target = texture.getTextureTarget();
    glBindTexture(target, texture.getTextureName());
    GLenum filter = GL_NEAREST;
    if (texture.getFiltering()||mVrDesktopEnable) {
        filter = GL_LINEAR;
    }
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);

    mState.setTexture(texture);
    mLayerTexture = texture;
}

LayerFbo* VrDesktopRenderEngine::scalDownSetup(int layerTextureName) {

    scaleRatio = 0.7f;

    GLuint texId, status;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mVpWidth*scaleRatio, mVpHeight*scaleRatio, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERROR();

    Texture mScaleDownTexture;
    mScaleDownTexture.init(Texture::TEXTURE_2D, texId);
    mScaleDownTexture.setDimensions(mVpWidth*scaleRatio, mVpHeight*scaleRatio);
    mScaleDownState.setOpaque(false);
    CHECK_GL_ERROR();

    GLuint mScaleDownFBO;
    glGenFramebuffers(1, &mScaleDownFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mScaleDownFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, texId, 0);
    CHECK_GL_ERROR();
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status!=GL_FRAMEBUFFER_COMPLETE) {
        mScaleDownFBO = 0;
        ALOGE("glCheckFramebufferStatus fail!");
    }

    LayerFbo* layerFbo = new LayerFbo(mScaleDownTexture, mScaleDownFBO);
    mLayerFboMap.add(layerTextureName, layerFbo);
    //ALOGE("scaleDownSetup layertex:%d scaletex:%d %d", layerTextureName, texId, mScaleDownFBO);
    return layerFbo;
}

void VrDesktopRenderEngine::clearScaleDown(int layerTextureName) {
    LayerFbo* layerFbo = mLayerFboMap.valueFor(layerTextureName);
    if (layerFbo!=NULL) {
        GLuint texId = layerFbo->scaleDownTex.getTextureName();
        glDeleteTextures(1, &texId);
        glDeleteFramebuffers(1, &(layerFbo->fboId));
        delete layerFbo;
        //ALOGD("clearScaleDown:%d %d", layerTextureName, texId);
        mLayerFboMap.removeItem(layerTextureName);
    }
}

void VrDesktopRenderEngine::bufferDestroy(int textureName) {
    clearScaleDown(textureName);
}

void VrDesktopRenderEngine::drawScaleDownMesh() {
    LayerFbo* layerFbo = mLayerFboMap.valueFor(mLayerTexture.getTextureName());
    if (layerFbo==NULL) {
        layerFbo = scalDownSetup(mLayerTexture.getTextureName());
    }
    GLuint mScaleDownFBO = layerFbo->fboId;

    GLfloat gTriangleVertices[] = {
        -1, 1, 0, 1.0f,
        -1, -1, 0, 0,
        1, 1, 1.0f, 1.0f,
        1, -1, 1.0f, 0
    };

    glViewport(0, 0, mVpWidth*scaleRatio, mVpHeight*scaleRatio);

    mat4 mvp;
    mScaleDownState.setProjectionMatrix(mvp);    //load the mvp matrix to mstate

    glBindTexture(mLayerTexture.getTextureTarget(), mLayerTexture.getTextureName());
    mScaleDownState.setTexture(mLayerTexture);
    ProgramCache::getInstance().useProgram(mScaleDownState);   //use the mstate to create program
    CHECK_GL_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mScaleDownFBO);
    CHECK_GL_ERROR();

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnableVertexAttribArray(Program::texCoords);
    glVertexAttribPointer(Program::texCoords,
            2,
            GL_FLOAT, GL_FALSE,
            4*sizeof(float),
            &gTriangleVertices[0]+2);
    glVertexAttribPointer(Program::position,
            2,
            GL_FLOAT, GL_FALSE,
            4*sizeof(float),
            &gTriangleVertices[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL_ERROR();

    glDisableVertexAttribArray(Program::texCoords);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR();

}

void VrDesktopRenderEngine::setupLayerBlackedOut() {
    glBindTexture(GL_TEXTURE_2D, mProtectedTexName);
    Texture texture(Texture::TEXTURE_2D, mProtectedTexName);
    texture.setDimensions(1, 1); // FIXME: we should get that from somewhere
    mState.setTexture(texture);
}

mat4 VrDesktopRenderEngine::setupColorTransform(const mat4& colorTransform) {
    mat4 oldTransform = mState.getColorMatrix();
    mState.setColorMatrix(colorTransform);
    return oldTransform;
}

void VrDesktopRenderEngine::disableTexturing() {
    mState.disableTexture();
}

void VrDesktopRenderEngine::disableBlending() {
    glDisable(GL_BLEND);
}


void VrDesktopRenderEngine::bindImageAsFramebuffer(EGLImageKHR image,
        uint32_t* texName, uint32_t* fbName, uint32_t* status) {
    GLuint tname, name;
    // turn our EGLImage into a texture
    glGenTextures(1, &tname);
    glBindTexture(GL_TEXTURE_2D, tname);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)image);

    // create a Framebuffer Object to render into
    glGenFramebuffers(1, &name);
    glBindFramebuffer(GL_FRAMEBUFFER, name);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tname, 0);

    *status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    *texName = tname;
    *fbName = name;
}

void VrDesktopRenderEngine::unbindFramebuffer(uint32_t texName, uint32_t fbName) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbName);
    glDeleteTextures(1, &texName);
}

void VrDesktopRenderEngine::setupFillWithColor(float r, float g, float b, float a) {
    mState.setPlaneAlpha(1.0f);
    mState.setPremultipliedAlpha(true);
    mState.setOpaque(false);
    mState.setColor(r, g, b, a);
    mState.disableTexture();
    glDisable(GL_BLEND);
}

static int sneedRecenter;
const float rot90InRadians = static_cast<float>(M_PI) / 2.0f;
const mat4 rot270 = mat4::rotate(rot90InRadians * 3.0f, vec3(0,0,1));
const mat4 rot90 = mat4::rotate(rot90InRadians, vec3(0, 0, 1));
const mat4 moveForward = mat4::translate(vec4(0.0f, 0.0f, -1.5f*METER_TO_PIXEL, 1.0f));

static mat4 covertToMatix(float x, float y, float z, float w)
{
    float ww = w * w;
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    mat4 cov;
    //convent to mat.
    //mat4 M;
    cov[0][0] = ww + xx - yy - zz;
    cov[0][1] = 2 * (x*y + w*z);
    cov[0][2] = 2 * (x*z - w*y);
    cov[0][3] = 0;
    cov[1][0] = 2 * (x*y - w*z);
    cov[1][1] = ww - xx + yy - zz;
    cov[1][2] = 2 * (y*z + w*x);
    cov[1][3] = 0;
    cov[2][0] = 2 * (x*z + w*y);
    cov[2][1] = 2 * (y*z - w*x);
    cov[2][2] = ww - xx - yy + zz;
    cov[2][3] = 0;
    cov[3][0] = 0;
    cov[3][1] = 0;
    cov[3][2] = 0;
    cov[3][3] = 1;
    return cov;
}

void VrDesktopRenderEngine::sendControllerData(int status, int needRecenter, float x,
                                            float y, float z, float w, float l){
    UNUSED(status);
    UNUSED(needRecenter);
    static mat4 lastM;

    if (status) {
        mCursorOn = true;
        if (sneedRecenter || needRecenter) {
            lastM = covertToMatix(-x, -y, -z, w);

            mControllerPos = lastM * covertToMatix(x, y, z, w) ;
            ALOGV("(%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f),",
                mControllerPos[0][0], mControllerPos[0][1], mControllerPos[0][2], mControllerPos[0][3],
                mControllerPos[1][0], mControllerPos[1][1], mControllerPos[1][2], mControllerPos[1][3],
                mControllerPos[2][0], mControllerPos[2][1], mControllerPos[2][2], mControllerPos[2][3],
                mControllerPos[3][0], mControllerPos[3][1], mControllerPos[3][2], mControllerPos[3][3]);
        } else {
            mControllerPos = lastM * covertToMatix(x, y, z, w);
        }
        mCursorLength = l;
        ALOGV("mCursorLength = %f", mCursorLength);
    } else {
        mCursorOn = false;
    }
}

void VrDesktopRenderEngine::recenterOrientation()
{
    if(mHeadTrackingService.get() == NULL){
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder = sm->getService(String16("softwinner.HeadTrackingService"));
        mHeadTrackingService = interface_cast<IHeadTrackingService>(binder);
        if (mHeadTrackingService.get() != NULL) {
            mHeadTrackingService->recenterOrientation();
        }
    } else {
        mHeadTrackingService->recenterOrientation();
    }
    sneedRecenter = 1;
    usleep(100000);
    sneedRecenter = 0;
}

mat4 VrDesktopRenderEngine::getHeadOrientation(){
    //TO DO
    return mat4();
}

void VrDesktopRenderEngine::enableVrDesktop(bool enable){
    mVrDesktopEnable = enable;
    if(enable && mHeadTrackingService.get() == NULL){
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder = sm->getService(String16("softwinner.HeadTrackingService"));
        mHeadTrackingService = interface_cast<IHeadTrackingService>(binder);
    }
}

uint64_t GetTicksNanos()
{
    struct timespec tp;
    const int       status = clock_gettime(CLOCK_MONOTONIC, &tp);
    if(status != 0)
    {
        //DBG("clock_gettime return %d \n", status);
    }

    const uint64_t result = (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t(tp.tv_nsec);
    return result;
}

void VrDesktopRenderEngine::drawBackground(){
    if(mVrDesktopEnable){
        ALOGV("VrDesktopRenderEngine::drawBackground() vpw=%d, vph=%d", mVpWidth, mVpHeight);
        vec2 center((mSourceCrop.right + mSourceCrop.left)/2, (mSourceCrop.bottom + mSourceCrop.top)/2);

        //Transform::orientation_flags rotation = Transform::ROT_90;
        Transform::orientation_flags rotation = Transform::ROT_0;
        mat4 model;

        switch (rotation) {
            case Transform::ROT_0:
                break;
            case Transform::ROT_90:
                model = mat4::rotate(rot90InRadians, vec3(0,0,1));
                break;
            case Transform::ROT_180:
                model = mat4::rotate(rot90InRadians * 2.0f, vec3(0,0,1));
                break;
            case Transform::ROT_270:
                model = mat4::rotate(rot90InRadians * 3.0f, vec3(0,0,1));
                break;
            default:
                break;
        }
        if(mHeadTrackingService.get() != NULL){
            double now = double(GetTicksNanos()) * 0.000000001;
            std::vector<float> orien = mHeadTrackingService->getPredictionForTime(now);
            mat4 orient(orien[0], orien[1], orien[2], orien[3],
                orien[4], orien[5], orien[6], orien[7],
                orien[8], orien[9], orien[10], orien[11],
                orien[12], orien[13], orien[14], orien[15]);
            /*
            ALOGE("[%f, %f, %f, %f][%f, %f, %f, %f][%f, %f, %f, %f][%f, %f, %f, %f]",
                orien[0], orien[1], orien[2], orien[3],
                orien[4], orien[5], orien[6], orien[7],
                orien[8], orien[9], orien[10], orien[11],
                orien[12], orien[13], orien[14], orien[15]);
            */
            mHeadOrient = orient;
        } else {
            ALOGE("mHeadTrackingService is NULL, get service again!");
            sp<IServiceManager> sm = defaultServiceManager();
            sp<IBinder> binder = sm->checkService(String16("softwinner.HeadTrackingService"));
            if(binder != NULL){
                mHeadTrackingService = interface_cast<IHeadTrackingService>(binder);
            }
        }
        switch(mDisplayOrient){
        case 2://rotate 270 degree
            glViewport(0, 0, mVpWidth, mVpHeight/2);
            mBackground->drawSelf(mProjectMatrix * rot90 * mHeadOrient * mRightEye * model);
            glViewport(0, mVpHeight/2, mVpWidth, mVpHeight/2);
            mBackground->drawSelf(mProjectMatrix * rot90 * mHeadOrient * mLeftEye * model);
            break;
        case 1://rotate 90 degree
        default:
            glViewport(0, 0, mVpWidth, mVpHeight/2);
            mBackground->drawSelf(mProjectMatrix * rot270 * mHeadOrient * mRightEye * model);
            glViewport(0, mVpHeight/2, mVpWidth, mVpHeight/2);
            mBackground->drawSelf(mProjectMatrix * rot270 * mHeadOrient * mLeftEye * model);
            break;
        }
    }
}

void VrDesktopRenderEngine::drawForground()
{
    if (mCursorOn) {
        float scale;
        if (mCursorLength <= 0.0f) {
            // is parallel
            scale = 0.5;
        } else if (mCursorLength >= 10.0f) {
            scale = 10.0;
        } else {
            scale = mCursorLength * METER_TO_PIXEL / CURSOR_LENGTH;
        }

        ALOGV("scale = %f, length = %f", scale, mCursorLength);
        mat4 model = mat4::translate(vec4(0.35 * METER_TO_PIXEL, -0.3* METER_TO_PIXEL, -0.3 * METER_TO_PIXEL, 1.0f))
                                                    * mControllerPos * mat4::scale(vec4(1, 1, scale, 1));
        switch(mDisplayOrient){
        case 2:{
            mat4 mvp1 = mProjectMatrix * rot90 * mHeadOrient * mLeftEye * model;
            mat4 mvp2 = mProjectMatrix * rot90 * mHeadOrient * mRightEye * model;
            mControllerCursor->drawSelf(mvp2, mvp1, mVpWidth, mVpHeight);
            }
            break;
        case 1:
        default:{
            mat4 mvp1 = mProjectMatrix * rot270 * mHeadOrient * mLeftEye * model;
            mat4 mvp2 = mProjectMatrix * rot270 * mHeadOrient * mRightEye * model;
            mControllerCursor->drawSelf(mvp2, mvp1, mVpWidth, mVpHeight);
            }
            break;
        }
    } else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, mVpWidth, mVpHeight/2);
        mGazeCursor->drawSelf(mProjectMatrix * rot270 * mRightEye * moveForward);
        glViewport(0, mVpHeight/2, mVpWidth, mVpHeight/2);
        mGazeCursor->drawSelf(mProjectMatrix * rot270 * mLeftEye * moveForward);
    }

}

void VrDesktopRenderEngine::bufferChange(bool change){
    mBufferChange = change;
}

void VrDesktopRenderEngine::drawMesh(const Mesh& mesh) {

    if(mVrDesktopEnable){
        if(mBufferChange){
            //update scaledown tex
            drawScaleDownMesh();
        }
        //use scaledown tex
        LayerFbo* pLayerFbo = mLayerFboMap.valueFor(mLayerTexture.getTextureName());
        if (pLayerFbo==NULL)
            return;
        Texture scaleDownTexture = pLayerFbo->scaleDownTex;
        glBindTexture(GL_TEXTURE_2D, scaleDownTexture.getTextureName());
        mState.setTexture(scaleDownTexture);

        //do not show the Layer which is bigger than the canvas of the vr desktop.
        float const* position = mesh.getPositions();
        size_t vertexCount = mesh.getVertexCount();
        for(size_t i = 0; i < vertexCount; i++){
            if(position[i] > mVpWidth){
                ALOGV("do not draw mesh as it's width(%f) is bigger than vr desktop", position[i]);
                return;
            }
            if(position[i+1] > mVpHeight){
                ALOGV("do not draw mesh as it's height(%f) is bigger than vr desktop", position[i+1]);
                return;
            }
            i = i + 1;
        }

        if(mDisplayOrient == 2){
            //draw left eye
            glViewport(0, 0, mVpWidth, mVpHeight/2);

            vec2 center((mSourceCrop.right + mSourceCrop.left)/2, (mSourceCrop.bottom + mSourceCrop.top)/2);
            ALOGV("center x=%f, y=%f", center.x, center.y);
            mLeftEye = mat4::lookAt(vec3(-METER_TO_PIXEL * IPD / 2, 0.0f, 0.0f),
                                    vec3(-METER_TO_PIXEL * IPD / 2, 0.0f, -1.0f),
                                    vec3(0.0f, 1.0f, 0.0f));
            mat4 model = rot270 * mat4::translate(vec4(-center.x, -center.y, -VRDESKTOP_DISTANCE * METER_TO_PIXEL, 1.0f));
            mat4 leftEyeMvp = mProjectMatrix * rot90 * mHeadOrient * mLeftEye * model;
            mState.setProjectionMatrix(leftEyeMvp);    //load the mvp matrix to mstate

            ProgramCache::getInstance().useProgram(mState);   //use the mstate to create program
            if(mesh.getTexCoordsSize()){        //load texcoord
                glEnableVertexAttribArray(Program::texCoords);
                glVertexAttribPointer(Program::texCoords,
                        mesh.getTexCoordsSize(),
                        GL_FLOAT, GL_FALSE,
                        mesh.getByteStride(),
                        mesh.getTexCoords());
            }
            glVertexAttribPointer(Program::position,    //load vertex
                    mesh.getVertexSize(),
                    GL_FLOAT, GL_FALSE,
                    mesh.getByteStride(),
                    mesh.getPositions());
            glDrawArrays(mesh.getPrimitive(), 0, mesh.getVertexCount());    //draw mesh

            //draw right eye
            glViewport(0, mVpHeight/2, mVpWidth, mVpHeight/2);

            mRightEye = mat4::lookAt(vec3(METER_TO_PIXEL*IPD/2, 0.0f, -0.0f),
                vec3(METER_TO_PIXEL*IPD/2, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

            mat4 rightEyeMvp = mProjectMatrix * rot90 * mHeadOrient * mRightEye * model;

            mState.setProjectionMatrix(rightEyeMvp);

            ProgramCache::getInstance().useProgram(mState);   //use the mstate to create program
            if(mesh.getTexCoordsSize()){        //load texcoord
                glEnableVertexAttribArray(Program::texCoords);
                glVertexAttribPointer(Program::texCoords,
                        mesh.getTexCoordsSize(),
                        GL_FLOAT, GL_FALSE,
                        mesh.getByteStride(),
                        mesh.getTexCoords());
            }
            glVertexAttribPointer(Program::position,    //load vertex
                    mesh.getVertexSize(),
                    GL_FLOAT, GL_FALSE,
                    mesh.getByteStride(),
                    mesh.getPositions());
            glDrawArrays(mesh.getPrimitive(), 0, mesh.getVertexCount());    //draw mesh
        }else{
            //draw left eye
            glViewport(0, mVpHeight/2, mVpWidth, mVpHeight/2);

            vec2 center((mSourceCrop.right + mSourceCrop.left)/2, (mSourceCrop.bottom + mSourceCrop.top)/2);
            ALOGV("center x=%f, y=%f", center.x, center.y);
            mLeftEye = mat4::lookAt(vec3(-METER_TO_PIXEL * IPD / 2, 0.0f, 0.0f),
                                    vec3(-METER_TO_PIXEL * IPD / 2, 0.0f, -1.0f),
                                    vec3(0.0f, 1.0f, 0.0f));
            mat4 model = rot90 * mat4::translate(vec4(-center.x, -center.y, -VRDESKTOP_DISTANCE * METER_TO_PIXEL, 1.0f));
            mat4 leftEyeMvp = mProjectMatrix * rot270 * mHeadOrient * mLeftEye * model;
            mState.setProjectionMatrix(leftEyeMvp);    //load the mvp matrix to mstate

            ProgramCache::getInstance().useProgram(mState);   //use the mstate to create program
            if(mesh.getTexCoordsSize()){        //load texcoord
                glEnableVertexAttribArray(Program::texCoords);
                glVertexAttribPointer(Program::texCoords,
                        mesh.getTexCoordsSize(),
                        GL_FLOAT, GL_FALSE,
                        mesh.getByteStride(),
                        mesh.getTexCoords());
            }
            glVertexAttribPointer(Program::position,    //load vertex
                    mesh.getVertexSize(),
                    GL_FLOAT, GL_FALSE,
                    mesh.getByteStride(),
                    mesh.getPositions());
            glDrawArrays(mesh.getPrimitive(), 0, mesh.getVertexCount());    //draw mesh

            //draw right eye
            glViewport(0, 0, mVpWidth, mVpHeight/2);

            mRightEye = mat4::lookAt(vec3(METER_TO_PIXEL*IPD/2, 0.0f, -0.0f),
                vec3(METER_TO_PIXEL*IPD/2, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

            mat4 rightEyeMvp = mProjectMatrix * rot270 * mHeadOrient * mRightEye * model;


            mState.setProjectionMatrix(rightEyeMvp);

            ProgramCache::getInstance().useProgram(mState);   //use the mstate to create program
            if(mesh.getTexCoordsSize()){        //load texcoord
                glEnableVertexAttribArray(Program::texCoords);
                glVertexAttribPointer(Program::texCoords,
                        mesh.getTexCoordsSize(),
                        GL_FLOAT, GL_FALSE,
                        mesh.getByteStride(),
                        mesh.getTexCoords());
            }
            glVertexAttribPointer(Program::position,    //load vertex
                    mesh.getVertexSize(),
                    GL_FLOAT, GL_FALSE,
                    mesh.getByteStride(),
                    mesh.getPositions());
            glDrawArrays(mesh.getPrimitive(), 0, mesh.getVertexCount());    //draw mesh
        }


        if (mesh.getTexCoordsSize()) {
            glDisableVertexAttribArray(Program::texCoords);
        }
    }else{
        ProgramCache::getInstance().useProgram(mState);
        if (mesh.getTexCoordsSize()) {
            glEnableVertexAttribArray(Program::texCoords);
            glVertexAttribPointer(Program::texCoords,
                    mesh.getTexCoordsSize(),
                    GL_FLOAT, GL_FALSE,
                    mesh.getByteStride(),
                    mesh.getTexCoords());
        }

        glVertexAttribPointer(Program::position,
                mesh.getVertexSize(),
                GL_FLOAT, GL_FALSE,
                mesh.getByteStride(),
                mesh.getPositions());

        glDrawArrays(mesh.getPrimitive(), 0, mesh.getVertexCount());

        if (mesh.getTexCoordsSize()) {
            glDisableVertexAttribArray(Program::texCoords);
        }
    }
}

void VrDesktopRenderEngine::dump(String8& result) {
    RenderEngine::dump(result);
}

// ---------------------------------------------------------------------------
}; // namespace android
// ---------------------------------------------------------------------------

#if defined(__gl_h_)
#error "don't include gl/gl.h in this file"
#endif
