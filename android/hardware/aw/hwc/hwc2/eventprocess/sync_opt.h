
#ifndef __sync_opt_h__
#define __sync_opt_h__

void sync_opt_init(int timeline_fd);
void sync_opt_forward(unsigned int framecount);
void sync_opt_releaseall(void);
void sync_opt_vsync_handler(hwc2_display_t display, uint64_t framenumber, int64_t timestamp);

#endif
