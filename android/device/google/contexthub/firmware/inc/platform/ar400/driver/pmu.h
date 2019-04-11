/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                pmu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pmu.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-8
* Descript: power management unit module public header.
* Update  : date                auther      ver     notes
*           2012-5-8 8:52:39    Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __PMU_H__
#define __PMU_H__

#define NMI_INT_TYPE_PMU (0)
#define NMI_INT_TYPE_RTC (1)
#define NMI_INT_TYPE_PMU_OFFSET (0x1 << NMI_INT_TYPE_PMU)
#define NMI_INT_TYPE_RTC_OFFSET (0x1 << NMI_INT_TYPE_RTC)

#if (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1)
//AW1636 IIC address
#define PMU_IIC_ADDR            (0x34)

//the AW1636 pmu type no
#define AW1636_PMU_NO           (0x6)

//the AW1655 pmu type no
#define AW1655_PMU_NO           (0x6)

//the AW1652 pmu type no
#define AW1652_PMU_NO           (0x7)

//AW1636 registers list
#define AW1636_PWR_SRC_STA      (0x00)
#define AW1636_PWRM_CHGR_STA    (0x01)
#define AW1636_IC_NO_REG        (0x03)
#define AW1636_DATA_BUFF0       (0x04)
#define AW1636_OUTPUT_PWR_CTRL0 (0x10)
#define AW1636_OUTPUT_PWR_CTRL1 (0x12)
#define AW1636_ALDO123_OP_MODE  (0x13)
#define AW1636_DLDO1_VOL_CTRL   (0x15)
#define AW1636_DLDO2_VOL_CTRL   (0x16)
#define AW1636_DLDO3_VOL_CTRL   (0x17)
#define AW1636_DLDO4_VOL_CTRL   (0x18)
#define AW1636_ELDO1_VOL_CTRL   (0x19)
#define AW1636_ELDO2_VOL_CTRL   (0x1A)
#define AW1636_ELDO3_VOL_CTRL   (0x1B)
#define AW1636_DC5LDO_VOL_CTRL  (0x1C)
#define AW1636_DCDC1_VOL_CTRL   (0x21)
#define AW1636_DCDC2_VOL_CTRL   (0x22)
#define AW1636_DCDC3_VOL_CTRL   (0x23)
#define AW1636_DCDC4_VOL_CTRL   (0x24)
#define AW1636_DCDC5_VOL_CTRL   (0x25)
#define AW1636_DCDC23_DVM_CTRL  (0x27)
#define AW1636_ALDO1_VOL_CTRL   (0x28)
#define AW1636_ALDO2_VOL_CTRL   (0x29)
#define AW1636_ALDO3_VOL_CTRL   (0x2a)
#define AW1636_VBUS_CTRL        (0x30)
#define AW1636_PWR_WAKEUP_CTRL  (0x31)
#define AW1636_PWR_OFF_CTRL     (0x32)
#define AW1636_CHARGER_CTRL1    (0x33)
#define AW1636_CHARGER_CTRL2    (0x34)
#define AW1636_CHARGER_CTRL3    (0x35)
#define AW1636_IRQ_ENABLE1      (0x40)
#define AW1636_IRQ_ENABLE2      (0x41)
#define AW1636_IRQ_ENABLE3      (0x42)
#define AW1636_IRQ_ENABLE4      (0x43)
#define AW1636_IRQ_ENABLE5      (0x44)
#define AW1636_IRQ_STATUS1      (0x48)
#define AW1636_IRQ_STATUS2      (0x49)
#define AW1636_IRQ_STATUS3      (0x4a)
#define AW1636_IRQ_STATUS4      (0x4b)
#define AW1636_IRQ_STATUS5      (0x4c)
#define AW1636_IC_TEMP_HIG      (0x56)
#define AW1636_IC_TEMP_LOW      (0x57)
#define AW1636_ADC_DATA_HIGH    (0x58)
#define AW1636_ADC_DATA_LOW     (0x59)
#define AW1636_CHAR_CURR_STA1   (0x7a)
#define AW1636_CHAR_CURR_STA2   (0x7b)
#define AW1636_BAT_QUANTITY     (0xb9)
#define AW1636_INVALID_ADDR     (0xff)

