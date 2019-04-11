/**
 * driver\hwmsgbox\hwmsgbox.c
 *
 * Descript: hardware message-box driver.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#include "hwmsgbox_i.h"

#if HW_MESSAGE_USED

static u32 hwmsg_suspend = 0;
volatile static u32 old_value = 1;
volatile static u32 new_value;

/*
*********************************************************************************************************
*                                           CLEAR PENDING
*
* Description:  clear the receiver interrupt pending of message-queue.
*
* Arguments  :  queue   : the number of message-queue which we want to clear.
*               user    : the user which we want to clear.
*
* Returns    :  OK if clear pending succeeded, others if failed.
*********************************************************************************************************
*/
#define hwmsgbox_clear_receiver_pending(queue, user)\
           writel((0x1 << (queue * 2)), MSGBOX_IRQ_STATUS_REG(user))

/* ar100 wait message method: wait private flag to set 1 */
#define hwmsgbox_message_feedback(pmessage)  do{(pmessage)->private = 1;}while(0)

/*
*********************************************************************************************************
*                                           INT HANDLER
*
* Description:  the interrupt handler for message-queue 1 receiver.
*
* Arguments  :  parg    : the argument of this handler.
*
* Returns    :  TRUE if handle interrupt succeeded, others if failed.
*********************************************************************************************************
*/
static s32 hwmsgbox_int_handler(void *parg)
{
	volatile u32 value;

	//INF("msgbox irq coming...\n");

	//process ar100 asyn received channel, process all received messages
	while (readl(MSGBOX_MSG_STATUS_REG(HWMSGBOX_AR100_ASYN_RX_CH))) {
		struct message *pmessage;

		value = readl(MSGBOX_MSG_REG(HWMSGBOX_AR100_ASYN_RX_CH));
		pmessage = message_map_to_cpus(value);
		if (message_valid(pmessage)) {
			//message state switch
			if (pmessage->state == MESSAGE_PROCESSED) {
				//MESSAGE_PROCESSED->MESSAGE_FEEDBACKED,
				//process feedback message.
				pmessage->state = MESSAGE_FEEDBACKED;
				hwmsgbox_message_feedback(pmessage);
			} else {
				//MESSAGE_INITIALIZED->MESSAGE_RECEIVED,
				//notify new message coming.
				pmessage->state = MESSAGE_RECEIVED;
				//INF("asyn\n");
				message_coming_notify(pmessage);
			}
		} else {
			ERR("inv msg[%p] rec\n", pmessage);
		}

	}

	//clear pending
	hwmsgbox_clear_receiver_pending(HWMSGBOX_AR100_ASYN_RX_CH, HWMSG_QUEUE_USER_ARISC);

	//process ac327 syn tx channel, process only one message
	if (readl(MSGBOX_MSG_STATUS_REG(HWMSGBOX_AC327_SYN_TX_CH))) {
		struct message *pmessage;
		value = readl(MSGBOX_MSG_REG(HWMSGBOX_AC327_SYN_TX_CH));
		pmessage = message_map_to_cpus(value);
		if (message_valid(pmessage)) {
			//message state switch
			if (pmessage->state == MESSAGE_PROCESSED) {
				//MESSAGE_PROCESSED->MESSAGE_FEEDBACKED,
				//process feedback message.
				pmessage->state = MESSAGE_FEEDBACKED;
				hwmsgbox_message_feedback(pmessage);
			} else {
				//MESSAGE_INITIALIZED->MESSAGE_RECEIVED,
				//notify new message coming.
				pmessage->state = MESSAGE_RECEIVED;
				//INF("syn\n");
				message_coming_notify(pmessage);
			}
		} else {
			ERR("inv msg[%p] rec\n", pmessage);
		}
	}

	//clear pending
	hwmsgbox_clear_receiver_pending(HWMSGBOX_AC327_SYN_TX_CH, HWMSG_QUEUE_USER_ARISC);

	//process AP to SH channel, process only one message
	while (readl(MSGBOX_MSG_STATUS_REG(HWMSGBOX_AP_WAKEUP_SH_CH))) {
		readl(MSGBOX_MSG_REG(HWMSGBOX_AP_WAKEUP_SH_CH));
		new_value = readl(RTC_AP_WAKEUP_SH_REG);
		if (new_value != old_value) {
			//ERR("%u begin\n", readl(RTC_AP_WAKEUP_SH_REG));
			platWakeupIsr((bool)new_value);
			//ERR("%u end\n", readl(RTC_AP_WAKEUP_SH_REG));
		}

		old_value = new_value;
		//clear pending
		hwmsgbox_clear_receiver_pending(HWMSGBOX_AP_WAKEUP_SH_CH, HWMSG_QUEUE_USER_ARISC);
	}

	return TRUE;
}

