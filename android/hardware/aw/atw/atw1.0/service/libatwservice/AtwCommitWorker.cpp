#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include "AtwCommitWorker.h"
#include "AtwClient.h"
#include "AtwLayer.h"
#include "IHeadTracker.h"
#include "AtwHal.h"
#include "AtwCalculator.h"
#include <pthread.h>
#include "bmp/bmp.h"
#include "sw_sync/sw_sync.h"

#include "cutils/properties.h"

#define ALIGN(x,a)   (((x) + (a) - 1L) & ~((a) - 1L))

namespace android
{

AtwCommitWorker::AtwCommitWorker(sp<AtwHal> device, sp<AtwCalculator> calculator) :
    VsyncListener(VSYNC_CALLBACK_ID),
    mDevice(device),
    mCalculator(calculator)
{
    mCommitFrames.clear();
    mCommitFrames.reserve(64);
}

AtwCommitWorker::~AtwCommitWorker() {}

status_t AtwCommitWorker::initCheck()
{
    mDevice->getSwapProgram(mSwapProgram);
    return NO_ERROR;
}

void AtwCommitWorker::LoopOnce()
{
    if(mClient->getLayer() == 0)
    {
        const useconds_t sleepTime = 1000*100;//0.1s
        //_LOGV("loopOnce go to usleep(%d). layer is no ready",sleepTime);
        usleep(sleepTime);
        return;
    }
    Commit();
}

void AtwCommitWorker::resetVsyncBase(uint64_t vsyncBase, int32_t swPeriod, int32_t vsyncPeriod)
{
    // 仅用作性能评估模式:
    //  当swPeriod 小于 vsyncPeriod, 希望应用渲染帧率不受 display 刷新帧率的限制, 因此希望直接释放当前设置的帧。
    mDebugDynamicRenderFps = (vsyncPeriod - swPeriod) > 1e6 ? true : false;

    HardwareVsync_t hwVsync;
    hwVsync.vsyncCount = 0;
    hwVsync.vsyncBaseNano = vsyncBase;
    hwVsync.vsyncPeriodsNano = mDebugDynamicRenderFps ? swPeriod : vsyncPeriod; // 当做性能评估的时候，让 atw commit 也按照性能评估的帧率来处理。
    mState.UpdateVsync(hwVsync);
}

void AtwCommitWorker::resetClient(sp<AtwClient> newer) // call from commit thread.
{
    std::lock_guard<std::mutex> lock(mCommitLock);

    mDevice->setBlank();

    //1. release current buffer.
    if(mLastFrameBuffer.mSlot != -1)
    {
        sp<AtwLayer> layer = mClient->getLayer();
        layer->releaseBuffer(mLastReleaseFence, mLastFrameBuffer);
        mLastFrameBuffer = BufferItem();
        mLastReleaseFence = 0;
    }

    //2. signaled all fences.
    int targetKey = -1;
    for(auto itr=mCommitted2.begin(); itr != mCommitted2.end(); )
    {
        const CommitFrameInfo_t& frame = *itr;
        if(targetKey < frame.fenceval)
        {
            targetKey = frame.fenceval;
        }
        itr = mCommitted2.erase(itr);
    }
    mDevice->timelineAdd2(targetKey);

    //3. reset client.
    mClient = newer;
}

void AtwCommitWorker::signalLastBuffer(const HwVsync_t &vsync)
{
    typedef struct
    {
        double   vsyncNow;
        uint64_t vsyncBase;
        uint64_t vsyncPeriod;
    }VsyncNow_t;
    VsyncNow_t now;
    mState.GetLastestVsyncState(now.vsyncNow, now.vsyncBase, now.vsyncPeriod);

    const uint64_t tolerance = 0;
    const uint64_t current = vsync.timestamp - tolerance;
    const uint64_t last = current - now.vsyncPeriod;

    std::vector<CommitFrameInfo_t> prev;
    std::vector<CommitFrameInfo_t> curr;
    if(mCommitted2.size() >= 2)
    {
        for(auto itr = mCommitted2.begin(); itr != mCommitted2.end(); itr++)
        {
            CommitFrameInfo_t info = *itr;
            if(info.timestamp < last)
            {
                prev.push_back(info);
            }
            else if(info.timestamp>=last && info.timestamp<=current)
            {
                curr.push_back(info);
            }
            else // info.timestamp > current.
            {
                break;
            }
        }

        const int xx = prev.size();
        const int yy = curr.size();
        int freedNum = -1;
        int targetKey = -1;
        if(yy > 0)
        {
            freedNum = xx+yy-1;
        }
        else
        {
            freedNum = xx-1;
        }

        if(freedNum > 0)
        {
            targetKey = mCommitted2[freedNum-1].fenceval;
        }
        for(auto itr=mCommitted2.begin(); freedNum>0; )
        {
            itr = mCommitted2.erase(itr);
            freedNum = freedNum-1;
            //_LOGV("FENCE: close fence={%d,%d}", frame.fence.fd, frame.fence.value);
        }

        ATRACE_NAME("ReleaseFence");
        mDevice->timelineAdd2(targetKey);
    }
}

void AtwCommitWorker::onVsync(const HwVsync_t &vsync)
{
    // call from vsync thread.
    ATRACE_CALL();
    std::lock_guard<std::mutex> lock(mCommitLock);
    if(mClient != 0)
    {
        signalLastBuffer(vsync);
    }
}

void AtwCommitWorker::releaseLastBuffer(const BufferItem& buffer, const FenceLocal_t& fence)
{
    sp<Fence> releasefence = new Fence(fence.fd);// will be auto close by sp<Fence>.
    int fenceval = fence.value;

    if(mLastFrameBuffer.mSlot != -1 && mLastFrameBuffer.mFrameNumber == buffer.mFrameNumber)
    {
        //nothing to do when last frame is exist and current buffer is same to last frame.
        return;
    }

    if(mLastFrameBuffer.mSlot != -1)
    {
        // release last framebuffer.
        sp<AtwLayer> layer = mClient->getLayer();
        layer->releaseBuffer(mLastReleaseFence, mLastFrameBuffer);
    }
    mLastFrameBuffer = buffer;
    mLastReleaseFence = releasefence;

    // post to vsync thread, vsync thread will update fence timeline depend on hwvsync messages.
    if(false == mDebugDynamicRenderFps)
    {// lock scope.
        //_LOGV("worker commit frame(%" PRIu64 " to display for {pid,idx}={%d,%d}", buffer.mFrameNumber, mClient->getPid(),mClient->getIndex());
        CommitFrameInfo_t info = {buffer.mFrameNumber, (uint64_t) NanoTime(), fenceval};
        std::lock_guard<std::mutex> lock(mCommitLock);
        mCommitted2.push_back(info);
    }
    else
    {
        // 每送一帧下来就立即释放它，让应用的渲染帧率不受 display 影响。
        // 仅用作性能评估需求下，动态控制应用渲染帧率 大于 显示刷新帧率。
        mDevice->timelineAdd2(fence.value);
    }
}

bool AtwCommitWorker::GetVsyncState(double &vsyncNow, uint64_t &hwVsyncBaseInNano, uint64_t &hwVsyncPeriodsInNano)
{
    mState.GetLastestVsyncState(vsyncNow, hwVsyncBaseInNano, hwVsyncPeriodsInNano);
    if(vsyncNow < 0.0f)
    {
        return false;
    }
    return true;
}

static double GetCurrentFramePoint(uint64_t vsyncBaseInNano, uint64_t vsyncPeriodsInNano)
{
    uint64_t now = NanoTime();
    return 1.0f * (now - vsyncBaseInNano) / vsyncPeriodsInNano;
}

bool AtwCommitWorker::Commit()
{
    ATRACE_CALL();
    double vsyncNow = 0.0f;
    uint64_t latestHwVsyncBaseInNano = 0;
    uint64_t latestHwVsyncPeriodsInNano = 0;
    if(GetVsyncState(vsyncNow, latestHwVsyncBaseInNano, latestHwVsyncPeriodsInNano) == false)
    {
        const useconds_t sleepTime = 1000*20;//20ms
        usleep(sleepTime);
        return true;
    }

    const uint64_t timeRequiredForCalculation = 1500000; // assume coefficents calculate need conume 1.5ms for each eye.
    const double coefficentsCalculationTimeInVsyncPeriods = timeRequiredForCalculation / ((1.0f)*latestHwVsyncPeriodsInNano);

    double nextVsync = ceil(vsyncNow);
    double vsyncBase = 0;
    if(vsyncNow + coefficentsCalculationTimeInVsyncPeriods >= nextVsync)
    {
        // NOTE: drop vsync here !
        vsyncBase = nextVsync;// time is not sufficent for us to compute coefficents. so we wait to next vsync.
        _LOGV("Drop vsync(%f) due to no enough time to do calculation. now=%f, now+cacl=%f", vsyncBase, vsyncNow, vsyncNow + coefficentsCalculationTimeInVsyncPeriods);
    }
    else
    {
        vsyncBase = nextVsync - 1.0f;
    }

    // Catching any possible happened "vsync missing" event, and notify anyone who care about these events through logging.
    // but please note carefully about not all vsync missing event(or drop frame) will be catched by these codes:
    // 1. vsync thread will update mState very often, and if hw vsync is not stable, we will drop vsync here without any notifications, this case can not be catched.
    // 2. in fact, we are calling setAtwLayer in another thread which may set frame failed without any notifications, so this case also can not be catched.
    bool bVsyncBaseAdjusted = false;
    if(mLastHwVsyncBaseInNano != latestHwVsyncBaseInNano)
    {
        bVsyncBaseAdjusted = true;
        mLastHwVsyncBaseInNano = latestHwVsyncBaseInNano;
    }
    if( (vsyncBase - mLastSwVsyncBase) > 1.0f && bVsyncBaseAdjusted == false && mDebugEnabled)
    {
        _LOGV("VSYNC dropped. Change from %f to %f", mLastSwVsyncBase, vsyncBase); // notify anyone who care about vsync drop event.
    }
    mLastSwVsyncBase = vsyncBase;

    const swapProgram_t& swap = mSwapProgram;
    sp<AtwClient> client = mClient;
    BufferItem currentBuffer;
    FramePose_t fp;
#if DEBUG_ONLY_CORRECT_DISTORTION
    bool bOnlyDoDistortionCorrection = true;
#else
    bool bOnlyDoDistortionCorrection = false;
#endif
    double beginWarpTimePoints[2];
    double real_commit = 0.0f;
    double real_vsync = 0.0f;
    const double tolerance = 0.5f / (1e-6*latestHwVsyncPeriodsInNano); // 优化后的setAtwLayer 时间消耗
    const double vsync_offset = 0.0f; // TODO.
    beginWarpTimePoints[0] = mState.FramePointTimeInSeconds( vsyncBase + swap.deltaVsync[0] );// time point of warping first eye
    beginWarpTimePoints[1] = mState.FramePointTimeInSeconds( vsyncBase + swap.deltaVsync[1] );// time point of warping second eye
    bool discardAcquireFence = mDiscardAcquireFence;
    for(int warp=0; warp < 2; warp++)
    {
        SleepUntilTimePoint( beginWarpTimePoints[warp], false );

        if(warp == 0) // try get a buffer from bufferqueue
        {
            const uint64_t presentTimeInNano = 1e9 * mState.FramePointTimeInSeconds( vsyncBase + swap.predictionPoints[0][0] );
            if( false == client->getAvailableFrameBuffer(presentTimeInNano, discardAcquireFence,  &currentBuffer, &fp) )
            {
                const useconds_t sleepTime = 1000*20;//20ms
                usleep(sleepTime);
                return true; // no buffer available yet.
            }
            bOnlyDoDistortionCorrection = !fp.isValid;
        }

        // 不再需要提前获取头部姿态，client->getOrientation不需要做 ipc 通信操作。
        avrQuatf warpPose[2];
        uint64_t warpTimeInNano[2];
        if(bOnlyDoDistortionCorrection == false)
        {
            warpTimeInNano[0] = mState.FramePointTimeInSeconds(vsyncBase + swap.predictionPoints[warp][0]) * 1e9;
            warpTimeInNano[1] = mState.FramePointTimeInSeconds(vsyncBase + swap.predictionPoints[warp][1]) * 1e9;
            if(NO_ERROR != client->getOrientation(&warpTimeInNano[0], &warpPose[0]))
            {
                bOnlyDoDistortionCorrection = true;
            }
        }

        // prepare compensation matrices for this eye.
        Eye_t eye = mWarpOrder[warp];
        WarpInput_t  calcInput;
        if(bOnlyDoDistortionCorrection == false)
        {
            buildTimeWarpMatrices(fp.orientation, calcInput, &warpPose[0]);
        }
        else
        {
            //_LOGV("only do distortion compensation for client");
            buildDistortionMatrices(calcInput);
        }

        buffer_handle_t coefficents;
        mCalculator->Prepare(vsyncBase, eye, warp, bOnlyDoDistortionCorrection, &coefficents);
        if(warp == 0)
        {
            // 用于估计设下去的这一帧是否会错过真实的 vsync.
            // 算法思路: 当前时间 commitFramePoint ，加上disp ioctl 需要消耗 tolerance 时间, 就可以计算出当前帧 setAtwlayer 的真实提交时间。 real_commit = commitFramePoint + tolerance;
            //          驱动将于 real_vsync 时间点附近来更新硬件寄存器，因此只要 real_commit 比 real_vsync 早，我们就认为这一帧被底层丢弃的概率很低。
            //          由于驱动的消影期应该比 vsync 还要早一点， 因此 real_vsync 的计算处理一个 vsync_offset. 目前 vsync_offset 需要手动测量，底层没有接口提供给我们查询vsync_offset。
            double commitFramePoint = GetCurrentFramePoint(latestHwVsyncBaseInNano, latestHwVsyncPeriodsInNano);// 当前时间。
            real_commit = commitFramePoint + tolerance;
            real_vsync  = (vsyncBase+1.0f)-vsync_offset;

            ANativeWindowBuffer* anb = currentBuffer.mGraphicBuffer->getNativeBuffer();
            FenceLocal_t fence = mDevice->setAtwLayer(currentBuffer.mFrameNumber, anb->handle, coefficents);
            releaseLastBuffer(currentBuffer, fence);
        }
        // 提前设 layer, 然后再计算系数。
        mCalculator->Compute(&calcInput.mats[0]);
    }

    // for debug.
    if(mDebugEnabled)
    {
        ShowFps(currentBuffer);
        if(real_commit >= real_vsync) // 判断前面设置的 frame 是否有可能丢帧。
        {
            _LOGV("WARNING! Drop frame risk: frame=%" PRIu64 ". %8.6f > %8.6f, set_need=%f, beyonded=%f",
                    currentBuffer.mFrameNumber, real_commit, real_vsync, tolerance, real_commit-real_vsync);
        }
    }
    if(currentBuffer.mGraphicBuffer != 0)
    {
        ScreenCap(currentBuffer.mGraphicBuffer);
    }
    return true;
}

static bool GetTimewarpDebugInfo()
{
    const char* debug_node = "debug.atw.rotation";
    static int debug_value = 0;
    static int cnt = 0;
    if(cnt++ >= 200)
    { // get timewarp debug setting value every cnt loops.
        debug_value = property_get_int32(debug_node, 0);
        cnt = 0;
    }
    return debug_value != 0;
}

void AtwCommitWorker::buildTimeWarpMatrices(const avrQuatf &cur, WarpInput_t &input, avrQuatf *pose)
{
    const float max_pitch = 15.0f * allwinner::Mathf::DegreeToRadFactor; // allwinner::Mathf::RadToDegreeFactor
    const float max_yaw = max_pitch;
    const float max_roll = max_pitch;
    const allwinner::Matrix4f TexM = allwinner::Matrix4f(0.500000, 0.000000,-0.500000, 0.000000,
                                                         0.000000, 0.500000,-0.500000, 0.000000,
                                                         0.000000, 0.000000,-1.000000, 0.000000,
                                                         0.000000, 0.000000,-1.000000, 0.000000);

    bool bOnlyDoDistCorrection = false;
    bool bDebugTimeWarpDegree = GetTimewarpDebugInfo();
    for(int scan=0; scan<2; scan++)
    {
        allwinner::Matrix4f warp = allwinner::CalculateTimeWarpMatrix2(cur, pose[scan]);

        float dd[3];
        allwinner::Quatf d = allwinner::Quatf(warp);
        d.GetEulerAngles<allwinner::Axis_X, allwinner::Axis_Y, allwinner::Axis_Z>(&dd[0],&dd[1],&dd[2]);
        if(bDebugTimeWarpDegree)
        {
            _LOGV("warp[%d]: %4.2f %4.2f %4.2f", scan, dd[0], dd[1], dd[2]);
        }

        if(fabs(dd[0]) >= max_pitch || fabs(dd[1]) >= max_yaw || fabs(dd[2]) >= max_roll)
        {
            _LOGV("ATW: warp degree over limits. cur={%f %f %f}, limits={%f %f %f}",
                fabs(dd[0]), fabs(dd[1]), fabs(dd[2]), max_pitch, max_yaw, max_roll);
            bOnlyDoDistCorrection = true;
            break;
        }

        /*
        Quatf 是 android device coordinate system;
        反畸变网格是根据显示设备坐标系 display device coordinate system 来建立的;
        因此需要从 android device coordinate system 转成 display device coordinate system.
        */
        allwinner::Quatf q;
        allwinner::Quatf xx = allwinner::Quatf(allwinner::Axis_X, dd[1]*-1.0f);
        allwinner::Quatf yy = allwinner::Quatf(allwinner::Axis_Y, dd[0]*-1.0f);
        allwinner::Quatf zz = allwinner::Quatf(allwinner::Axis_Z, dd[2]*-1.0f);
        q = xx*yy*zz;
        input.mats[scan] = TexM * allwinner::Matrix4f(q);
    }

    if(true == bOnlyDoDistCorrection)
    {
        input.mats[0] = TexM;
        input.mats[1] = TexM;
    }
}

void AtwCommitWorker::buildDistortionMatrices(WarpInput_t &input)
{
    const allwinner::Matrix4f TexM = allwinner::Matrix4f(0.500000, 0.000000,-0.500000, 0.000000,
                                                         0.000000, 0.500000,-0.500000, 0.000000,
                                                         0.000000, 0.000000,-1.000000, 0.000000,
                                                         0.000000, 0.000000,-1.000000, 0.000000);
    input.mats[0] = TexM;
    input.mats[1] = TexM;
}

void AtwCommitWorker::ShowFps(const BufferItem& current)
{
#if DEBUG_ATW_PERFORMANCE_FPS
    float renderFps = 0.0f;
    float commitFps = 0.0f;

    uint64_t currentTimeNano = NanoTime();
    uint64_t currentFrameNumber = current.mFrameNumber;

    if(mBeginFrameNumber == 0)
    {
        mBeginFrameNumber = currentFrameNumber;
    }
    CommitFrameInfo_t commitFrame;
    commitFrame.frameNumber = currentFrameNumber;
    commitFrame.timestamp = currentTimeNano;
    mCommitFrames.push_back(commitFrame);

    if(mCommitFrames.size() >= 60)// 在 commit >= 60的时候，统计渲染帧率和commit帧率
    {
        auto totalRenderFrames = currentFrameNumber - mBeginFrameNumber; // atw commit 60 帧的时间内，应用实际的渲染帧数。
        float totalTimeInSeconds = (currentTimeNano - mCommitFrames[0].timestamp) * 1.0f * 1e-9; // atw commit 60 帧所花费的实际时间, 单位秒
        renderFps = totalRenderFrames / totalTimeInSeconds;
        commitFps = mCommitFrames.size() / totalTimeInSeconds;

        _LOGV("RenderFps=%4.2f, CommitFps=%4.2f", renderFps, commitFps);

        // 为下一次统计做准备。
        mBeginFrameNumber = 0;
        mCommitFrames.clear();
    }
#else
    UN_USED(current);
#endif
}

// ---------------- Debug functions:

void AtwCommitWorker::resetDebugCtl(int value)
{
    bool ctrl = value != 0 ?  true : false;
    if(ctrl != mDebugEnabled)
    {
        mDebugEnabled = ctrl;
        _LOGV("worker debug %s. %d", mDebugEnabled ? "enabled":"disabled", value);
    }
}

void AtwCommitWorker::ScreenCap(sp<GraphicBuffer> sgb)
{
    const char* sAtwDgbAttributes = "debug.atw.capture";
    static int count = 60;
    count--;
    if(count >= 0)
    {
        return;
    }
    else
    {
        count = 60;
        mCaptureFrames = property_get_int32(sAtwDgbAttributes, 0);
        if(mCaptureFrames <= 0)
        {
            return;
        }
    }

    char sName[256];
    sprintf(sName, "/sdcard/disp-%d.bmp", mCaptureFrames);
    property_set(sAtwDgbAttributes, "0");

    uint8_t *addr = nullptr;
    if(sgb->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, (void**)&addr) != NO_ERROR)
    {
        return;
    }

