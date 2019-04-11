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

#ifndef ANDRIOD_GPIOSERVICE_H
#define ANDRIOD_GPIOSERVICE_H

#include <utils/Log.h>
#include <utils/Errors.h>
#include "IDisplayService.h"
#include <hardware/display.h>
#include <hardware/hardware.h>

namespace android{

class DisplayService:public BnDisplayService{

public:
    static void instantiate();
    virtual int displayCtrl(int disp, int cmd, int val0, int val1);
private:
    DisplayService();
    int displayOpen();
    int dispClose();
    virtual ~DisplayService();
    int mopenFlag;
    display_device_t *mdevice;
};
};
#endif
