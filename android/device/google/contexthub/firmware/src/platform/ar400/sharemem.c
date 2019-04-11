/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <gpio.h>
#include <sharemem.h>
#include <seos.h>
#include <util.h>
#include <gpio.h>
#include <atomicBitset.h>
#include <atomic.h>
#include <platform.h>

#include <plat/inc/cmsis.h>
#include <plat/inc/dma.h>
#include <plat/inc/gpio.h>
#include <plat/inc/pwr.h>
#include <plat/inc/plat.h>
#include <plat/inc/include.h>

#include <cpu/inc/barrier.h>

#define SM_VERBOSE_DEBUG       0
#define DEBUG_SHAREMEM 0

#if SM_VERBOSE_DEBUG
#define sm_log_debug(x) osLog(LOG_DEBUG, x "\n")
#else
#define sm_log_debug(x) do {} while(0)
#endif

struct SharememState {
    struct {
        union {
            uint8_t *buf;
            const uint8_t *cbuf;
            uint8_t byte;
        };
        size_t size;
        bool preamble;

        SharememCallbackF callback;
        void *cookie;
    } rx, tx;

    enum {
        AR_SM_IDLE,
        AR_SM_RX_ARMED,
        AR_SM_RX,
        AR_SM_TX_ARMED,
        AR_SM_SLAVE_TX,
    } state;

    uint16_t tid;
};

static struct SharememState state;

static inline void InvokeRxCallback(size_t tx, size_t rx, int err)
{
    uint16_t oldTid = osSetCurrentTid(state.tid);
    state.rx.callback(state.rx.cookie, tx, rx, err);
    osSetCurrentTid(oldTid);
}

static inline void InvokeTxCallback(size_t tx, size_t rx, int err)
{
    uint16_t oldTid = osSetCurrentTid(state.tid);
    state.tx.callback(state.tx.cookie, tx, rx, err);
    osSetCurrentTid(oldTid);
}

static inline void SharememRxDone(void)
{
    InvokeRxCallback(0, state.rx.size, 0);
}

static void SharememStopRxed(void)
{
    sm_log_debug("stopf");

    SharememRxDone();
}

int ap_write_data(char *data, uint32_t length)
{
#if DEBUG_SHAREMEM
    LOG("%p,%lu,%p,%u\n", data, length, state.rx.buf, state.rx.size);

    {
	uint8_t *ptr = (uint8_t *)data;
		osLog(LOG_ERROR,
		    "ap write: %02x [%02x %02x %02x %02x] [%02x %02x %02x %02x] [%02x] [%02x %02x %02x %02x\n",
		    ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6],
		    ptr[7], ptr[8], ptr[9], ptr[10], ptr[11], ptr[12],
		    ptr[13]);
	}
#endif

    memcpy(state.rx.buf, data, length);
    state.rx.size = length;

    SharememStopRxed();

    return 0;
}

int SharememRequest(uint32_t busId)
{
    return 0;
}

int SharememRelease(uint32_t busId)
{
    return 0;
}

void SharememEnableRx(uint32_t busId, void *rxBuf, size_t rxSize,
        SharememCallbackF callback, void *cookie)
{
	//osLog(LOG_ERROR, "%s-%u:len:%u\n", __func__, __LINE__, rxSize);
        state.rx.buf = rxBuf;
        state.rx.size = rxSize;
        state.rx.callback = callback;
        state.rx.cookie = cookie;
        state.state = AR_SM_RX_ARMED;
        state.tid = osGetCurrentTid();
}

static int SharememTx(uint32_t busId, const void *txBuf, uint8_t byte,
        size_t txSize, SharememCallbackF callback, void *cookie)
{
	struct message *pmessage;

	//osLog(LOG_ERROR, "%s-%u:len:%u\n", __func__, __LINE__, txSize);

        if (txBuf) {
            state.tx.cbuf = txBuf;
            state.tx.preamble = false;
        } else {
            state.tx.byte = byte;
            state.tx.preamble = true;
        }
        state.tx.size = txSize;
        state.tx.callback = callback;
        state.tx.cookie = cookie;

	pmessage = message_allocate();
	if (pmessage == NULL)
	{
		WRN("allocate message for nmi int notify failed\n");
		return FALSE;
	}

	//initialize message
	pmessage->type     = SH_WRITE_DATA;
	pmessage->private  = 0;
	pmessage->attr     = MESSAGE_ATTR_SOFTSYN;
	pmessage->state    = MESSAGE_INITIALIZED;

	if (txBuf)
		pmessage->paras[0] = (uint32_t)txBuf;
	else
		pmessage->paras[0] = (uint32_t)(&byte);
	pmessage->paras[1] = (uint32_t)txSize;
#if DEBUG_SHAREMEM
	WRN("%s-%u:0x%x,%u\n", __func__, __LINE__, pmessage->paras[0], pmessage->paras[1]);
	{
		uint8_t *ptr = (uint8_t *)txBuf;
		WRN(
		    "tx: %x [%x %x %x %x] [%x %x %x %x] [%x] [%x %x %x %x\n",
		    ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6],
		    ptr[7], ptr[8], ptr[9], ptr[10], ptr[11], ptr[12],
		    ptr[13]);
	}
#endif
	hwmsgbox_send_message(pmessage, SEND_MSG_TIMEOUT);

	if ((pmessage->attr & MESSAGE_ATTR_SOFTSYN) ||
		(pmessage->attr & MESSAGE_ATTR_HARDSYN))
		message_free(pmessage);

	InvokeTxCallback(state.tx.size, 0, 0);

	return 0;
}

int SharememTxPreamble(uint32_t busId, uint8_t byte, SharememCallbackF callback,
        void *cookie)
{
    return SharememTx(busId, NULL, byte, 0, callback, cookie);
}

int SharememTxPacket(uint32_t busId, const void *txBuf, size_t txSize,
        SharememCallbackF callback, void *cookie)
{
    return SharememTx(busId, txBuf, 0, txSize, callback, cookie);
}
