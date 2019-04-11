#include <sys/time.h>
#include <sys/types.h>//uint64_t

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "AwVRSensorHal.h"

#include <queue>    // c++ stl conainter queue.
#include <string.h> //memcpy

#include <sys/system_properties.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include "OVR_Log.h"

#ifdef MIN
#undef MIN
#endif
#define MIN(X,Y)  ((X)>(Y) ? (Y):(X))

#ifdef MAX
#undef MAX
#endif
#define MAX(X,Y)  ((X)>(Y) ? (X):(Y))

#define VECTOR_CONVERSION_1(V) (OVR::Vector3f((V).y*(-1.0f),(V).x,(V).z))
#define VECTOR_CONVERSION_2(V) (OVR::Vector3f((V).y,(V).x*(-1.0f),(V).z))

static uint64_t GetTicksNanos()
{
    struct timespec tp;
    const int       status = clock_gettime(CLOCK_MONOTONIC, &tp);
    if(status != 0)
    {
        ALOGE("clock_gettime return %d \n", status);
    }

    const uint64_t result = (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t(tp.tv_nsec);
    return result;
}

const int DefaultEventsBufferSize = 128;
const int LOOP_ID  = 0x1;
const int MPU_SAMPLE_RATE = 800;
const int MAG_RATE = 100;
class SensorDeviceLocal : public SensorHal
{
public:
    SensorDeviceLocal(OVR::SensorFusion &fusion):mFusion(fusion)
    {
        mFusion.setSensor(this);
        mRawSensorEventsBuffer.reserve(DefaultEventsBufferSize);
        mRawSensorEventsBuffer.resize(DefaultEventsBufferSize);
        mSfMsseageQueue.reserve(DefaultEventsBufferSize);
        mSfMsseageQueue.resize(DefaultEventsBufferSize);

        char value[128];
        __system_property_get("ro.sys.vr.forcelandscape", value);
        mLcdOrient = atoi(value); // 1 means rotate 90, 2 means rotate 270
    }

    ~SensorDeviceLocal()
    {
        if( NULL != mSensorMgr && NULL != mSensorEventQueue )
        {
            ASensorManager_destroyEventQueue(mSensorMgr,mSensorEventQueue);
        }
        mSensorMgr = NULL;
        mSensorEventQueue = NULL;

        if( mLooper != NULL )
        {
            ALooper_release( mLooper );
            mLooper = NULL;
        }
    }

    bool Init()
    {
        int ret = 0;

        mLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        if( NULL == mLooper )
        {
            ALOGE( "ERROR: Init LOOPER Failed. " );
            return false;
        }
        ALooper_acquire(mLooper);

        //get sensormgr, and get sensors from it.
        mSensorMgr   = ASensorManager_getInstance();
        if( NULL == mSensorMgr )
        {
            ALOGE( "ERROR: Get SensorManager Failed." );
            return false;
        }
        mGyro  = ASensorManager_getDefaultSensor(mSensorMgr, ASENSOR_TYPE_GYROSCOPE);
        mAccel = ASensorManager_getDefaultSensor(mSensorMgr, ASENSOR_TYPE_ACCELEROMETER);
        mMag = ASensorManager_getDefaultSensor(mSensorMgr, ASENSOR_TYPE_MAGNETIC_FIELD);

        if( NULL == mGyro || NULL == mAccel )
        {
            ALOGE("ERROR: Start Sensor Failed \n");
            return false;
        }

        //create a sensorEventQueue from sensormgr.
        mSensorEventQueue = ASensorManager_createEventQueue(mSensorMgr, mLooper, LOOP_ID, NULL, NULL);
        if(NULL == mSensorEventQueue)
        {
            ALOGE( "ERROR: Create SensorEventQueue Failed" );
            return false;
        }

        //TODO. How to handle error in here ?
        ret = ASensorEventQueue_enableSensor(mSensorEventQueue, mGyro);
        if( ret < 0 )
        {
            ALOGE( "ERROR: Enable Gyro Failed. ret = %d ", ret );
        }

        ret = ASensorEventQueue_enableSensor(mSensorEventQueue, mAccel);
        if( ret < 0 )
        {
            ALOGE( "ERROR: Enable Accel Failed. ret = %d ", ret );
        }

        ret = ASensorEventQueue_enableSensor(mSensorEventQueue, mMag);
        if( ret < 0 )
        {
            ALOGE( "ERROR: Enable Mag Failed. ret = %d ", ret );
        }

        ret = ASensorEventQueue_setEventRate(mSensorEventQueue, mGyro,  1000.0f / MPU_SAMPLE_RATE * 1000);
        if( ret < 0 )
        {
            ALOGE( "ERROR: Set Gyro Rate Failed. ret = %d ", ret );
        }

        ASensorEventQueue_setEventRate(mSensorEventQueue, mAccel, 1000.0f / MPU_SAMPLE_RATE * 1000);
        if( ret < 0 )
        {
            ALOGE( "ERROR: Set Accel Rate Failed. ret = %d ", ret );
        }

        ASensorEventQueue_setEventRate(mSensorEventQueue, mMag, 1000.0f / MAG_RATE* 1000);
        if( ret < 0 )
        {
            ALOGE( "ERROR: Set Mag Rate Failed. ret = %d ", ret );
        }

        ALOGD(" SensorThread Complete Init Work . mpu_rate=%d, mag_rate=%d", MPU_SAMPLE_RATE, MAG_RATE);
        return true;
    }
    bool PollEvents(int &events)
    {
        return ALooper_pollAll(1, NULL, &events, NULL) == LOOP_ID;
    }

    void setNeedSyncTime(bool needSyncTime)
    {
        mNeedResyncTime = needSyncTime;
    }

    bool Exit()
    {
        int ret = 0;
        ret = ASensorEventQueue_disableSensor(mSensorEventQueue,mAccel);
        ALOGD("disable accel. ret=%d",ret);
        ret = ASensorEventQueue_disableSensor(mSensorEventQueue,mGyro);
        ALOGD("disable gyro. ret=%d",ret);
        ret = ASensorEventQueue_disableSensor(mSensorEventQueue,mMag);
        ALOGD("disable mag. ret=%d",ret);
        return ret == 0;
    }

    void handleMessage( int events )
    {
        int count = ReadRawSensorEventsFromService(events);
        if(true == PushEventsToFifos(mRawSensorEventsBuffer.data(), count))
        {
            int builded = BuildSfMessagesFromRawSensorEvents();
            for(int i=0; i<builded; i++)
            {
                OVR::SFMessageBodyFrame &message = mSfMsseageQueue[i];
                mFusion.handleMessage(message);
            }
        }
    }
private:
    typedef enum
    {
        FUSION_GYRO = 0,
        FUSION_ACCEL,
        FUSION_MAGNET,
        FUSION_UNKNOWN
    }FusionType_t;

    FusionType_t ASensorTypeToFusionType(const ASensorEvent &sample)
    {
        switch(sample.type)
        {
            case ASENSOR_TYPE_GYROSCOPE:
            {
                return FUSION_GYRO;
            }
            case ASENSOR_TYPE_ACCELEROMETER:
            {
                return FUSION_ACCEL;
            }
            case ASENSOR_TYPE_MAGNETIC_FIELD:
            {
                return FUSION_MAGNET;
            }
            default:
            {
                ALOGE("Unknown sensor event type=%d \n", sample.type);
                return FUSION_UNKNOWN;
            }
        }
    }

    bool UpdateEventPeriod(int index, const ASensorEvent &sample)
    {
        if(mLastRawSensorEventTimeStamp[index] != 0)
        {
            long long interval = sample.timestamp - mLastRawSensorEventTimeStamp[index];
            mAvgIntervals[index] = (mAvgIntervals[index] + interval) / 2;
        }
        mLastRawSensorEventTimeStamp[index] = sample.timestamp;
        return true;
    }

    bool PushEventsToFifos(ASensorEvent *events, ssize_t cnt)
    {
        for(ssize_t i=0; i<cnt; i++)
        {
            ASensorEvent *event = &events[i];
            FusionType_t internalType = ASensorTypeToFusionType(*event);
            if(internalType != FUSION_UNKNOWN)
            {
                int index = (int)internalType;
                UpdateEventPeriod(index, *event);
                mClassifiedEvents[index].push_back(*event);
            }
        }

        return mClassifiedEvents[0].size() !=0 && mClassifiedEvents[1].size() != 0;
    }

    int ReadRawSensorEventsFromService(int events)
    {
        int size = mRawSensorEventsBuffer.size();
        if(size < events)
        {
            int sizeNew = events*2;
            ALOGD("extend raw sensor event buffer to %d. %d bytes", sizeNew, sizeNew*sizeof(ASensorEvent));
            mRawSensorEventsBuffer.reserve(sizeNew);
            mRawSensorEventsBuffer.resize(sizeNew);
        }
        ssize_t cnt = ASensorEventQueue_getEvents(mSensorEventQueue, mRawSensorEventsBuffer.data(), events);
        if(cnt != events)
        {
            ALOGE("ERROR: expect read %d, but read %d", events, cnt);
        }
        return cnt;
    }

    // 同步操作用于保证 gyro 和 accel 序列各自的首个sample的 timestamp 相差的距离小于指定的 tolerance.
    // 简单算法: 取 gyro 和 accel 序列首个sample的较新时间戳作为基准时间戳，删除 gyro 和 accel 中比这个时间戳更早的数据
    void SyncImuSamples(long long tolerance)
    {
        long long t0 = mClassifiedEvents[0][0].timestamp;
        long long t1 = mClassifiedEvents[1][0].timestamp;
        if(llabs(t0-t1) > tolerance)
        { // need sync
            int queueIdx = t0 > t1 ? 1 : 0;
            long long base = MAX(t0, t1);
            std::vector<ASensorEvent> &events = mClassifiedEvents[queueIdx];
            std::vector<ASensorEvent>::iterator head = events.begin();
            std::vector<ASensorEvent>::iterator tail = events.begin();
            while(tail != events.end() && tail->timestamp < base)
            {
                tail++;
            }
            events.erase(head, tail);
        }
    }

    // 将同步好的 gyro 和 accel 序列一一对应，生成 SFMessageBodyFrame 序列
    int PackImuSamples()
    {
        int maxPackNum = MIN(mClassifiedEvents[0].size(), mClassifiedEvents[1].size());
        if(maxPackNum==0)
        {
            return 0;
        }

        int size = mSfMsseageQueue.size();
        if(size <= maxPackNum)
        {
            int sizeNew = mSfMsseageQueue.size()*2;
            ALOGD("extend messagequeue to %d. %d bytes", sizeNew, sizeNew*sizeof(OVR::SFMessageBodyFrame));
            mSfMsseageQueue.reserve(sizeNew);
            mSfMsseageQueue.resize(sizeNew);
        }

        if(mNeedResyncTime == true || mSystemTime == 0)
        {
            const long long sampleLatency = 0;// TODO: 这里假设最新的 sample 到达这里的延时为 0.
            long long deviceTimeBegin = MIN(mClassifiedEvents[0].front().timestamp, mClassifiedEvents[1].front().timestamp) - mAvgIntervals[0];
            long long deviceTimeFinish = MAX(mClassifiedEvents[0].back().timestamp, mClassifiedEvents[1].back().timestamp);
            long long offset = deviceTimeFinish - deviceTimeBegin + sampleLatency;

#if 0
            // 使用设备驱动的时间轴，这里假设设备驱动通过 mono_clock 获取时间戳。
            // 理论上，使用这个时间轴更加接近真实的头部跟踪，但是实际使用发现，
            // 这种模式下，预测算法会不停地触发 resync time.
            long long now = deviceTimeBegin;
#else
            // 使用自己的时间轴
            // 这里预测算法大概 1~2秒会触发一次 resync time.
            long long now = GetTicksNanos();
#endif
            mSystemTime = now - offset;
            mDeviceTime = deviceTimeBegin;
            // TODO: fix this.
            // ALOGD("reset headtracking system time =%f", mSystemTime*1e-9);
            mNeedResyncTime = false;
        }

        const long long tolerance = 2*MIN(mAvgIntervals[0], mAvgIntervals[1]);
        for(int i=0; i<maxPackNum; i++)
        {
            OVR::SFMessageBodyFrame &message = mSfMsseageQueue[i];
            long long distBetweenGyroAndAccel = mClassifiedEvents[0][i].timestamp - mClassifiedEvents[1][i].timestamp;
            if(distBetweenGyroAndAccel >= tolerance)
            {
                ALOGD("WARNING! Mismatch detected for {gyro=%lld, accel=%lld}, dist=%lld, tolerance=%lld",
                    mClassifiedEvents[0][i].timestamp, mClassifiedEvents[1][i].timestamp, distBetweenGyroAndAccel, tolerance);
            }

            switch(mLcdOrient)
            {
                case 2:
                {
                    message.RotationRate = VECTOR_CONVERSION_2(mClassifiedEvents[0][i].vector);
                    message.Acceleration = VECTOR_CONVERSION_2(mClassifiedEvents[1][i].vector);
                    break;
                }
                case 1:
                default:
                {
                    message.RotationRate = VECTOR_CONVERSION_1(mClassifiedEvents[0][i].vector);
                    message.Acceleration = VECTOR_CONVERSION_1(mClassifiedEvents[1][i].vector);
                    break;
                }
            }
            message.MagneticField = OVR::Vector3f::ZERO;
            message.MagneticBias = OVR::Vector3f::ZERO;
            message.Temperature = 0;

            // 将 device time 转为 system time.
            long long deviceNow = mClassifiedEvents[0][i].timestamp;
            long long delta = deviceNow - mDeviceTime;
            long long systemNow = mSystemTime + delta;
            mDeviceTime = deviceNow; // update device time.
            mSystemTime = systemNow; // update system time
            message.TimeDelta = delta*1e-9;
            message.AbsoluteTimeSeconds = mSystemTime*1e-9;
        }

        // clear used samples.
        RemoveFromHead(mClassifiedEvents[0], maxPackNum);
        RemoveFromHead(mClassifiedEvents[1], maxPackNum);
        return maxPackNum;
    }

    // 将各个 magnet 根据其时间戳插入到对应的 SFMessageBodyFrame 中。
    void PackMagnetSamples(int packedImuSamples, long long maxImuInterval)
    {
        int maxPackNum = packedImuSamples;
        int magnetCnt = mClassifiedEvents[2].size();
        if(magnetCnt == 0 || maxPackNum == 0)
        {
            return;
        }

        long long msgHead = (long long)mSfMsseageQueue[0].AbsoluteTimeSeconds * 1e9;
        long long msgTail = (long long)mSfMsseageQueue[maxPackNum-1].AbsoluteTimeSeconds * 1e9;
        int magnetInsertedCnt = 0;
        for(int i=0; i<magnetCnt; i++)
        {
            long long timestamp = mClassifiedEvents[2][i].timestamp;
            if(timestamp < msgHead)
            {
                //TODO: should we drop it?
                switch(mLcdOrient)
                {
                    case 2:
                    {
                        mSfMsseageQueue[0].MagneticField = VECTOR_CONVERSION_2(mClassifiedEvents[2][i].vector);
                        break;
                    }
                    case 1:
                    default:
                    {
                        mSfMsseageQueue[0].MagneticField = VECTOR_CONVERSION_1(mClassifiedEvents[2][i].vector);
                        break;
                    }
                }
                magnetInsertedCnt++;
            }
            else if(timestamp > msgTail)
            {
                // finish.
                break;
            }
            else
            {
                // msgHead <= timestamp <= msgTail
                int nearest = MAX((timestamp-msgHead+maxImuInterval)/maxImuInterval , maxPackNum-1);
                long long minDist = llabs(timestamp - 1e9*mSfMsseageQueue[nearest].AbsoluteTimeSeconds);
                int mask[2] = {-1,1};
                for(int k=0; k<2; k++)
                {
                    int pos = nearest + mask[0];
                    if(pos >=0 && pos <= maxPackNum-1)
                    {
                        long long dist = llabs(timestamp - 1e9*mSfMsseageQueue[pos].AbsoluteTimeSeconds);
                        if(dist < minDist)
                        {
                            minDist = dist;
                            nearest = pos;
                        }
                    }
                }

                switch(mLcdOrient)
                {
                    case 2:
                    {
                        mSfMsseageQueue[nearest].MagneticField = VECTOR_CONVERSION_2(mClassifiedEvents[2][i].vector); // insert to builded.
                        break;
                    }
                    case 1:
                    default:
                    {
                        mSfMsseageQueue[nearest].MagneticField = VECTOR_CONVERSION_1(mClassifiedEvents[2][i].vector); // insert to builded.
                        break;
                    }
                }
                magnetInsertedCnt++;
            }
        }
        RemoveFromHead(mClassifiedEvents[2], magnetInsertedCnt);
    }

    int BuildSfMessagesFromRawSensorEvents()
    {
        long long maxImuInterval = MAX(mAvgIntervals[0], mAvgIntervals[1]);
        SyncImuSamples(maxImuInterval); // 让 gyro event 和 accel event 的 timestamp 之间的差距不超过 maxImuInterval
        int maxPackNum = PackImuSamples(); // 将 gyro event 和 accel event 一一匹配。
        PackMagnetSamples(maxPackNum, maxImuInterval); // 将 magnet event 插入到匹配好的 list {gyro,accel} 中。
        return maxPackNum;
    }

    void RemoveFromHead(std::vector<ASensorEvent> &queue, int count)
    {
        if(count <= 0) return;
        if(count == 1)
        {
            queue.erase(queue.begin());
        }
        else
        {
            std::vector<ASensorEvent>::iterator head = queue.begin();
            std::vector<ASensorEvent>::iterator tail = head + count;
            queue.erase(head, tail);
        }
    }

private:
    OVR::SensorFusion &mFusion;

    ALooper *mLooper;
    ASensorManager *mSensorMgr;
    ASensorEventQueue *mSensorEventQueue;
    ASensorRef   mGyro;
    ASensorRef   mAccel;
    ASensorRef   mMag;

    int mLcdOrient = 0;
    bool mNeedResyncTime = true;
    long long mDeviceTime = 0;
    long long mSystemTime = 0;
    long long mLastRawSensorEventTimeStamp[3] = {0};// 0 = gyro, 1 = accel, 2 = magnet.
    long long mAvgIntervals[3] = {(long long)1e9/MPU_SAMPLE_RATE, (long long)1e9/MPU_SAMPLE_RATE, (long long)1e9/MAG_RATE};// 0 = gyro, 1 = accel, 2 = magnet.
    std::vector<ASensorEvent> mRawSensorEventsBuffer;
    std::vector<ASensorEvent> mClassifiedEvents[3]; // classified sensor events, 0=gyro, 1=accel, 2=magnet
    std::vector<OVR::SFMessageBodyFrame> mSfMsseageQueue; // packed messages from matched sensor events
};


void* AwSensorThreadImpl::ThreadFunc(void *arg)
{
    ALOGD("Allwinner VR Sensor Hal Thread Starting .\n");
    pthread_setname_np( pthread_self(), "sys_headtracking" );
    pthread_detach(pthread_self());

    AWASSERT( NULL!=arg );
    SensorThreadParms_t *pLocal = (SensorThreadParms_t*)arg;
    SensorDeviceLocal *pDevLocal = new SensorDeviceLocal( *pLocal->pFusion );
    AWASSERT( NULL!=pDevLocal );
    if( false == pDevLocal->Init() )
    {
        delete pDevLocal;
        pDevLocal = NULL;
        pLocal->bStop = true; //let caller to free the ptr
        return NULL;
    }

    while( 1 )
    {
        if(pLocal->bStop == true)
        {
            pDevLocal->Exit();
            break;
        }
        else
        {
            int events = 0;
            if( true == pDevLocal->PollEvents( events ) )
            {
                pDevLocal->handleMessage( events );
            }
            else
            {
                //TODO. improve robust
            }
        }
    }

    delete pDevLocal;
    pDevLocal = NULL;
    delete pLocal->pFusion;
    pLocal->pFusion = NULL;
    delete pLocal;
    pLocal = NULL;
    ALOGD("SensorThread Exited");
    return NULL;
}

void AwSensorThreadImpl::StopThread()
{
    ALOGD(" Stop Sensor Thread ");
    if( NULL != mCurrent )
    {
        if(true == mCurrent->bStop)
        {
            ALOGE("ERROR: SensorThread Meet Error");
            delete mCurrent->pFusion;
            mCurrent->pFusion = NULL;
            delete mCurrent;
        }
        else
        {
            mCurrent->bStop = true;
        }
        mCurrent = NULL;
    }
}

OVR::SensorFusion* AwSensorThreadImpl::StartThread()
{
    if( mCurrent != NULL )
    {
        return mCurrent->pFusion;
    }

    ALOGD("try to start sensor thread");
    // TODO: fix this hack.
    if ( OVR::Allocator::GetInstance() == NULL) {
        OVR::Allocator* palloc = OVR::DefaultAllocator::InitSystemSingleton();
        OVR::Allocator::setInstance(palloc);
    }

    SensorThreadParms_t *pThreadParms = new SensorThreadParms_t;
    pThreadParms->pFusion   = new OVR::SensorFusion();
    pThreadParms->bStop     = false;

    pthread_t threadID = 0;
    if( 0 != pthread_create(&threadID, NULL, ThreadFunc, (void *)pThreadParms) )
    {
        ALOGE("create headtracking thread failed. error=%d(%s)", errno, strerror(errno));
        delete pThreadParms->pFusion;
        delete pThreadParms;
        return NULL;
    }
    ALOGD("start headtracking thread ok");
    return pThreadParms->pFusion;
}

AwSensorThreadImpl::AwSensorThreadImpl() : mCurrent(NULL){}
AwSensorThreadImpl::~AwSensorThreadImpl()
{
    StopThread();
}
