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

#include <hostIntf.h>
#include <hostIntf_priv.h>
#include <nanohubPacket.h>
#include <sharemem.h>
#include <plat/inc/include.h>

struct SharememCallbackCookie {
	HostIntfCommCallbackF callback;
	size_t size;
	int err;
};

static struct SharememCallbackCookie CallbackCookie;

static uint32_t gBusId;

static void SharememCallback(void *paras)
{
	struct SharememCallbackCookie *cookie = paras;
	cookie->callback(cookie->size, cookie->err);
}

static void hostIntfSharememRxCallback(void *cookie, size_t tx, size_t rx, int err)
{
    CallbackCookie.callback = cookie;
    CallbackCookie.size = rx;
    CallbackCookie.err = err;

    if (!osDefer(SharememCallback, (void *)&CallbackCookie, true)) {
        osLog(LOG_ERROR, "hostIntfSharememRxCallback osDefer error\n");
    }
}

static void hostIntfSharememTxCallback(void *cookie, size_t tx, size_t rx, int err)
{
    HostIntfCommCallbackF callback = cookie;
    callback(tx, err);
}

static int hostIntfSharememRequest()
{
    return SharememRequest(gBusId);
}

static int hostIntfSharememRxPacket(void *rxBuf, size_t rxSize,
        HostIntfCommCallbackF callback)
{
    SharememEnableRx(gBusId, rxBuf, rxSize, hostIntfSharememRxCallback,
            callback);
    return 0;
}

static int hostIntfSharememTxPacket(const void *txBuf, size_t txSize,
        HostIntfCommCallbackF callback)
{
    //osLog(LOG_ERROR, "%s-%u:len:%u\n", __func__, __LINE__, txSize);
    return SharememTxPacket(gBusId, txBuf, txSize, hostIntfSharememTxCallback,
            callback);
}

static int hostIntfSharememRelease(void)
{
    return SharememRelease(gBusId);
}

static const struct HostIntfComm gSharememComm = {
   .request = hostIntfSharememRequest,
   .rxPacket = hostIntfSharememRxPacket,
   .txPacket = hostIntfSharememTxPacket,
   .release = hostIntfSharememRelease,
};

const struct HostIntfComm *hostIntfSharememInit(uint8_t busId)
{
    gBusId = busId;
    return &gSharememComm;
}
