#include <plat/inc/include.h>

#if TRACETIME_USED

static volatile unsigned long trace_time_run = 0;

#define trace_time_init() cpucfg_counter_clear()

#define timer24m_counter_read() cpucfg_counter_read()

static volatile u32 trace_time_buf[TRACE_TIMER_N_OBJS][TRACE_TIMER_DEPTH];
static volatile u64 trace_time_last[TRACE_TIMER_N_OBJS];
static volatile u32 trace_time_num[TRACE_TIMER_N_OBJS];

void trace_time_tag(unsigned int index)
{
	//BUG_ON(index >= TRACE_TIMER_N_OBJS);

	if (!trace_time_run)
		return ;

	trace_time_last[index] = timer24m_counter_read();
}

void trace_time_record(unsigned int index)
{
	u64 temp;

	//BUG_ON(index >= TRACE_TIMER_N_OBJS);
	if (!trace_time_run)
		return ;

	temp = timer24m_counter_read();
	/* we should check && (u32)trace_time_last[index]) if varible
	 * trace_time_run changed between tag and record. but tag and record is
	 * used in paire during interrupt disable environment, so only check
	 * num is little than DEPTH is ok.
	 */
	if ((trace_time_num[index] < TRACE_TIMER_DEPTH))
		trace_time_buf[index][trace_time_num[index]++] = \
		    (u32)(temp - trace_time_last[index]);
}

int tarce_time_show(int index)
{
	unsigned int len, ret = 0;
	unsigned int i, j;
	u64 sum;

	switch (index) {
	case 0:
		for (i = 0; i < TRACE_TIMER_N_OBJS; i++) {
			sum = 0;
			for (j = 0; j < trace_time_num[i]; j++)
				sum += (u64)trace_time_buf[i][j];
			LOG("sum[%d]:%d%d, num:%d\n", i, (u32)(sum>>32), \
			    (u32)(sum), trace_time_num[i]);
		}
		break;
	case 1:
		trace_time_run = 1;
		break;
	case 2:
		trace_time_run = 0;
		break;
	case 3:
		trace_time_run = 0;
		for (i = 0; i < TRACE_TIMER_N_OBJS; i++) {
			trace_time_num[i] = 0;
			trace_time_last[i] = 0;
		}
		break;
	default:
		/* programmer goofed */
		//WARN_ON_ONCE(1);
		break;
	}

	return ret;
}

#endif
