#pragma once
#include <utils/Errors.h>  // for status_t
#include <utils/RefBase.h>
#include <system/window.h> // for buffer_handle_t
#include <ui/GraphicBuffer.h> // for neon calculator.
#include <vector>
#include "AtwCore.h"
#include "AtwTypes.h"

namespace android{

class GraphicBuffer;
class GLES_Calculator;
class AtwCalculator : public virtual RefBase
{
public:
    AtwCalculator(int eye_w, int eye_h, int scn_w, int scn_h);
    virtual ~AtwCalculator();

    status_t initCheck();
    status_t loadResource();

    // 先拿到系数的 handle, 再做计算。
    void Prepare(double vsyncBase, const Eye_t target, const int slot, bool onlyCorrectDistortion, buffer_handle_t *output);
    void Compute(const allwinner::Matrix4f *mats);

private:
    void InitDistortionCoefficents();

protected:
    AtwCalculator& operator=(const AtwCalculator&) = delete;
    AtwCalculator(const AtwCalculator&) = delete;
public:
    int mEyeW;
    int mEyeH;
    int mScnW;
    int mScnH;

    int64_t mCurrentVsyncCount = -1;
    uint32_t mCurrentBufferSlot = 0;
    sp<GraphicBuffer> mCurrentCoefficentsBuffer = 0;
    int32_t *mTargetBuffer = nullptr;
    int32_t mTargetEye = -1;

    mat4 mMats[2];
    struct AtwCalculatorNeon *mpDevice = nullptr;
    std::vector<sp<GraphicBuffer>> mCoeffientBuffers;

    int32_t* mBufferTargets[3] = {nullptr, nullptr, nullptr};

    sp<GraphicBuffer> mCorrectDistortionCoefficents;
    bool mUseCorrectionDistortionCoefficents = false;

#if DEBUG_COMPUTE_PERF
    int mComputeCount = 0;
    uint64_t mTotalComputeTimeInNano = 0;
#endif

};

}; // namespace atw