//1636 pmu voltage types
typedef enum power_voltage_type
{
	POWER_VOL_DC5LDO = 0x0,
	POWER_VOL_DCDC1,
	POWER_VOL_DCDC2,
	POWER_VOL_DCDC3,
	POWER_VOL_DCDC4,
	POWER_VOL_DCDC5,
	POWER_VOL_ALDO1,
	POWER_VOL_ALDO2,

	POWER_VOL_ELDO1,
	POWER_VOL_ELDO2,
	POWER_VOL_ELDO3,
	POWER_VOL_DLDO1,
	POWER_VOL_DLDO2,
	POWER_VOL_DLDO3,
	POWER_VOL_DLDO4,
	POWER_VOL_DC1SW,

	POWER_VOL_ALDO3,

	POWER_VOL_MAX
} power_voltage_type_e;

//1636 pmu power key types
typedef enum power_key_type
{
	POWER_KEY_SHORT = 1,
	POWER_KEY_LONG  = 2,
	POWER_LOW_POWER = 3,
} power_key_type_e;

typedef enum power_voltage_state
{
	POWER_VOL_OFF = 0x0,
	POWER_VOL_ON  = 0x1,
} power_voltage_state_e;

#if PMU_USED
extern s32 pmu_init(void);
extern s32 pmu_exit(void);
#else
#define pmu_init() (0)
#define pmu_exit() (0)
#endif

//if the set vlotage is not intergel multiple defined by pmu spec, round up it to an integer
//eg. the step of DC-DC1 is 100mV, if set 280mV, the true set value is 300mV
s32 pmu_set_voltage(u32 type, u32 voltage); //the voltage base on mV
s32 pmu_get_voltage(u32 type);
s32 pmu_set_voltage_state(u32 type, u32 state);

//register read and write
s32 pmu_reg_read(u8 *devaddr, u8 *regaddr, u8 *data, u32 len);
s32 pmu_reg_write(u8 *devaddr, u8 *regaddr, u8 *data, u32 len);

s32 pmu_get_chip_id(struct message *pmessage);

//pmu standby process
s32 pmu_standby_init(u32 event, u32 gpio_bitmap);
s32 pmu_standby_exit(void);
s32 pmu_query_event(u32 *event);

s32 pmu_set_paras(struct message *pmessage);
s32 pmu_is_in_safe_range(void);
#if PMU_CHRCUR_CRTL_USED
void pmu_contrl_batchrcur(void);
#endif
s32 pmu_get_batconsum(void);
u32 pmu_get_powerstate(u32 power_reg);

#elif (defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1)
//AW1660 IIC address
#define PMU_IIC_ADDR            (0x34)

//the AW1636 pmu type no
#define AW1660_PMU_NO           (0x6)

