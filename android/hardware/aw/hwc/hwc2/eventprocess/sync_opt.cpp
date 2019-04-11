
#include <stdint.h>
#include <memory.h>
#include <pthread.h>
#include <cutils/log.h>
#include <hardware/hwcomposer2.h>
#define MAX_PENDING_FRAME   16

//extern int sw_sync_timeline_inc(int fd, unsigned count);
#define sw_sync_timeline_inc(fmt, args...)

typedef struct frame_track {
    unsigned int framecount;
    uint32_t hardware_pt;
} frame_track_t;

struct sync_opt {
    uint32_t hardware_timeline;

    frame_track_t track[16];
    int pending;

    pthread_mutex_t mutex;
    int timeline_fd;
};

static struct sync_opt syncopt;
void sync_opt_init(int timeline_fd)
{
    memset(&syncopt, 0, sizeof(syncopt));
    syncopt.timeline_fd = timeline_fd;
    pthread_mutex_init(&syncopt.mutex, 0);
}

void sync_opt_forward(unsigned int framecount)
{
    frame_track_t *track;
    pthread_mutex_lock(&syncopt.mutex);
    if (syncopt.pending >= MAX_PENDING_FRAME)
        goto __out;

    track = &syncopt.track[syncopt.pending++];
    track->framecount   = framecount;
    track->hardware_pt = syncopt.hardware_timeline;
__out:
    pthread_mutex_unlock(&syncopt.mutex);
}

void sync_opt_releaseall(void)
{
    pthread_mutex_lock(&syncopt.mutex);
    if (!syncopt.pending)
        goto __out;

    sw_sync_timeline_inc(syncopt.timeline_fd, syncopt.pending);
    syncopt.pending = 0;
    memset(syncopt.track, 0, sizeof(syncopt.track));

__out:
    pthread_mutex_unlock(&syncopt.mutex);
}

void sync_opt_vsync_handler(hwc2_display_t display, uint64_t framenumber, int64_t timestamp)
{
    pthread_mutex_lock(&syncopt.mutex);

    syncopt.hardware_timeline++;
    if (!syncopt.pending)
        goto __out;

    sw_sync_timeline_inc(syncopt.timeline_fd, syncopt.pending);
    syncopt.pending = 0;
    memset(syncopt.track, 0, sizeof(syncopt.track));

__out:
    pthread_mutex_unlock(&syncopt.mutex);
}

