/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#undef NDEBUG
#define LOD_TAG "IDisplayService"
#include "utils/Log.h"

#include <memory.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include "IDisplayService.h"

#include <utils/Errors.h>
#define DEBUG false

namespace android{

enum {
    DISP_CTRL = IBinder::FIRST_CALL_TRANSACTION,
    READ = IBinder::LAST_CALL_TRANSACTION
};

class BpDisplayService: public BpInterface<IDisplayService>{

public:
    BpDisplayService(const sp<IBinder>& impl)
        : BpInterface<IDisplayService>(impl){
    }

    int displayCtrl(int disp, int cmd, int val0, int val1) {
        if(DEBUG) {
            ALOGD("IDisplayService::ctrl() disp = %d, cmd = %d, val0 = %d, val1 = %d",
                    disp, cmd, val0, val1);
        }
        Parcel data, reply;
        data.writeInterfaceToken(IDisplayService::getInterfaceDescriptor());
        data.writeInt32(disp);
        data.writeInt32(cmd);
        data.writeInt32(val0);
        data.writeInt32(val1);
        status_t status = remote()->transact(DISP_CTRL, data, &reply);
        if (status != NO_ERROR) {
            ALOGW("displayCtrl() could not contact remote: %d\n", status);
            return -1;
        }
        int32_t err = reply.readExceptionCode();
        int32_t ret = reply.readInt32();
        if (err < 0) {
            ALOGW("displayCtrl() caught exception %d\n", err);
            return -1;
        }
        return ret;
    }
};

IMPLEMENT_META_INTERFACE(DisplayService, "com.softwinner.IDisplayService");

status_t BnDisplayService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags){
    switch(code){
        case DISP_CTRL: {
            CHECK_INTERFACE(IDisplayService, data, reply);
            int disp = data.readInt32();
            int cmd = data.readInt32();
            int val0 = data.readInt32();
            int val1 = data.readInt32();
            reply->writeNoException();
            reply->writeInt32(displayCtrl(disp, cmd, val0, val1));
            return NO_ERROR;
            break;
        }
        default:
            ALOGW("unkonwn code: %d\n", code);
            break;
    }

    return BBinder::onTransact(code, data, reply, flags);
}

};