    auto w = sgb->getWidth();
    auto h = sgb->getHeight();

    size_t bytesPerPixel = 0;
    auto format = sgb->getPixelFormat();
    switch(format)
    {
        case PIXEL_FORMAT_RGBA_8888:
        case PIXEL_FORMAT_RGBX_8888:
        case PIXEL_FORMAT_BGRA_8888:
        {
            bytesPerPixel = 4;
            break;
        }
        case PIXEL_FORMAT_RGB_888:
        {
            bytesPerPixel = 3;
            break;
        }
        case PIXEL_FORMAT_RGBA_5551:
        {
            bytesPerPixel = 2;
            break;
        }
        default:
        {
            _FATAL("unsupport bmp format(%d), %s %d", format, __func__, __LINE__);
        }
    }

    size_t totalBytes = w * h * bytesPerPixel;
    uint8_t *data = (uint8_t *)malloc(totalBytes);
    size_t copyBytesPerLine = w * bytesPerPixel;
    size_t realBytesPerLine = ALIGN(w * bytesPerPixel, 64);
    for(uint32_t line=0; line<h; line++)
    {
        uint8_t* dst = data + line * copyBytesPerLine;
        uint8_t* src = addr + line * realBytesPerLine;
        memcpy(dst, src, copyBytesPerLine);
    }
    //bool SaveAsBmp(const void *pixels, const size_t width, const size_t height, const size_t bitsPerPixel, const char* fpath);
    SaveAsBmp(data, w, h, bytesPerPixel*8, sName);
    sgb->unlock();

    free(data);
    data = nullptr;
}
}; // namespace android
