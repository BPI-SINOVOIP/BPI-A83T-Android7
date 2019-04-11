#ifndef	__TRACE_TIME_H
#define	__TRACE_TIME_H

#include <plat/inc/include.h>
#if TRACETIME_USED
#define TRACE_TIMER_DEPTH 50 /* record 50 time, average */
#define TRACE_TIMER_N_OBJS 9

extern void trace_time_tag(unsigned int index);
extern void trace_time_record(unsigned int index);
extern int tarce_time_show(int index);
#else
static inline void trace_time_tag(unsigned int index) { }
static inline void trace_time_record(unsigned int index) { }
static inline int tarce_time_show(unsigned int index) { return 0; }
#endif
#endif	/* __TRACE_TIME_H */
