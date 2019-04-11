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

#ifndef _SHAREMEM_H_
#define _SHAREMEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (*SharememCallbackF)(void *cookie, size_t tx, size_t rx, int err);

int ap_write_data(char *data, uint32_t length);

int SharememRequest(uint32_t busId);
int SharememRelease(uint32_t busId);

void SharememEnableRx(uint32_t busId, void *rxBuf, size_t rxSize,
        SharememCallbackF callback, void *cookie);
int SharememTxPreamble(uint32_t busId, uint8_t byte,
        SharememCallbackF callback, void *cookie);
int SharememTxPacket(uint32_t busId, const void *txBuf, size_t txSize,
        SharememCallbackF callback, void *cookie);

#ifdef __cplusplus
}
#endif

#endif /* _SHAREMEM_H_ */
