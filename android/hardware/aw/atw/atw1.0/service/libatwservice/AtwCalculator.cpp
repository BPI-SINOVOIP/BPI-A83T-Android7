#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include <sys/types.h>
#include <unistd.h>

#include <ui/GraphicBuffer.h>

#include "AtwCalculator.h"
#include "AtwLog.h"
#include "AtwTimer.h"

#if DEBUG_COMPUTE_PERF
#define _LOG_COMPUTE(...) ( (void)__android_log_print(ANDROID_LOG_VERBOSE, "ComputePerf", __VA_ARGS__) )
#endif

namespace android{

const static size_t COEFFCIENT_BUFFER_MAX_NUM = 3;

AtwCalculator::AtwCalculator(int eye_w, int eye_h, int scn_w, int scn_h)
{
    mEyeW = eye_w;
    mEyeH = eye_h;
    mScnW = scn_w;
    mScnH = scn_h;
}
AtwCalculator::~AtwCalculator()
{
    destroyAtwCalculatorNeon(mpDevice);
}

status_t AtwCalculator::initCheck()
{
    // int eyew = 1080;
    // int eyeh = 1920/2;
    // int disp_w = 1080;
    // int disp_h = 1920;
    createAtwCalculatorNeon(&mpDevice, mEyeW, mEyeH, mScnW, mScnH);

    const int tx = 40;
    const int ty = 40;
    const int NumIntsPerBlock = 4*8; //R|G|B|K, each block has 4*8 int32_t, which equal to 4*8 RGBA8888 pixels.
    const int NumBlocksPerEye = tx*ty;
    const int NumIntsPerEye = NumBlocksPerEye*NumIntsPerBlock;
    // each sgb will need to store tx*ty*2 i32blocks
    int bufferWidth = tx*8;
    int bufferHeight = ty*4*2;
    uint32_t usage = GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_HW_COMPOSER;
    PixelFormat format = PIXEL_FORMAT_RGBA_8888;
    mCoeffientBuffers.reserve(3);
    mCoeffientBuffers.resize(3);
    for(size_t i = 0; i < 3; i++)
    {
        _LOGV("#### DEBUG: alloc buffer. format=%d, usage=0x%x ", format, usage);
        sp<GraphicBuffer> sgb = new GraphicBuffer(bufferWidth, bufferHeight, format, usage);
        if(NO_ERROR != sgb->initCheck())
        {
            _LOGE("ERROR: create GraphicBuffer(%u, %u, %u, %u) failed", bufferWidth, bufferHeight, format, usage);
            mCoeffientBuffers.clear();
            return -1;
        }
        mCoeffientBuffers[i] = sgb;
    }

    sp<GraphicBuffer> sgb = new GraphicBuffer(bufferWidth, bufferHeight, format, usage);
    mCorrectDistortionCoefficents = sgb;
    InitDistortionCoefficents();
    return NO_ERROR;
}

void AtwCalculator::InitDistortionCoefficents()
{
    const size_t IntsPerEye = 40*40*32;
    const allwinner::Matrix4f TexM = allwinner::Matrix4f(0.500000, 0.000000,-0.500000, 0.000000,
                                                         0.000000, 0.500000,-0.500000, 0.000000,
                                                         0.000000, 0.000000,-1.000000, 0.000000,
                                                         0.000000, 0.000000,-1.000000, 0.000000);
    int32_t *bufferTarget = nullptr;
    mCorrectDistortionCoefficents->lock(GraphicBuffer::USAGE_SW_WRITE_OFTEN, (void **)&bufferTarget);
    for(int slot=0; slot<2;slot++)
    {
        bufferTarget += IntsPerEye*slot;
        float *m0 = &TexM.Transposed().M[0][0];
        float *m1 = &TexM.Transposed().M[0][0];
        memcpy(&mMats[0].c1.r1, m0, sizeof(mat4));
        memcpy(&mMats[1].c1.r1, m1, sizeof(mat4));
        mpDevice->DoTimeWarp(mpDevice, &mMats[0], slot, bufferTarget);
    }
    mCorrectDistortionCoefficents->unlock();
}

status_t AtwCalculator::loadResource()
{
    return  NO_ERROR;
}

void AtwCalculator::Prepare(double vsyncBase, const Eye_t target, const int slot, bool onlyCorrectDistortion, buffer_handle_t *output)
{
    const size_t IntsPerEye = 40*40*32;
    if(onlyCorrectDistortion == true)
    {
        mUseCorrectionDistortionCoefficents = true;
        *output = mCorrectDistortionCoefficents->getNativeBuffer()->handle;
        return;
    }
    mUseCorrectionDistortionCoefficents = false;

    int64_t currentVsyncCount = vsyncBase;
    if(mCurrentVsyncCount != currentVsyncCount)
    {
        mCurrentBufferSlot = (mCurrentBufferSlot+1) % 3;
        mCurrentVsyncCount = currentVsyncCount;
    }

    sp<GraphicBuffer> sgb = mCoeffientBuffers[mCurrentBufferSlot];
    int32_t *bufferTarget = nullptr;
    {
        ATRACE_NAME("Compute-lock");
        if(mBufferTargets[mCurrentBufferSlot] == nullptr)
        {
            sgb->lock(GraphicBuffer::USAGE_SW_WRITE_OFTEN, (void **)&bufferTarget);
            mBufferTargets[mCurrentBufferSlot] = bufferTarget;
        }
        else
        {
            bufferTarget = mBufferTargets[mCurrentBufferSlot];
        }

        bufferTarget += IntsPerEye*slot;
    }

    mTargetEye = (int) target;
    mTargetBuffer = bufferTarget;
    mCurrentCoefficentsBuffer = sgb;
    *output = sgb->getNativeBuffer()->handle;
}

void AtwCalculator::Compute(const allwinner::Matrix4f *mats)
{
    ATRACE_NAME("Compute");
    if(mUseCorrectionDistortionCoefficents == true)
    {
        return;
    }

#if DEBUG_COMPUTE_PERF
    const int MaxComputeCountForEachLog = 240;
    uint64_t computeBegin = NanoTime();
#endif

    float *m0 = &mats[0].Transposed().M[0][0];
    float *m1 = &mats[1].Transposed().M[0][0];
    memcpy(&mMats[0].c1.r1, m0, sizeof(mat4));
    memcpy(&mMats[1].c1.r1, m1, sizeof(mat4));
    mpDevice->DoTimeWarp(mpDevice, &mMats[0], mTargetEye, mTargetBuffer);

#if DEBUG_COMPUTE_PERF
    uint64_t computeFinish = NanoTime();
    mTotalComputeTimeInNano += (computeFinish-computeBegin);
    mComputeCount++;
    if(mComputeCount >= MaxComputeCountForEachLog)
    {
        float avg = 1.0f*mTotalComputeTimeInNano*1e-6 / mComputeCount;
        _LOG_COMPUTE("ComputePerf: avg=%8.6f(ms) in %4d", avg, mComputeCount);

        mTotalComputeTimeInNano = 0;
        mComputeCount = 0;
    }
#endif

    // clear cache for each eye because hardware will use it very soon.
    {
        ATRACE_NAME("Compute-unlock");
        //_LOGV("call unlock");
        mCurrentCoefficentsBuffer->unlock();
    }
}

}; // namespace android
