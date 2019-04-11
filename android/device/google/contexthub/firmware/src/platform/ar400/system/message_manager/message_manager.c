/**
 * system/message_manager/message_manager.c
 *
 * Descript: message manager module
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#include "message_manager_i.h"
#include "../../prcm/ccu_i-sun50iw3.h"

#if HW_MESSAGE_USED
/* cache for free messages */
volatile message_cache_t message_cache;

/* the start and end of message pool */
static struct message *message_start;
static struct message *message_end;

static u32 message_pool_base;
static u32 message_pool_size;

static u32 section_in_sram =0x0;
extern char __load_start_text0,__load_stop_text0;
extern char __load_start_text1,__load_stop_text1;
extern char __load_start_text2,__load_stop_text2;
extern char __load_start_text3,__load_stop_text3;
extern char __load_start_text4,__load_stop_text4;
extern u32 ar100_code_addr;
extern char textx_vma;
#define TEXTX_VMA  (char  *)(&textx_vma)

int message_valid(struct message *pmessage)
{
	/* valid message */
	if ((pmessage >= message_start) && (pmessage <  message_end))
		return 1;

	/* invalid message */
	return 0;
}

//#define MSG_BUF_MON

#ifdef MSG_BUF_MON
void message_buf_monitor(void)
{
	u32 *p = (u32 *)((u32)message_end - sizeof(struct message));

	while (!(*p--))
		;

	LOG("msg free:%d byte\n", (u32)((u32 *)message_end - p));
}
#endif

/**
 * message_manager_init() - initialize message manager.
 *
 * @return: OK if initialize succeeded, others if failed.
 */
s32 message_manager_init(void)
{
	u32 i;

	//get message pool information from arisc parameters
	arisc_para_get_message_pool_info(&message_pool_base, &message_pool_size);
	//message_pool_base = MESSAGE_POOL_START;
	//message_end = MESSAGE_POOL_END;
	//message_pool_size = MESSAGE_POOL_END - MESSAGE_POOL_START;

	LOG("%s: message pool addr=%x, size=%x\n", __func__, message_pool_base, message_pool_size);

	//initialize message pool start and end
	message_start = (struct message *)(message_pool_base);
	message_end   = (struct message *)(message_pool_base + message_pool_size);

	//clear message pool as initialize status
	memset((void *)message_start, 0, message_pool_size);

	//initialize free list
	for (i = 0; i < MESSAGE_CACHED_MAX;i++)
		message_cache.cache[i] = NULL;
	message_cache.number = 0;

	section_in_sram = SECTION_NONE;
#ifdef MSG_BUF_MON
	daemon_register_service((__pNotifier_t)message_buf_monitor);
#endif
	return OK;
}

/**
 * message_manager_exit() - exit message manager.
 *
 * @return: OK if exit succeeded, others if failed.
 */
s32 message_manager_exit(void)
{
	return OK;
}

/**
 * message_allocate() - allocate one message frame. mainly use for send message
 *                      by message-box, the message frame allocate form messages
 *                      pool shared memory area.
 *
 * @return: the pointer of allocated message frame, NULL if failed;
 */
struct message *message_allocate(void)
{
	s32 cpsr;
	struct message *pmessage = NULL;
	struct message *palloc   = NULL;

	/* first find in free_list */
	cpsr = cpuIntsOff();
	if (message_cache.number) {
		/* free_list have cached message, allocate the head node */
		message_cache.number--;
		palloc = message_cache.cache[message_cache.number];
		message_cache.cache[message_cache.number] = NULL;
		//INF("message [%p] alloc from cache\n", palloc);
		//INF("message_cache num:%x\n", message_cache.number + 1);
	}
	cpuIntsRestore(cpsr);
	if (!message_valid(palloc)) {
		/* find in message_cache fail, allocate from message pool,
		 * use spinlock 0 to exclusive with ac327.
		 */
		hwspin_lock_timeout(MSG_HWSPINLOCK, SPINLOCK_TIMEOUT);

		/* seach from the start of message pool every time */
		pmessage = message_start;
		while (pmessage < message_end) {
			if (pmessage->state == MESSAGE_FREED) {
				/* allocate free message in message pool */
				palloc = pmessage;
				palloc->state = MESSAGE_ALLOCATED;
				//INF("message alloc from pool\n");
				break;
			}
			/* next message frame */
			pmessage++;
		}
		/* unlock hwspinlock */
		hwspin_unlock(MSG_HWSPINLOCK);
	}
	if (!message_valid(palloc))
		ERR("ami,add:%p\n", palloc);

	/* initialize message frame */
	palloc->next   = 0;
	palloc->private= 0;
	palloc->attr   = 0;

	return palloc;
}