/*
*********************************************************************************************************
*                                           ENABLE RECEIVER INT
*
* Description:  enbale the receiver interrupt of message-queue.
*
* Arguments  :  queue   : the number of message-queue which we want to enable interrupt.
*               user    : the user which we want to enable interrupt.
*
* Returns    :  OK if enable interrupt succeeded, others if failed.
*********************************************************************************************************
*/
static s32 hwmsgbox_enable_receiver_int(u32 queue, u32 user)
{
	volatile u32 value;

	value  =  readl(MSGBOX_IRQ_EN_REG(user));
	value &= ~(0x1 << (queue * 2));
	value |=  (0x1 << (queue * 2));
	writel(value, MSGBOX_IRQ_EN_REG(user));

	return OK;
}

/*
*********************************************************************************************************
*                                           QUERY PENDING
*
* Description:  query the receiver interrupt pending of message-queue.
*
* Arguments  :  queue   : the number of message-queue which we want to query.
*               user    : the user which we want to query.
*
* Returns    :  OK if query pending succeeded, others if failed.
*********************************************************************************************************
*/
#if 0
static s32 hwmsgbox_query_receiver_pending(u32 queue, u32 user)
{
	volatile u32 value;

	value  =  readl(MSGBOX_IRQ_STATUS_REG(user));

	return value & (0x1 << (queue * 2));
}
#endif

/*
*********************************************************************************************************
*                                           SET RECEIVER
*
* Description:  set user as receiver of one message-queue.
*
* Arguments  :  queue   : the number of message-queue which we want to set.
*               user    : the user which we want to set.
*
* Returns    :  OK if set user succeeded, others if failed.
*********************************************************************************************************
*/
static s32 hwmsgbox_set_receiver(u32 queue, u32 user)
{
	volatile u32 value;

	value  =  readl(MSGBOX_CTRL_REG(queue));
	value &= ~(1    << (0 + ((queue & 0x3)<<3)));
	value |=  (user << (0 + ((queue & 0x3)<<3)));
	writel(value, MSGBOX_CTRL_REG(queue));

	return OK;
}

/*
*********************************************************************************************************
*                                           SET TRANSMITTER
*
* Description:  set user as transmitter of one message-queue.
*
* Arguments  :  queue   : the number of message-queue which we want to set.
*               user    : the user which we want to set.
*
* Returns    :  OK if set user succeeded, others if failed.
*********************************************************************************************************
*/
static s32 hwmsgbox_set_transmitter(u32 queue, u32 user)
{
	volatile u32 value;

	value  =  readl(MSGBOX_CTRL_REG(queue));
	value &= ~(1    << (4 + ((queue & 0x3)<<3)));
	value |=  (user << (4 + ((queue & 0x3)<<3)));
	writel(value, MSGBOX_CTRL_REG(queue));

	return OK;
}

static s32 hwmsgbox_wait_message_feedback(struct message *pmessage)
{
	//ar100 wait message method: wait private flag to set 1.
	while (pmessage->private == 0)
		; //do nothing

	//INF("message : %p finished\n", pmessage);
	return OK;
}

