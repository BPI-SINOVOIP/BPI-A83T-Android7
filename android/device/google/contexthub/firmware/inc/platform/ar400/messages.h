/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                   messages
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : messages.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-4-26
* Descript: the messages for asynchronous cpus communication.
* Update  : date                auther      ver     notes
*           2012-4-26 10:18:39  Sunny       1.0     Create this file.
*********************************************************************************************************
*/


#ifndef __MESSAGES_H__
#define __MESSAGES_H__

//message attributes(only use 8bit)
#define MESSAGE_ATTR_SOFTSYN        (1<<0)  //need use soft syn with another cpu
#define MESSAGE_ATTR_HARDSYN        (1<<1)  //need use hard syn with another cpu

//message states
#define MESSAGE_FREED               (0x0)   //freed state
#define MESSAGE_ALLOCATED           (0x1)   //allocated state
#define MESSAGE_INITIALIZED         (0x2)   //initialized state
#define MESSAGE_RECEIVED            (0x3)   //received state
#define MESSAGE_PROCESSING          (0x4)   //processing state
#define MESSAGE_PROCESSED           (0x5)   //processed state
#define MESSAGE_FEEDBACKED          (0x6)   //feedback state

#if (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
//call back struct
typedef struct ar100_msg_cb
{
	u64 handler;
	u64 arg;
} ar100_msg_cb_t;

//the structure of message frame,
//this structure will transfer between ar100 and ac327.
//sizeof(struct message) : 128Byte.
typedef struct message
{
	unsigned char          state;       //identify the used status of message frame
	unsigned char          attr;        //message attribute : SYN OR ASYN
	unsigned char          type;        //message type : DVFS_REQ
	unsigned char          result;      //message process result
	unsigned char          reserved[4]; /* reserved for 8byte align */
	u64                    next;        //pointer of next message frame
	struct   ar100_msg_cb  cb;          //the callback function and arg of message
	volatile u64           private;     //message private data
	unsigned int           paras[22];   //the parameters of message
} message_t;
#else
//call back struct
typedef struct ar100_msg_cb
{
	ar100_cb_t   handler;
	void        *arg;
} ar100_msg_cb_t;

//the structure of message frame,
//this structure will transfer between ar100 and ac327.
//sizeof(struct message) : 64Byte.
typedef struct message
{
	unsigned char          state;       //identify the used status of message frame
	unsigned char          attr;        //message attribute : SYN OR ASYN
	unsigned char          type;        //message type : DVFS_REQ
	unsigned char          result;      //message process result
	struct   message      *next;        //pointer of next message frame
	struct   ar100_msg_cb  cb;          //the callback function and arg of message
	volatile u32           private;     //message private data
	unsigned int           paras[11];   //the parameters of message
} message_t;
#endif

//the base of messages
#define MESSAGE_BASE            (0x10)

//standby commands
#define SSTANDBY_ENTER_REQ          (MESSAGE_BASE + 0x00)  /* request to enter(ac327 to ar100)                 */
#define SSTANDBY_RESTORE_NOTIFY     (MESSAGE_BASE + 0x01)  /* restore finished(ac327 to ar100)                 */
#define NSTANDBY_ENTER_REQ          (MESSAGE_BASE + 0x02)  /* request to enter(ac327 to ar100)                 */
#define NSTANDBY_WAKEUP_NOTIFY      (MESSAGE_BASE + 0x03)  /* wakeup notify   (ar100 to ac327)                 */
#define NSTANDBY_RESTORE_REQ        (MESSAGE_BASE + 0x04)  /* request to restore    (ac327 to ar100)           */
#define NSTANDBY_RESTORE_COMPLETE   (MESSAGE_BASE + 0x05)  /* ar100 restore complete(ar100 to ac327)           */
#define ESSTANDBY_ENTER_REQ         (MESSAGE_BASE + 0x06)  /* request to enter       (ac327 to ar100)          */
#define TSTANDBY_ENTER_REQ          (MESSAGE_BASE + 0x07)  /* request to enter(ac327 to ar100)                 */
#define TSTANDBY_RESTORE_NOTIFY     (MESSAGE_BASE + 0x08)  /* restore finished(ac327 to ar100)                 */
#define FAKE_POWER_OFF_REQ          (MESSAGE_BASE + 0x09)  /* fake power off request(ac327 to ar100)           */
#define CPUIDLE_ENTER_REQ           (MESSAGE_BASE + 0x0A)  /* request to enter cpuidle (ac327 to ar200)        */
#define STANDBY_INFO_REQ            (MESSAGE_BASE + 0x10)  /* request sst info(ac327 to arisc)                 */
#define CPUIDLE_CFG_REQ             (MESSAGE_BASE + 0x11)  /* request to config      (ac327 to arisc)          */
#define CPU_OP_REQ                  (MESSAGE_BASE + 0x12)  /* cpu operations         (ac327 to arisc)          */
#define QUERY_WAKEUP_SRC_REQ        (MESSAGE_BASE + 0x13)  /* query wakeup source    (ac327 to arisc)          */
#define SYS_OP_REQ                  (MESSAGE_BASE + 0x14)  /* cpu operations         (ac327 to arisc)          */