/**
 * message_free() - free one message frame. mainly use for process message
 *                  finished, free it to messages pool or add to free message
 *                  queue.
 *
 * pmessage: the pointer of free message frame.
 * @return: none.
 */
void message_free(struct message *pmessage)
{
	s32 cpsr;

	if (!message_valid(pmessage)) {
		WRN("free invalid message\n");
		return;
	}
	/* try to add message_cache first */
	cpsr = cpuIntsOff();
	if (message_cache.number < MESSAGE_CACHED_MAX) {
		/* cached this message, message state: ALLOCATED */
		//INF("message [%p] insert to message_cache\n", pmessage);
		//INF("message_cache number:%x\n", message_cache.number);
#if (defined TF_USED)
		pmessage->next = 0;
#else
		pmessage->next = NULL;
#endif
		pmessage->state = MESSAGE_ALLOCATED;
		message_cache.cache[message_cache.number] = pmessage;
		message_cache.number++;
	} else {
		/* free to message pool, set message state as FREED */
		hwspin_lock_timeout(MSG_HWSPINLOCK, SPINLOCK_TIMEOUT);
		pmessage->state = MESSAGE_FREED;
#if (defined TF_USED)
		pmessage->next = 0;
#else
		pmessage->next = NULL;
#endif
		hwspin_unlock(MSG_HWSPINLOCK);
	}
	cpuIntsRestore(cpsr);
}

/**
 * process_message() - process one message.
 *
 * pmessage: the pointer of message frame which we want to process.
 * @return: OK if process message succeeded, other if failed.
 */
/* use main REQM here, and used sub REQS in msg_handler will be better,
 * because message_manager.c is a system(kernel space) file, it is should NOT
 * modified every time when message(user space) changed!
 */