/*
*********************************************************************************************************
*                                           INITIALIZE HWMSGBOX
*
* Description:  initialize hwmsgbox.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_init(void)
{
	//enable msgbox clock and set reset as de-assert state.
	ccu_set_mclk_reset(CCU_MOD_CLK_MSGBOX, CCU_CLK_NRESET);
	ccu_set_mclk_onoff(CCU_MOD_CLK_MSGBOX, CCU_CLK_ON);

	//ar100 asyn tx channel configure
	//ar100 set as transmitter,
	//ac327 set as receiver.
	hwmsgbox_set_transmitter(HWMSGBOX_AR100_ASYN_TX_CH, HWMSG_QUEUE_USER_ARISC);
	hwmsgbox_set_receiver   (HWMSGBOX_AR100_ASYN_TX_CH, HWMSG_QUEUE_USER_AC327);

	//ar100 asyn rx channel configure
	//ar100 set as receiver,
	//ac327 set as transmitter.
	hwmsgbox_set_transmitter(HWMSGBOX_AR100_ASYN_RX_CH, HWMSG_QUEUE_USER_AC327);
	hwmsgbox_set_receiver   (HWMSGBOX_AR100_ASYN_RX_CH, HWMSG_QUEUE_USER_ARISC);

	//ar100 syn tx channel configure
	//ar100 set as transmitter,
	//ac327 set as receiver.
	hwmsgbox_set_transmitter(HWMSGBOX_AR100_SYN_TX_CH, HWMSG_QUEUE_USER_ARISC);
	hwmsgbox_set_receiver   (HWMSGBOX_AR100_SYN_TX_CH, HWMSG_QUEUE_USER_AC327);

	//ar100 syn rx channel configure
	//ar100 set as receiver,
	//ac327 set as transmitter.
	hwmsgbox_set_transmitter(HWMSGBOX_AR100_SYN_RX_CH, HWMSG_QUEUE_USER_AC327);
	hwmsgbox_set_receiver   (HWMSGBOX_AR100_SYN_RX_CH, HWMSG_QUEUE_USER_ARISC);

	//ac327 syn tx channel configure
	//ar100 set as receiver,
	//ac327 set as transmitter.
	hwmsgbox_set_transmitter(HWMSGBOX_AC327_SYN_TX_CH, HWMSG_QUEUE_USER_AC327);
	hwmsgbox_set_receiver   (HWMSGBOX_AC327_SYN_TX_CH, HWMSG_QUEUE_USER_ARISC);

	//ac327 syn rx channel configure
	//ar100 set as transmitter,
	//ac327 set as receiver.
	hwmsgbox_set_transmitter(HWMSGBOX_AC327_SYN_RX_CH, HWMSG_QUEUE_USER_ARISC);
	hwmsgbox_set_receiver   (HWMSGBOX_AC327_SYN_RX_CH, HWMSG_QUEUE_USER_AC327);

#if 1
	//SH to AP channel configure
	//SH set as transmitter,
	//AP set as receiver.
	hwmsgbox_set_transmitter(HWMSGBOX_SH_WAKEUP_AP_CH, HWMSG_QUEUE_USER_ARISC);
	hwmsgbox_set_receiver   (HWMSGBOX_SH_WAKEUP_AP_CH, HWMSG_QUEUE_USER_AC327);

	//AP to SH channel configure
	//SH set as receiver,
	//AP set as transmitter.
	hwmsgbox_set_transmitter(HWMSGBOX_AP_WAKEUP_SH_CH, HWMSG_QUEUE_USER_AC327);
	hwmsgbox_set_receiver   (HWMSGBOX_AP_WAKEUP_SH_CH, HWMSG_QUEUE_USER_ARISC);
#endif

	//register message-box interrupt handler.
	if (!hwmsg_suspend) {
		install_isr(MBOX_IRQn, hwmsgbox_int_handler, NULL);
		interrupt_enable(MBOX_IRQn);
	}
	//enable ar100 asyn rx channel interrupt.
	hwmsgbox_enable_receiver_int(HWMSGBOX_AR100_ASYN_RX_CH, HWMSG_QUEUE_USER_ARISC);

	//enable ac327 syn tx channel interrupt.
	hwmsgbox_enable_receiver_int(HWMSGBOX_AC327_SYN_TX_CH, HWMSG_QUEUE_USER_ARISC);

	//enable AP to SH channel interrupt.
	hwmsgbox_enable_receiver_int(HWMSGBOX_AP_WAKEUP_SH_CH, HWMSG_QUEUE_USER_ARISC);

	//hwmsgbox_enable_receiver_int(HWMSGBOX_AR100_ASYN_TX_CH, HWMSG_QUEUE_USER_AC327);

	return OK;
}

/*
*********************************************************************************************************
*                                           EXIT HWMSGBOX
*
* Description:  exit hwmsgbox.
*
* Arguments  :  none.
*
* Returns    :  OK if exit hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_exit(void)
{
	//disable msgbox clock and set reset as assert state.
	ccu_set_mclk_reset(CCU_MOD_CLK_MSGBOX, CCU_CLK_RESET);
	ccu_set_mclk_onoff(CCU_MOD_CLK_MSGBOX, CCU_CLK_OFF);

	return OK;
}

/*
*********************************************************************************************************
*                                       SEND MESSAGE BY HWMSGBOX
*
* Description:  send one message to another processor by hwmsgbox.
*
* Arguments  :  pmessage    : the pointer of sended message frame.
*               timeout     : the wait time limit when message fifo is full.
*
* Returns    :  OK if send message succeeded, other if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_send_message(struct message *pmessage, u32 timeout)
{
	ASSERT(pmessage != NULL);

	if (pmessage->attr & MESSAGE_ATTR_HARDSYN) {
		volatile u32 value;
		//use ar100 hwsyn transmit channel.
		while (readl(MSGBOX_FIFO_STATUS_REG(HWMSGBOX_AR100_SYN_TX_CH)) == 1) {
			//message-queue fifo is full,
			//wait 1ms for message-queue process.
			if (timeout == 0) {
				return -ETIMEOUT;
			}
			time_mdelay(1);
			timeout--;
		}
		//INF("send syn message : %p\n", pmessage);
		value = message_map_to_cpux(pmessage);
		writel(value, MSGBOX_MSG_REG(HWMSGBOX_AR100_SYN_TX_CH));

		//hwsyn messsage must feedback use syn rx channel
		while (readl(MSGBOX_MSG_STATUS_REG(HWMSGBOX_AR100_SYN_RX_CH)) == 0)
			; //message not valid
			//printk("-->%p:%x\n", (void *)MSGBOX_MSG_STATUS_REG(HWMSGBOX_AR100_SYN_RX_CH), readl(MSGBOX_MSG_STATUS_REG(HWMSGBOX_AR100_SYN_RX_CH)));

		//check message valid
		if (value != readl(MSGBOX_MSG_REG(HWMSGBOX_AR100_SYN_RX_CH))) {
			ERR("hsyn msg err\n");
			return -EFAULT;
		}
		//INF("hsyn msg[%p, %x] fb\n", pmessage, pmessage->type);

		return OK;
	}
	//asyn message use asyn tx channel
	while (readl(MSGBOX_FIFO_STATUS_REG(HWMSGBOX_AR100_ASYN_TX_CH)) == 1) {
		//message-queue fifo is full,
		//wait 1ms for message-queue process.
		if (timeout == 0) {
			ERR("send fifo full\n");
			return -ETIMEOUT;
		}
		time_mdelay(1);
		timeout--;
	}
	//write message to message-queue fifo.
	INF("send asyn or soft syn message : %p\n", pmessage);
	writel(message_map_to_cpux(pmessage), MSGBOX_MSG_REG(HWMSGBOX_AR100_ASYN_TX_CH));

	//syn messsage must wait message feedback.
	if (pmessage->attr & MESSAGE_ATTR_SOFTSYN) {
		hwmsgbox_wait_message_feedback(pmessage);
	}
	return OK;
}

int hwmsgbox_feedback_message(struct message *pmessage, u32 timeout)
{
	ASSERT(pmessage != NULL);

	if (pmessage->attr & MESSAGE_ATTR_HARDSYN) {
		//use ac327 hard syn receiver channel.
		while (readl(MSGBOX_FIFO_STATUS_REG(HWMSGBOX_AC327_SYN_RX_CH)) == 1) {
			//message-queue fifo is full,
			//wait 1ms for message-queue process.
			time_mdelay(1);
			if (timeout == 0) {
				return -ETIMEOUT;
			}
			timeout--;
		}
		//INF("feedback hard syn message : %p\n", pmessage);
		writel(message_map_to_cpux(pmessage), MSGBOX_MSG_REG(HWMSGBOX_AC327_SYN_RX_CH));
		return OK;
	}
	//soft syn use asyn tx channel
	if ((pmessage->attr & MESSAGE_ATTR_SOFTSYN) || (pmessage->attr == 0)) {
		while (readl(MSGBOX_FIFO_STATUS_REG(HWMSGBOX_AR100_ASYN_TX_CH)) == 1) {
			//message-queue fifo is full,
			//wait 1ms for message-queue process.
			if (timeout == 0) {
				return -ETIMEOUT;
			}
			time_mdelay(1);
			timeout--;
		}
		//write message to message-queue fifo.
		//INF("send asyn or soft syn message : %p\n", pmessage);
		writel(message_map_to_cpux(pmessage), MSGBOX_MSG_REG(HWMSGBOX_AR100_ASYN_TX_CH));
		return OK;
	}

	//invalid syn message
	return -ESRCH;
}

/*
*********************************************************************************************************
*                                        QUERY MESSAGE
*
* Description:  query message of hwmsgbox syn channel by hand, mainly for.
*
* Arguments  :  none.
*
* Returns    :  the point of message, NULL if timeout.
*********************************************************************************************************
*/
struct message *hwmsgbox_query_message(void)
{
	u32 value;

