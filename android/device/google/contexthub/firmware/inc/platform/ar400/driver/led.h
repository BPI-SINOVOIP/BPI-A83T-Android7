/*
 * include/drivers/led.h
 *
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: xiafeng <xiafeng@allwinnertech.com>
 *
 */
#ifndef __LED_H__
#define __LED_H__

#if ((defined BOOT_USED) || (defined TF_USED))
extern void led_sysconfig_cfg(void);
extern unsigned int led_get_powercfg(void);
extern unsigned int led_get_statecfg(void);
#else
static inline void led_sysconfig_cfg(void) { }
#endif
#if LED_BLN_USED
extern int led_bln_cfg(unsigned int rgb, unsigned int onms,  unsigned int offms, \
                       unsigned int darkms);
extern int led_bln_init(void);
extern s32 led_bln_adjust(void *arg);
extern int led_bln_suspend(void *arg);
extern int led_bln_resume(void *arg);
#else
static inline int led_bln_init(void) { return 0; }
static inline s32 led_bln_adjust(void *arg) { return 0; }
static inline int led_bln_suspend(void *arg) { return 0; }
static inline int led_bln_resume(void *arg) { return 0; }
#endif
#endif