static s32 process_message(struct message *pmessage)
{
	s32 result;

	/*message per-process, message state : PROCESSING.*/
	pmessage->state = MESSAGE_PROCESSING;

	/*process message*/
	switch (pmessage->type) {

	case AP_WRITE_DATA:
		//LOG("ap write data\n");
		result = ap_write_data((char *)(pmessage->paras[0]), pmessage->paras[1]);
		break;
#if CPUOP_USED
	case CPU_OP_REQ:
		INF("cpu op req\n");
		result = cpu_op(pmessage);
		break;
#endif

#if SYSOP_USED
	case SYS_OP_REQ:
		INF("sys op req\n");
		result = sys_op(pmessage);
		break;
#endif

#if DVFS_USED
	/*=============dvfs entry=============*/
	case CPUX_DVFS_REQ:
		INF("cpux dvfs request\n");
		result = dvfs_set_freq(pmessage);
		break;
#if (defined CONFIG_ARCH_SUN8IW1P1) || \
	(defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW7P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1)
	case CPUX_DVFS_CFG_VF_REQ:
		INF("cpux dvfs vf-table config request\n");
		result = dvfs_config_vf_table(pmessage);
		break;
#endif
#endif
#if SST_USED
		/*=============standby entry=============*/
		case SSTANDBY_ENTER_REQ:
			INF("super-standby enter request\n");
			if (section_in_sram != SECTION_SSTANDBY) {
				section_in_sram = SECTION_SSTANDBY;
				memcpy((void *)TEXTX_VMA, (void *)(ar100_code_addr+(u32)&__load_start_text0),
							   (u32)&__load_stop_text0-(u32)&__load_start_text0);
				icache_coherent();
				LOG("copy sstandby code\n");
				time_udelay(1000);
			}
			save_state_flag(REC_SSTANDBY | REC_COPY_DONE);//RECORD
			result = super_standby_entry(pmessage);
			break;
#endif
#if NST_USED
		case NSTANDBY_ENTER_REQ:
			INF("normal-standby enter request\n");
			if (section_in_sram != SECTION_NSTANDBY) {
				section_in_sram = SECTION_NSTANDBY;
				memcpy((void *)TEXTX_VMA, (void *)(ar100_code_addr+(u32)&__load_start_text1),
							   (u32)&__load_stop_text1-(u32)&__load_start_text1);
				icache_coherent();
				LOG("copy nstandby code\n");
				time_udelay(1000);
			}
			save_state_flag(REC_NSTANDBY | REC_COPY_DONE);//RECORD
			result = normal_standby_entry(pmessage);
			break;
#endif
#if EST_USED
		case ESSTANDBY_ENTER_REQ:
			INF("esstandby enter request\n");
			save_state_flag(REC_ESTANDBY | REC_COPY_DONE);//RECORD
			result = extended_super_standby_entry(pmessage);
			break;
#endif
#if TST_USED
		case TSTANDBY_ENTER_REQ:
			INF("talk-standby enter request\n");
			if (section_in_sram != SECTION_TSTANDBY) {
				section_in_sram = SECTION_TSTANDBY;
				memcpy((void *)TEXTX_VMA, (void *)(ar100_code_addr+(u32)&__load_start_text4),
							   (u32)&__load_stop_text4-(u32)&__load_start_text4);
				icache_coherent();
				LOG("copy tstandby code\n");
				time_udelay(1000);
			}
			save_state_flag(REC_TSTANDBY | REC_COPY_DONE);//RECORD
			result = talk_standby_entry(pmessage);
			break;
#endif
#if FAKE_POWOFF_USED
#if (defined CONFIG_ARCH_SUN8IW1P1) || \
	(defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW7P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1)
		case FAKE_POWER_OFF_REQ:
			INF("fake power off enter request\n");
			if (section_in_sram != SECTION_FAKEPOWEROFF) {
				section_in_sram = SECTION_FAKEPOWEROFF;
				memcpy((void *)TEXTX_VMA, (void *)(ar100_code_addr+(u32)&__load_start_text3),
							   (u32)&__load_stop_text3-(u32)&__load_start_text3);
				icache_coherent();
				LOG("copy fakepoweroff code\n");
				time_udelay(1000);

			}
			save_state_flag(REC_FAKEPOWEROFF | REC_COPY_DONE);//RECORD
			result = fake_power_off_entry(pmessage);
			break;
#else
		case FAKE_POWER_OFF_REQ:
			INF("fake power off enter request\n");
			result = fake_power_off_entry(pmessage);
			break;
#endif // CONFIG_ARCH_SUN50IW1P1
#endif // FAKE_POWOFF_USED
#if (defined CONFIG_ARCH_SUN8IW1P1) || \
	(defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW7P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1)
		case SET_DRAM_PARAS:
		        INF("config dram paras request\n");
		        result = dram_config_paras(pmessage->paras[0], pmessage->paras[1], &(pmessage->paras[2]));
		        break;
#endif

#if LED_BLN_USED
		case SET_LED_BLN:
			INF("cpu idle enter\n");
			result = led_bln_cfg(pmessage->paras[0], pmessage->paras[1], \
			                     pmessage->paras[2], pmessage->paras[3]);
			break;
#endif
#if  IR_USED
#if (defined KERNEL_USED)
		case SET_IR_PARAS:
			INF("set ir paras request\n");
			result = ir_set_paras(pmessage);
			break;
#endif
#endif
#if STANDBY_USED
#if (defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
		case STANDBY_INFO_REQ:
			INF("standby info request\n");
			result = sstandby_query_info(pmessage);
			break;
#endif
#endif
		/*=============pmu entry=============*/
		case AXP_DISABLE_IRQ:
			INF("disable axp irq request\n");
			//result = interrupt_disable(EXT_NMI_IRQn);
			ccu_reg_addr->nmi_int_en.nmi_irq_en = 0;
			result = OK;
			break;
		case AXP_ENABLE_IRQ:
			INF("enable axp irq request\n");
			//interrupt_clear_pending(EXT_NMI_IRQn);
			//result = interrupt_enable(EXT_NMI_IRQn);
			ccu_reg_addr->nmi_int_st.nmi_irq_pend = 1;
			ccu_reg_addr->nmi_int_en.nmi_irq_en = 1;
			result = OK;
			break;
		case CLR_NMI_STATUS:
			INF("clear nmi status request\n");
			result = interrupt_clear_pending(EXT_NMI_IRQn);
			break;
		case SET_NMI_TRIGGER:
			LOG("set nmi trigger request\n");
			result = interrupt_set_nmi_trigger(pmessage->paras[0]);
			break;
		case AXP_GET_CHIP_ID:
			INF("axp get chip id request\n");
			result = pmu_get_chip_id(pmessage);
			break;
		case SET_PMU_VOLT:
			INF("set pmu volt request\n");
			result = pmu_set_voltage(pmessage->paras[0], pmessage->paras[1]);
			break;
		case GET_PMU_VOLT:
			INF("get pmu volt request\n");
			result = pmu_get_voltage(pmessage->paras[0]);
			if (result >= 0) {
				pmessage->paras[1] = result;
				result = OK;
			}
			break;
		case SET_PMU_VOLT_STA:
			INF("set pmu volt state request\n");
			result = pmu_set_voltage_state(pmessage->paras[0], pmessage->paras[1]);
			break;
		case GET_PMU_VOLT_STA:
			INF("get pmu volt state request\n");
			result = pmu_get_voltage_state(pmessage->paras[0]);
			if (result >= 0) {
				pmessage->paras[1] = result;
				result = OK;
			}
			break;
#if (defined CONFIG_ARCH_SUN8IW1P1) || \
	(defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
		case AXP_SET_PARAS:
			INF("pmu set paras request\n");
			result = pmu_set_paras(pmessage);
			break;
#endif
#if SET_PWR_TREE_USED
		case SET_PWR_TREE:
			INF("set power tree request\n");
			result = set_pwr_tree(pmessage);
			break;
#endif

		/*=============p2wi/rsb entry=============*/
#if P2WI_USED
		case P2WI_READ_BLOCK_DATA:
			INF("read pmu register request\n");
			result = p2wi_read_block_data(pmessage);
			break;
		case P2WI_WRITE_BLOCK_DATA:
			INF("write pmu register request\n");
			result = p2wi_write_block_data(pmessage);
			break;
		case P2WI_BITS_OPS_SYNC:
			INF("p2wi bits ops request\n");
			result = p2wi_bits_ops_sync(pmessage);
			break;
#elif RSB_USED
		case RSB_READ_BLOCK_DATA:
			INF("read pmu register request\n");
			result = rsb_read_block_data(pmessage);
			break;
		case RSB_WRITE_BLOCK_DATA:
			INF("write pmu register request\n");
			result = rsb_write_block_data(pmessage);
			break;
		case RSB_BITS_OPS_SYNC:
			INF("rsb bits ops request\n");
			result = rsb_bits_ops_sync(pmessage);
			break;
		case RSB_SET_RTSADDR:
			INF("rsb set rts addr request\n");
			result = rsb_set_run_time_addr((u32)pmessage->paras[0], (u32)pmessage->paras[1]);
			break;
#elif TWI_USED
		case TWI_READ_BLOCK_DATA:
			INF("read pmu register request\n");
			result = twi_read_block_data(pmessage);
			break;
		case TWI_WRITE_BLOCK_DATA:
			INF("write pmu register request\n");
			result = twi_write_block_data(pmessage);
			break;
		case TWI_BITS_OPS_SYNC:
			INF("rsb bits ops request\n");
			result = twi_bits_ops_sync(pmessage);
			break;
#endif

		/*=============debug entry=============*/
		case SET_DEBUG_LEVEL_REQ:
			INF("set debug level request\n");
			//result = set_debug_level(pmessage->paras[0]);
			result = FAIL;
			break;
		case SET_UART_BAUDRATE:
			INF("set uart baudrate request\n");
			result = uart_set_baudrate(pmessage->paras[0]);
			break;
#if STANDBY_USED
		case SET_DRAM_CRC_PARAS:
			INF("set dram crc paras request\n");
			result = standby_set_dram_crc_paras(pmessage->paras[0], pmessage->paras[1], pmessage->paras[2]);
			break;
#endif
		case MESSAGE_LOOPBACK:
			//just loopback this message
			INF("loopback message request\n");
			result = OK;
			break;
		case SET_PARAS:
			INF("set paras request\n");
			set_paras();
			result = OK;
			break;
		default :
			ERR("imt [%x]\n", pmessage->type);
			hexdump("msg", (char *)pmessage, sizeof(struct message));
			result = -ESRCH;
			break;
	}

	/* message post-process, message state : PROCESSED */
	pmessage->state = MESSAGE_PROCESSED;
	pmessage->result = result;
	/* synchronous message, should feedback process result */
	if (pmessage->attr & (MESSAGE_ATTR_SOFTSYN | MESSAGE_ATTR_HARDSYN))
		hwmsgbox_feedback_message(pmessage, SEND_MSG_TIMEOUT);

	/* asyn message:
	 * if no error, no need feedback message result, free message directly
	 * by ar100. if error, feedback message result, free message by ac327
	 */
	if (pmessage->attr == 0) {
		if (result != OK)
			hwmsgbox_feedback_message(pmessage, SEND_MSG_TIMEOUT);
		else
			message_free(pmessage);
	}

	return OK;
}

/**
 * message_coming_notify() - notify system that one message coming.
 *
 * pmessage: the pointer of coming message frame.
 * @return: OK if notify succeeded, other if failed.
 */
s32 message_coming_notify(struct message *pmessage)
{
	s32 cpsr;

	/* ar100 receive message from ac327 */
	//INF("MESSAGE FROM AC327\n");
	//INF("addr:%p\n", pmessage);
	//LOG("type:%x\n", pmessage->type);
	//INF("attr:%x\n", pmessage->attr);

	/* process this message directly, by sunny at 2012-12-12 18:50:17 */
	cpsr = cpuIntsOff();
	if (process_message(pmessage) != OK)
		WRN("message [%p, %x] process fail\n", pmessage, pmessage->type);
	cpuIntsRestore(cpsr);

	return OK;
}

struct message *message_map_to_cpus(u32 addr)
{
	struct message *message;

	message = (struct message *)(addr + message_pool_base);

	return message;
}

u32 message_map_to_cpux(struct message *message)
{
	u32 value = (u32)message - message_pool_base;

	return value;
}

#endif /* HW_MESSAGE_USED */