//dvfs commands
#define CPUX_DVFS_REQ               (MESSAGE_BASE + 0x20)  /* request dvfs    (ac327 to ar100)                 */
#define CPUX_DVFS_CFG_VF_REQ        (MESSAGE_BASE + 0x21)  /* request config dvfs V-F table (ac327 to ar100)   */

/* pmu commands */
#define AXP_INT_COMING_NOTIFY       (MESSAGE_BASE + 0x40)  /* interrupt coming notify(ar200 to ac327)          */
#define AXP_DISABLE_IRQ             (MESSAGE_BASE + 0x41)  /* disable axp irq of ar200                         */
#define AXP_ENABLE_IRQ              (MESSAGE_BASE + 0x42)  /* enable axp irq of ar200                          */
#define AXP_GET_CHIP_ID             (MESSAGE_BASE + 0x43)  /* axp get chip id                                  */
#define AXP_SET_PARAS               (MESSAGE_BASE + 0x44)  /* config axp parameters (ac327 to arisc)           */
#define SET_PMU_VOLT                (MESSAGE_BASE + 0x45)  /* set pmu voltage (ac327 to arisc)                 */
#define GET_PMU_VOLT                (MESSAGE_BASE + 0x46)  /* get pmu voltage (ac327 to arisc)                 */
#define SET_LED_BLN                 (MESSAGE_BASE + 0x47)  /* set led bln (ac327 to arisc)                     */
#define SET_PWR_TREE                (MESSAGE_BASE + 0x49)  /* set power tree (ac327 to arisc)                  */
#define CLR_NMI_STATUS              (MESSAGE_BASE + 0x4a)  /* clear nmi status (ac327 to arisc)                */
#define SET_NMI_TRIGGER             (MESSAGE_BASE + 0x4b)  /* set nmi tigger (ac327 to arisc)                  */
#define SET_PMU_VOLT_STA            (MESSAGE_BASE + 0x4c)  /* set pmu voltage state(ac327 to arisc)            */
#define GET_PMU_VOLT_STA            (MESSAGE_BASE + 0x4d)  /* get pmu voltage state(ac327 to arisc)            */

//set debug level commands
#define SET_DEBUG_LEVEL_REQ         (MESSAGE_BASE + 0x50)  /* set debug level   (ac327 to ar100)               */
#define MESSAGE_LOOPBACK            (MESSAGE_BASE + 0x51)  /* loopback message  (ac327 to ar100)               */
#define SET_UART_BAUDRATE           (MESSAGE_BASE + 0x52)  /* set uart baudrate (ac327 to ar100)               */
#define SET_DRAM_PARAS              (MESSAGE_BASE + 0x53)  /* config dram parameter (ac327 to ar100)           */
#define SET_DRAM_CRC_PARAS          (MESSAGE_BASE + 0x54)  /* config dram crc parameters (ac327 to ar100)      */
#define SET_IR_PARAS                (MESSAGE_BASE + 0x55)  /* config ir parameter (ac327 to ar100)             */
#define SET_PARAS                   (MESSAGE_BASE + 0x57)  /* set paras (arisc to ac327)                       */

#if defined CONFIG_ARCH_SUN8IW1P1
/* p2wi commands */
#define P2WI_READ_BLOCK_DATA        (MESSAGE_BASE + 0x70)  /* p2wi read block data        (ac327 to arisc)     */
#define P2WI_WRITE_BLOCK_DATA       (MESSAGE_BASE + 0x71)  /* p2wi write block data       (ac327 to arisc)     */
#define P2WI_BITS_OPS_SYNC          (MESSAGE_BASE + 0x72)  /* p2wi clear bits sync        (ac327 to arisc)     */
#elif (defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
/* rsb commands */
#define RSB_READ_BLOCK_DATA         (MESSAGE_BASE + 0x70)  /* rsb read block data        (ac327 to arisc)      */
#define RSB_WRITE_BLOCK_DATA        (MESSAGE_BASE + 0x71)  /* rsb write block data       (ac327 to arisc)      */
#define RSB_BITS_OPS_SYNC           (MESSAGE_BASE + 0x72)  /* rsb clear bits sync        (ac327 to arisc)      */
#define RSB_SET_RTSADDR             (MESSAGE_BASE + 0x74)  /* rsb set runtime slave addr (ac327 to arisc)      */
#elif (defined CONFIG_ARCH_SUN50IW2P1)
/* twi commands */
#define TWI_READ_BLOCK_DATA        (MESSAGE_BASE + 0x70)   /* twi read block data        (ac327 to arisc)      */
#define TWI_WRITE_BLOCK_DATA       (MESSAGE_BASE + 0x71)   /* twi write block data       (ac327 to arisc)      */
#define TWI_BITS_OPS_SYNC          (MESSAGE_BASE + 0x72)   /* twi clear bits sync        (ac327 to arisc)      */
#endif

//ar100 initialize state notify commands
#define AR100_STARTUP_NOTIFY        (MESSAGE_BASE + 0x80)  //ar100 init state notify(ar100 to ac327)

//Sensor Hub communication commands
#define AP_READ_DATA          (MESSAGE_BASE + 0x90)  //ap read data
#define AP_WRITE_DATA         (MESSAGE_BASE + 0x91)  //ap write data
#define SH_READ_DATA          (MESSAGE_BASE + 0x92)  //sh read data
#define SH_WRITE_DATA         (MESSAGE_BASE + 0x93)  //sh write data

#endif  //__MESSAGES_H__