//AW1660 registers list
#define AW1660_PWR_SRC_STA      (0x00)
#define AW1660_PWRM_CHGR_STA    (0x01)
#define AW1660_PWR_REASON       (0x02)
#define AW1660_IC_NO_REG        (0x03)
#define AW1660_DATA_BUFF1       (0x04)
#define AW1660_DATA_BUFF2       (0x05)
#define AW1660_DATA_BUFF3       (0x06)
#define AW1660_DATA_BUFF4       (0x07)
#define AW1660_DATA_BUFF5       (0x08)
#define AW1660_DATA_BUFF6       (0x09)
#define AW1660_DATA_BUFF7       (0x0A)
#define AW1660_DATA_BUFF8       (0x0B)
#define AW1660_DATA_BUFF9       (0x0C)
#define AW1660_DATA_BUFFA       (0x0D)
#define AW1660_DATA_BUFFB       (0x0E)
#define AW1660_DATA_BUFFC       (0x0F)
#define AW1660_OUTPUT_PWR_CTRL0 (0x10)
#define AW1660_OUTPUT_PWR_CTRL1 (0x12)
#define AW1660_ALDO123_OP_MODE  (0x13)
#define AW1660_ON_OFF_SYN_CTRL  (0x14)
#define AW1660_DLDO1_VOL_CTRL   (0x15)
#define AW1660_DLDO2_VOL_CTRL   (0x16)
#define AW1660_DLDO3_VOL_CTRL   (0x17)
#define AW1660_DLDO4_VOL_CTRL   (0x18)
#define AW1660_ELDO1_VOL_CTRL   (0x19)
#define AW1660_ELDO2_VOL_CTRL   (0x1A)
#define AW1660_ELDO3_VOL_CTRL   (0x1B)
#define AW1660_FLDO1_VOL_CTRL   (0x1C)
#define AW1660_FLDO23_VOL_CTRL  (0x1D)
#define AW1660_DCDC1_VOL_CTRL   (0x20)
#define AW1660_DCDC2_VOL_CTRL   (0x21)
#define AW1660_DCDC3_VOL_CTRL   (0x22)
#define AW1660_DCDC4_VOL_CTRL   (0x23)
#define AW1660_DCDC5_VOL_CTRL   (0x24)
#define AW1660_DCDC6_VOL_CTRL   (0x25)
#define AW1660_DCDC7_VOL_CTRL   (0x26)
#define AW1660_DCDC_DVM_CTRL    (0x27)
#define AW1660_ALDO1_VOL_CTRL   (0x28)
#define AW1660_ALDO2_VOL_CTRL   (0x29)
#define AW1660_ALDO3_VOL_CTRL   (0x2a)
#define AW1660_BC_MOD_GLB_REG   (0x2c)
#define AW1660_BC_MOD_VBUS_REG  (0x2d)
#define AW1660_BC_USB_STA_REG   (0x2e)
#define AW1660_BC_DET_STA_REG   (0x2f)
#define AW1660_VBUS_CTRL        (0x30)
#define AW1660_PWR_WAKEUP_CTRL  (0x31)
#define AW1660_PWR_OFF_CTRL     (0x32)
#define AW1660_CHARGER_CTRL1    (0x33)
#define AW1660_CHARGER_CTRL2    (0x34)
#define AW1660_CHARGER_CTRL3    (0x35)
#define AW1660_POK_SET          (0x36)
#define AW1660_POK_PWR_OFF_SET  (0x37)
#define AW1660_VLTF_CHARGE_SET  (0x38)
#define AW1660_VHTF_CHARGE_SET  (0x39)
#define AW1660_ACIN_PATH_CTRL   (0x3a)
#define AW1660_DCDC_FREQ_SET    (0x3b)
#define AW1660_VLTF_WORK_SET    (0x3c)
#define AW1660_VHTF_WORK_SET    (0x3d)
#define AW1660_IF_MODE_SEL      (0x3e)
#define AW1660_IRQ_ENABLE1      (0x40)
#define AW1660_IRQ_ENABLE2      (0x41)
#define AW1660_IRQ_ENABLE3      (0x42)
#define AW1660_IRQ_ENABLE4      (0x43)
#define AW1660_IRQ_ENABLE5      (0x44)
#define AW1660_IRQ_ENABLE6      (0x45)
#define AW1660_IRQ_STATUS1      (0x48)
#define AW1660_IRQ_STATUS2      (0x49)
#define AW1660_IRQ_STATUS3      (0x4a)
#define AW1660_IRQ_STATUS4      (0x4b)
#define AW1660_IRQ_STATUS5      (0x4c)
#define AW1660_IRQ_STATUS6      (0x4d)
#define AW1660_IC_TEMP_HIG      (0x56)
#define AW1660_IC_TEMP_LOW      (0x57)
#define AW1660_CHAR_CURR_STA1   (0x7a)
#define AW1660_CHAR_CURR_STA2   (0x7b)
#define AW1660_GPIOLDO0_CTRL    (0x90)
#define AW1660_GPIOLDO0_VOL     (0x91)
#define AW1660_GPIOLDO1_CTRL    (0x92)
#define AW1660_GPIOLDO1_VOL     (0x93)
#define AW1660_BAT_QUANTITY     (0xb9)
#define AW1660_INVALID_ADDR     (0x100)

