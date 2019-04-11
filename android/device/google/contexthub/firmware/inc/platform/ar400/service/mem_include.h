#ifndef __MEM_INCLUDE_H__
#define __MEM_INCLUDE_H__

#include <plat/inc/include.h>
#include "../../../src/platform/ar400/service/standby/suspend_resume/mem_spc.h"
#include "../../../src/platform/ar400/service/standby/suspend_resume/mem_tmstmp.h"
#include "../../../src/platform/ar400/service/standby/suspend_resume/mem_gpio_sun50iw3.h"
#include "../../../src/platform/ar400/service/standby/suspend_resume/mem_timer_sun50iw3.h"
#include "../../../src/platform/ar400/service/standby/suspend_resume/mem_syscfg_sun50iw3.h"
#include "../../../src/platform/ar400/service/standby/suspend_resume/mem_ccu_sun50iw3.h"
#include "../../../src/platform/ar400/service/standby/suspend_resume/mem_smc_sun50iw3.h"

#if STANDBY_USED
#ifdef DEBUG_ON
/* debug levels */
#define MEM_DEBUG_LEVEL_INF    ((u32)1 << 0)
#define MEM_DEBUG_LEVEL_LOG    ((u32)1 << 1)
#define MEM_DEBUG_LEVEL_WRN    ((u32)1 << 2)
#define MEM_DEBUG_LEVEL_ERR    ((u32)1 << 3)

#if INF_USED
#define MEM_INF(...)    mem_debugger_printf(MEM_DEBUG_LEVEL_INF,__VA_ARGS__)
#else
#define MEM_INF(...)
#endif

#if WRN_USED
#define MEM_WRN(...)    mem_debugger_printf(MEM_DEBUG_LEVEL_WRN,__VA_ARGS__)
#else
#define MEM_WRN(...)
#endif

#if ERR_USED
#define MEM_ERR(...)    mem_debugger_printf(MEM_DEBUG_LEVEL_ERR,__VA_ARGS__)
#else
#define MEM_ERR(...)
#endif

#if LOG_USED
#define MEM_LOG(...)    mem_debugger_printf(MEM_DEBUG_LEVEL_LOG,__VA_ARGS__)
#else
#define MEM_LOG(...)
#endif

#else
#define MEM_INF(...)
#define MEM_WRN(...)
#define MEM_ERR(...)
#define MEM_LOG(...)
#endif

#define mem_printk(...)    mem_debugger_printf(0xff,__VA_ARGS__)

#if MEM_USED
typedef struct system_state {
	struct gpio_state gpio_back;
	u32 reserve_back1[2];
	struct smc_regs smc_back;
	u32 reserve_back2[2];
	struct spc_regs spc_back;
	u32 reserve_back3[2];
	mem_timer_reg_t timer_back;
	u32 reserve_back4[2];
	struct syscfg_state syscfg_back;
	u32 reserve_back5[2];
	struct mem_tmstmp_ctrl_reg tmstmp_back;
	struct ccm_state ccm_back;
	u32 reserve_back6[2];
}system_state_t;

extern struct system_state *system_back; /* for back all registers */
extern void *mem_memcpy(void *dest, const void *src, size_t n);
extern void *mem_memset(void *s, int c, size_t n);
extern int mem_long_jump(int (*fn)(u32 arg), u32 arg);
extern void mem_reg_debug(const char *name, const unsigned int *start, unsigned int len);
extern void mem_hexdump(char* name, char* base, int len);
extern s32 mem_debugger_printf(u32 level, const char *format, ...);
extern void mem_save_state_flag(u32 reg, u32 value);
extern int mem_ccu_save(void);
extern int mem_gpio_save(struct extended_standby *config);
extern int mem_timer_save(void);
extern int mem_syscfg_save(void);
extern int mem_smc_save(void);
extern s32 mem_tmstmp_save(void);
extern int mem_spc_save(void);
extern void mem_reg_save(unsigned int *dest, const unsigned int *src, \
                         unsigned int n, unsigned int skip);
extern void mem_set_wakeup_src(struct super_standby_para *para);
extern int mem_gpio_suspend_cfg(struct super_standby_para *para);
extern int mem_int_suspend_cfg(struct super_standby_para *para);
extern int mem_linux_save(void *arg);

extern int mem_ccu_restore(void);
extern int mem_gpio_restore(struct extended_standby *config);
extern int mem_timer_restore(void);
extern int mem_syscfg_restore(void);
extern int mem_smc_restore(void);
extern s32 mem_tmstmp_restore(void);
extern int mem_spc_restore(void);
extern void mem_reg_restore(unsigned int *dest, const unsigned int *src, \
                            unsigned int n, unsigned int skip);
extern int mem_gpio_resume_cfg(struct super_standby_para *para);
extern int mem_int_resume_cfg(struct super_standby_para *para);
extern int mem_linux_restore(void *arg);
extern s32 mem_int_enable(u32 intno);
extern s32 mem_int_disable(u32 intno);
extern s32 mem_pmu_standby_init(struct super_standby_para *para);
extern s32 mem_pmu_standby_exit(void);
#else
#define mem_long_jump(fn, arg) (*fn)(arg)
#define mem_debugger_printf debugger_printf
static inline int mem_int_suspend_cfg(struct super_standby_para *para) { return 0; }
static inline int mem_linux_save(void *arg) { return 0; }
static inline int mem_linux_restore(void *arg) { return 0; }
static inline int mem_int_resume_cfg(struct super_standby_para *para) { return 0; }
#endif
#endif
#endif /* __MEM_INCLUDE_H__ */