	struct message *pmessage = NULL;

	//query ar100 asyn received channel
	if (readl(MSGBOX_MSG_STATUS_REG(HWMSGBOX_AR100_ASYN_RX_CH))) {
		value = readl(MSGBOX_MSG_REG(HWMSGBOX_AR100_ASYN_RX_CH));
		pmessage = message_map_to_cpus(value);
		if (message_valid(pmessage)) {
			//message state switch
			if (pmessage->state == MESSAGE_PROCESSED)
				pmessage->state = MESSAGE_FEEDBACKED; //MESSAGE_PROCESSED->MESSAGE_FEEDBACKED
			else
				pmessage->state = MESSAGE_RECEIVED; //MESSAGE_INITIALIZED->MESSAGE_RECEIVED
		} else {
			ERR("inv msg rec\n");
			return NULL;
		}
		//clear pending
		hwmsgbox_clear_receiver_pending(HWMSGBOX_AR100_ASYN_RX_CH, HWMSG_QUEUE_USER_ARISC);

		return pmessage;
	}

	//query ar100 syn received channel
	if (readl(MSGBOX_MSG_STATUS_REG(HWMSGBOX_AC327_SYN_TX_CH))) {
		value = readl(MSGBOX_MSG_REG(HWMSGBOX_AC327_SYN_TX_CH));
		pmessage = message_map_to_cpus(value);
		if (message_valid(pmessage)) {
			//message state switch
			if (pmessage->state == MESSAGE_PROCESSED)
				pmessage->state = MESSAGE_FEEDBACKED; //MESSAGE_PROCESSED->MESSAGE_FEEDBACKED
			else
				pmessage->state = MESSAGE_RECEIVED; //MESSAGE_INITIALIZED->MESSAGE_RECEIVED
		} else {
			ERR("inv msg rec\n");
			return NULL;
		}
		//clear pending
		hwmsgbox_clear_receiver_pending(HWMSGBOX_AC327_SYN_TX_CH, HWMSG_QUEUE_USER_ARISC);
		LOG("query syn msg\n");

		return pmessage;
	}
	//no valid message
	return NULL;
}

s32 hwmsgbox_super_standby_init(void)
{
	hwmsg_suspend = 1;

	hwmsgbox_exit();

	return OK;
}

s32 hwmsgbox_super_standby_exit(void)
{
	hwmsgbox_init();

	hwmsg_suspend = 0;

	return OK;
}

void hwmsgbox_sh_wakeup_ap_set(void)
{
	if (readl(RTC_SH_WAKEUP_AP_REG) == 1) {
		writel(0, RTC_SH_WAKEUP_AP_REG);
		writel(0xfe, MSGBOX_MSG_REG(HWMSGBOX_SH_WAKEUP_AP_CH));
		//LOG("set\n");
	}
}

void hwmsgbox_sh_wakeup_ap_clear(void)
{
	//hwmsgbox_clear_receiver_pending(HWMSGBOX_SH_WAKEUP_AP_CH, HWMSG_QUEUE_USER_AC327);
	writel(1, RTC_SH_WAKEUP_AP_REG);
	//LOG("clear:%x\n", readl(RTC_SH_WAKEUP_AP_REG));
}

#endif /* HW_MESSAGE_USED */