//pmu voltage types
typedef enum power_voltage_type
{
	AW1660_POWER_DCDC1 = 0x0,
	AW1660_POWER_DCDC2,
	AW1660_POWER_DCDC3,
	AW1660_POWER_DCDC4,
	AW1660_POWER_DCDC5,
	AW1660_POWER_DCDC6,
	AW1660_POWER_DCDC7,
	AW1660_POWER_ALDO1,
	AW1660_POWER_ALDO2,
	AW1660_POWER_ALDO3,
	AW1660_POWER_DLDO1,
	AW1660_POWER_DLDO2,
	AW1660_POWER_DLDO3,
	AW1660_POWER_DLDO4,
	AW1660_POWER_ELDO1,
	AW1660_POWER_ELDO2,
	AW1660_POWER_ELDO3,
	AW1660_POWER_FLDO1,
	AW1660_POWER_FLDO2,
	AW1660_POWER_FLDO3,
	AW1660_POWER_LDOIO0,
	AW1660_POWER_LDOIO1,
	AW1660_POWER_DC1SW,
	AW1660_POWER_RTC,
	AW1660_POWER_MAX,
} power_voltage_type_e;

//1660 pmu power key types
typedef enum power_key_type
{
	POWER_KEY_SHORT = 1,
	POWER_KEY_LONG  = 2,
	POWER_LOW_POWER = 3,
} power_key_type_e;

typedef enum power_voltage_state
{
	POWER_VOL_OFF = 0x0,
	POWER_VOL_ON  = 0x1,
} power_voltage_state_e;

typedef enum regulator_mode
{
	REGU_MODE_AUTO = 0x0,
	REGU_MODE_PWM  = 0x1,
} regulator_mode_e;

//keep the struct word align
//by superm at 2014-2-13 15:34:09
typedef struct pmu_onoff_reg_bitmap
{
	u16 devaddr;
	u16 regaddr;
	u16 offset;
	u8  state;
	u8  dvm_st;
} pmu_onoff_reg_bitmap_t;
extern pmu_onoff_reg_bitmap_t pmu_onoff_reg_bitmap[];

#if PMU_USED
extern s32 pmu_init(void);
extern s32 pmu_exit(void);
#else
#define pmu_init()     (0)
#define pmu_exit()     (0)
#endif

#if (THERMAL_USED | CHECK_USED | SYSOP_USED)
void pmu_shutdown(void);
#endif

#if SYSOP_USED
void pmu_reset(void);
#endif

s32 pmu_set_voltage(u32 type, u32 voltage); //the voltage base on mV
s32 pmu_get_voltage(u32 type);
s32 pmu_set_voltage_state(u32 type, u32 state);
s32 pmu_set_mode(u32 type, regulator_mode_e mode);

//register read and write
s32 pmu_reg_read(u8 *devaddr, u8 *regaddr, u8 *data, u32 len);
s32 pmu_reg_write(u8 *devaddr, u8 *regaddr, u8 *data, u32 len);

s32 pmu_get_chip_id(struct message *pmessage);

//pmu standby process
s32 pmu_standby_init(u32 event, u32 gpio_bitmap);
s32 pmu_standby_exit(void);
s32 pmu_query_event(u32 *event);
int get_nmi_int_type(void);
u32 pmu_output_is_stable(u32 type);
s32 pmu_get_voltage_state(u32 type);
s32 pmu_set_paras(struct message *pmessage);
s32 set_pwr_tree(struct message *pmessage);
#if PMU_CHRCUR_CRTL_USED
void pmu_contrl_batchrcur(void);
#endif
s32 pmu_get_batconsum(void);
u32 pmu_get_powerstate(u32 power_reg);
#elif (defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
//AW1657 IIC address
#define PMU_IIC_ADDR            (0x36)
#define PMU_INV_ADDR            (0x00)
#define PMU_STEP_DELAY_US       (16)

