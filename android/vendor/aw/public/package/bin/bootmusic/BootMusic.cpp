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

//#define LOG_NDEBUG 0
#define LOG_TAG "BootMusic"

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <pthread.h>
#include <sys/select.h>

#include <cutils/properties.h>

#include <binder/IPCThreadState.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/threads.h>

#include <sys/stat.h>
#include <unistd.h>

#include "AudioPlayer.h"

#include "BootMusic.h"

#define UNUSED(x) (void)(x)

#define AUDIO_CONF_FILE "/system/media/audio_conf.txt"
#define BOOT_MUSIC_FILE "/system/media/boot.wav"

namespace android {

// ---------------------------------------------------------------------------

BootMusic::BootMusic() : Thread(false) {
    ALOGV("%s,line:%d", __func__, __LINE__);
}

BootMusic::~BootMusic() {
    ALOGV("%s,line:%d", __func__, __LINE__);
}

void BootMusic::onFirstRef() {
    ALOGV("%s,line:%d", __func__, __LINE__);
    run("BootMusic", PRIORITY_AUDIO);
}

status_t BootMusic::readyToRun() {
    ALOGV("%s,line:%d", __func__, __LINE__);
    return NO_ERROR;
}

void BootMusic::binderDied(const wp<IBinder>&)
{
    // woah, surfaceflinger died!
    ALOGD("SurfaceFlinger died, exiting...");

    // calling requestExit() is not enough here because the Surface code
    // might be blocked on a condition variable that will never be updated.
    kill( getpid(), SIGKILL );
    requestExit();
}

bool BootMusic::threadLoop()
{
    ALOGV("%s,line:%d", __func__, __LINE__);

    bool r;
    // We have no bootanimation file, so we use the stock android logo
    // animation.

    r = playMusic();
    IPCThreadState::self()->stopProcess();
    return r;
}

bool BootMusic::playMusic()
{
    ALOGV("%s,line:%d", __func__, __LINE__);

    char value[PROPERTY_VALUE_MAX];
    property_get("persist.sys.nobootmusic", value, "0");
    int noBootMusic = atoi(value) || (access("/cache/nobootmusic", F_OK) == 0);
    ALOGD("nobootmusic: %d\n", noBootMusic);
    if (noBootMusic) {
        return false;
    }

    if (0 != access(AUDIO_CONF_FILE, R_OK)) {
        ALOGE("can't read:%s", AUDIO_CONF_FILE);
        return false;
    }
    if (0 != access(BOOT_MUSIC_FILE, R_OK)) {
        ALOGE("can't read:%s", BOOT_MUSIC_FILE);
        return false;
    }

    AudioPlayer *player = new AudioPlayer();
    if (player->init(AUDIO_CONF_FILE))
        player->playFile(BOOT_MUSIC_FILE);

    delete player;

    return false;
}

// ---------------------------------------------------------------------------

}
; // namespace android
