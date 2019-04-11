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

#ifndef _BOOTMUSIC_AUDIOPLAYER_H
#define _BOOTMUSIC_AUDIOPLAYER_H

#include <utils/Thread.h>
#include <utils/FileMap.h>

namespace android {

class AudioPlayer
{
public:
                AudioPlayer();
    virtual     ~AudioPlayer();
    bool        init(const char *conf_file);

    void        playFile(const char *fileName);

private:
    bool play();

    int mCard;
    int mDevice;
    int mPeriodSize;
    int mPeriodCount;

    int mfd;
};

} // namespace android

#endif // _BOOTMUSIC_AUDIOPLAYER_H