//the AW1657 pmu type no
#define AW1657_PMU_NO           (0x40)
#define AW1657_PMU_NO_MASK      (0xCF)

//AW1657 registers list
#define AW1657_PWR_SRC_STA      (0x00)
#define AW1657_IC_NO_REG        (0x03)
#define AW1657_DATA_BUFF0       (0x04)
#define AW1657_OUTPUT_PWR_CTRL0 (0x10)
#define AW1657_OUTPUT_PWR_CTRL1 (0x11)
#define AW1657_DCDCA_VOL_CTRL   (0x12)
#define AW1657_DCDCB_VOL_CTRL   (0x13)
#define AW1657_DCDCC_VOL_CTRL   (0x14)
#define AW1657_DCDCD_VOL_CTRL   (0x15)
#define AW1657_DCDCE_VOL_CTRL   (0x16)
#define AW1657_ALDO1_VOL_CTRL   (0x17)
#define AW1657_ALDO2_VOL_CTRL   (0x18)
#define AW1657_ALDO3_VOL_CTRL   (0x19)
#define AW1657_DCDC_MODE_CTRL1  (0x1A)
#define AW1657_DCDC_MODE_CTRL2  (0x1B)
#define AW1657_DCDC_FREQ_SET    (0x1C)
#define AW1657_OUTPUT_MON_CTRL  (0x1D)
#define AW1657_IRQ_PWROK_CHARGE (0x1F)
#define AW1657_BLDO1_VOL_CTRL   (0x20)
#define AW1657_BLDO2_VOL_CTRL   (0x21)
#define AW1657_BLDO3_VOL_CTRL   (0x22)
#define AW1657_BLDO4_VOL_CTRL   (0x23)
#define AW1657_CLDO1_VOL_CTRL   (0x24)
#define AW1657_CLDO2_VOL_CTRL   (0x25)
#define AW1657_CLDO3_VOL_CTRL   (0x26)
#define AW1657_PWR_WAKEUP_CTRL  (0x31)
#define AW1657_PWR_OFF_CTRL     (0x32)
#define AW1657_WAKEUP_PIN_CTRL  (0x35)
#define AW1657_POK_SET          (0x36)
#define AW1657_IF_MODE_SEL      (0x3E)
#define AW1657_SPECIAL_CTRL     (0x3F)
#define AW1657_IRQ_ENABLE1      (0x40)
#define AW1657_IRQ_ENABLE2      (0x41)
#define AW1657_IRQ_STATUS1      (0x48)
#define AW1657_IRQ_STATUS2      (0x49)
#define AW1657_REG_ADDR_EX      (0xff)
#define AW1657_INVALID_ADDR     (0x100)

//pmu voltage types
typedef enum power_voltage_type
{
	AW1657_POWER_DCDCA = 0x0,
	AW1657_POWER_DCDCB, //1
	AW1657_POWER_DCDCC, //2
	AW1657_POWER_DCDCD, //3
	AW1657_POWER_DCDCE, //4
	AW1657_POWER_ALDO1, //5
	AW1657_POWER_ALDO2, //6
	AW1657_POWER_ALDO3, //7
	AW1657_POWER_BLDO1, //8
	AW1657_POWER_BLDO2, //9
	AW1657_POWER_BLDO3, //10
	AW1657_POWER_BLDO4, //11
	AW1657_POWER_CLDO1, //12
	AW1657_POWER_CLDO2, //13
	AW1657_POWER_CLDO3, //14
	AW1657_POWER_DC1SW, //15
	AW1657_POWER_MAX,   //16
	DUMMY_REGULATOR1,   //17, AVCC/VCC3V3-PLL/VCC3V3-TV
	DUMMY_REGULATOR2,   //18, DRAM
	DUMMY_REGULATOR3,   //19, SYSTEM
	DUMMY_REGULATOR4,   //20, VCC-CPUX
	DUMMY_REGULATOR5,   //21, WIFI
	DUMMY_REGULATOR6,   //22, VCC-IO
	DUMMY_REGULATOR_MAX,//23
} power_voltage_type_e;

