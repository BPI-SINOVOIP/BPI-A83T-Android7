#ifndef SF_VRDESKTOPRENDERENGINE_H_
#define SF_VRDESKTOPRENDERENGINE_H_

#include <stdint.h>
#include <sys/types.h>

#include <GLES2/gl2.h>
#include <Transform.h>
#include <utils/String8.h>
#include <mutex>

#include "../RenderEngine.h"
#include "../ProgramCache.h"
#include "../Description.h"
#include "Sphere.h"
#include "GazeCursor.h"
#include "ControllerCursor.h"
#include "IHeadTrackingService.h"
#include <string.h>
#include <stdio.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/Parcel.h>

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

class String8;
class Mesh;
class Texture;


class LayerFbo{
public:
    LayerFbo(Texture scaleDownTex, GLuint fboId) {
        this->scaleDownTex = scaleDownTex;
        this->fboId = fboId;
        changed = true;
    }

    Texture scaleDownTex;
    GLuint fboId;
    bool changed;
};

class VrDesktopRenderEngine : public RenderEngine {
    GLuint mProtectedTexName;
    GLint mMaxViewportDims[2];
    GLint mMaxTextureSize;
    GLuint mVpWidth;
    GLuint mVpHeight;
    mat4 mProjectMatrix;
    Rect mSourceCrop;
    Sphere* mBackground;
    GazeCursor* mGazeCursor;
    ControllerCursor* mControllerCursor;
    float mCursorLength;
    bool mCursorOn;
    mat4 mControllerPos;
    mat4 mLeftEye;
    mat4 mRightEye;
    mat4 mHeadOrient;
    float mFovScale;
    int mDisplayOrient;
    sp<IHeadTrackingService> mHeadTrackingService;
    struct Group {
        GLuint texture;
        GLuint fbo;
        GLuint width;
        GLuint height;
        mat4 colorTransform;
    };

    Description mState;
    Vector<Group> mGroupStack;

    virtual void bindImageAsFramebuffer(EGLImageKHR image,
            uint32_t* texName, uint32_t* fbName, uint32_t* status);
    virtual void unbindFramebuffer(uint32_t texName, uint32_t fbName);

public:
    VrDesktopRenderEngine();
    mat4 getHeadOrientation();
    virtual void drawBackground();
    virtual void enableVrDesktop(bool enable);
    virtual void drawForground();
    virtual void recenterOrientation();
    virtual void sendControllerData(int status, int needRecenter, float x,
                                        float y, float z, float w, float l);
protected:
    virtual ~VrDesktopRenderEngine();

    virtual void dump(String8& result);
    virtual void setViewportAndProjection(size_t vpw, size_t vph,
            Rect sourceCrop, size_t hwh, bool yswap,
            Transform::orientation_flags rotation);
#ifdef USE_HWC2
    virtual void setupLayerBlending(bool premultipliedAlpha, bool opaque,
            float alpha) override;
    virtual void setupDimLayerBlending(float alpha) override;
#else
    virtual void setupLayerBlending(bool premultipliedAlpha, bool opaque,
            int alpha);
    virtual void setupDimLayerBlending(int alpha);
#endif
    virtual void setupLayerTexturing(const Texture& texture);
    virtual void setupLayerBlackedOut();
    virtual void setupFillWithColor(float r, float g, float b, float a);
    virtual mat4 setupColorTransform(const mat4& colorTransform);
    virtual void disableTexturing();
    virtual void disableBlending();

    virtual void drawMesh(const Mesh& mesh);

    virtual size_t getMaxTextureSize() const;
    virtual size_t getMaxViewportDims() const;

    float scaleRatio;
    Texture mLayerTexture;
    Description mScaleDownState;
    bool mBufferChange;
    LayerFbo* scalDownSetup(int layerTextureName);
    void drawScaleDownMesh();
    void clearScaleDown(int layerTextureName);
    void bufferChange(bool change);
    void bufferDestroy(int textureName);

    DefaultKeyedVector<int, LayerFbo*> mLayerFboMap;
};

// ---------------------------------------------------------------------------
}; // namespace android
// ---------------------------------------------------------------------------

#endif /* SF_VRDESKTOPRENDERENGINE_H_ */