//1660 pmu power key types
typedef enum power_key_type
{
	POWER_KEY_SHORT = 1,
	POWER_KEY_LONG  = 2,
	POWER_LOW_POWER = 3,
} power_key_type_e;

typedef enum power_voltage_state
{
	POWER_VOL_OFF = 0x0,
	POWER_VOL_ON  = 0x1,
} power_voltage_state_e;

//keep the struct word align
//by superm at 2014-2-13 15:34:09
typedef struct pmu_onoff_reg_bitmap
{
	u16 devaddr;
	u16 regaddr;
	u16 offset;
	u8  state;
	u8  dvm_st;
} pmu_onoff_reg_bitmap_t;
extern pmu_onoff_reg_bitmap_t pmu_onoff_reg_bitmap[];

#if PMU_USED
extern s32 pmu_init(void);
extern s32 pmu_exit(void);
extern u32 is_pmu_exist(void);
#else
#define pmu_init()      (0)
#define pmu_exit()      (0)
#define is_pmu_exist()  (0)
#endif

#if (THERMAL_USED | CHECK_USED | SYSOP_USED)
void pmu_shutdown(void);
#endif

#if SYSOP_USED
void pmu_reset(void);
#endif

s32 pmu_set_voltage(u32 type, u32 voltage); //the voltage base on mV
s32 pmu_get_voltage(u32 type);
s32 pmu_set_voltage_state(u32 type, u32 state);

//register read and write
s32 pmu_reg_read(u8 *devaddr, u8 *regaddr, u8 *data, u32 len);
s32 pmu_reg_write(u8 *devaddr, u8 *regaddr, u8 *data, u32 len);

s32 pmu_get_chip_id(struct message *pmessage);

//pmu standby process
s32 pmu_standby_init(u32 event, u32 gpio_bitmap);
s32 pmu_standby_exit(void);
s32 pmu_query_event(u32 *event);
int get_nmi_int_type(void);
u32 pmu_output_is_stable(u32 type);
s32 pmu_get_voltage_state(u32 type);
s32 pmu_set_paras(struct message *pmessage);
s32 set_pwr_tree(struct message *pmessage);
#if PMU_CHRCUR_CRTL_USED
void pmu_contrl_batchrcur(void);
#endif
s32 pmu_get_batconsum(void);
u32 pmu_get_powerstate(u32 power_reg);

#endif



#if (defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
typedef struct pmu_paras
{
	u8 *devaddr;
	u8 *regaddr;
	u8 *data;
	u32 len;
} pmu_paras_t;

typedef struct box_start_os_cfg
{
	u32 used;
	u32 start_type;
	u32 irkey_used;
	u32 pmukey_used;
	u32 pmukey_num;
	u32 led_power;
	u32 led_state;
} box_start_os_cfg_t;

extern s32 pmu_reg_write_para(pmu_paras_t *para);
extern s32 pmu_reg_read_para(pmu_paras_t *para);
#endif

#if ((defined BOOT_USED) || (defined TF_USED))
extern void pmu_sysconfig_cfg(void);
extern void pmu_set_lowpcons(void);
extern void pmu_sys_lowpcons(void);
extern s32 pmu_set_pok_time(int time);
extern void pmu_poweroff_system(void);
extern void pmu_reset_system(void);
extern int pmu_set_gpio(unsigned int id, unsigned int state);
extern s32 pmu_clear_pendings(void);
extern bool pmu_pin_detect(void);
#endif
#endif  //__PMU_H__
