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
#define LOG_TAG "BootMusic_AudioPlayer"

#include "AudioPlayer.h"
#include <tinyalsa/asoundlib.h>
#include <utils/Log.h>
#include <utils/String8.h>

#include<string.h>
#include<stdio.h>
#include<fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define USE_BOOT_MUSIC_ROUTING 1
#define SPK        0
#define HEADSET    1
#define UEVENT_PATH         "/sys/class/switch/h2w/state"
#define INPUT_EVENT_PATH    "/sys/module/sunxi_sndcodec/parameters/switch_state"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

// Maximum line length for audio_conf.txt
// We only accept lines less than this length to avoid overflows using sscanf()
#define MAX_LINE_LENGTH 1024

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};


namespace android {

AudioPlayer::AudioPlayer() :
mCard(0), mDevice(0),
mPeriodSize(1024), mPeriodCount(2),
mfd(-1)
{
}

AudioPlayer::~AudioPlayer()
{
    if (mfd >= 0)
        close(mfd);
}

static bool setMixerValue(struct mixer* mixer, const char* name, const char* values)
{
    if (!mixer) {
        ALOGE("no mixer in setMixerValue");
        return false;
    }
    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mixer, name);
    if (!ctl) {
        ALOGE("mixer_get_ctl_by_name failed for %s", name);
        return false;
    }

    enum mixer_ctl_type type = mixer_ctl_get_type(ctl);
    int numValues = mixer_ctl_get_num_values(ctl);
    int intValue;
    char stringValue[MAX_LINE_LENGTH];

    for (int i = 0; i < numValues && values; i++) {
        // strip leading space
        while (*values == ' ') values++;
        if (*values == 0) break;

        switch (type) {
            case MIXER_CTL_TYPE_BOOL:
            case MIXER_CTL_TYPE_INT:
                if (sscanf(values, "%d", &intValue) == 1) {
                    if (mixer_ctl_set_value(ctl, i, intValue) != 0) {
                        ALOGE("mixer_ctl_set_value failed for %s %d", name, intValue);
                    }
                } else {
                    ALOGE("Could not parse %s as int for %s", values, name);
                }
                break;
            case MIXER_CTL_TYPE_ENUM:
                if (sscanf(values, "%s", stringValue) == 1) {
                    if (mixer_ctl_set_enum_by_string(ctl, stringValue) != 0) {
                        ALOGE("mixer_ctl_set_enum_by_string failed for %s %s", name, stringValue);
                    }
                } else {
                    ALOGE("Could not parse %s as enum for %s", values, name);
                }
                break;
            default:
                ALOGE("unsupported mixer type %d for %s", type, name);
                break;
        }

        values = strchr(values, ' ');
    }

    return true;
}

static int get_card(const char *card_name)
{
    int ret;
    int fd;
    int i;
    char path[128];
    char name[64];

    for (i = 0; i < 10; i++) {
        sprintf(path, "/sys/class/sound/card%d/id", i);
        ret = access(path, F_OK);
        if (ret) {
            ALOGW("can't find node %s, use card0", path);
            return 0;
        }

        fd = open(path, O_RDONLY);
        if (fd <= 0) {
            ALOGW("can't open %s, use card0", path);
            return 0;
        }

        ret = read(fd, name, sizeof(name));
        close(fd);
        if (ret > 0) {
            name[ret-1] = '\0';
            if (strstr(name, card_name))
                return i;
        }
    }

    ALOGW("can't find card:%s, use card0", card_name);
    return 0;
}

bool AudioPlayer::init(const char *conf_file)
{
    int tempInt;
    struct mixer* mixer = NULL;
    char    name[MAX_LINE_LENGTH];
    bool ret = true;
    int out_device = SPK;
    FILE *f_hs = NULL;
    char hs_state = '0';
    int is_mixer = 0;
    char *line;
    FILE *pf = NULL;

    line = (char *)malloc(MAX_LINE_LENGTH);
    if (!line) {
        ALOGE("could not malloc");
        ret = false;
        goto exit;
    }

    pf = fopen(conf_file, "r");
    if (!pf) {
        ALOGE("can't open %s", conf_file);
        ret = false;
        goto exit;
    }

    for (;;) {
        line = fgets(line, MAX_LINE_LENGTH, pf);
        if (!line)
            break;

        if (sscanf(line, "card=%d", &tempInt) == 1) {
            ALOGD("card=%d", tempInt);
            mCard = tempInt;

            mixer = mixer_open(mCard);
            if (!mixer) {
                ALOGE("could not open mixer for card %d", mCard);
                ret = false;
                goto exit;
            }
        } else if (sscanf(line, "card=%s", name) == 1) {
            mCard = get_card(name);
            ALOGD("card=%s, mCard=%d", name, mCard);

            mixer = mixer_open(mCard);
            if (!mixer) {
                ALOGE("could not open mixer for card %d", mCard);
                ret = false;
                goto exit;
            }
        } else if (sscanf(line, "device=%d", &tempInt) == 1) {
            ALOGD("device=%d", tempInt);
            mDevice = tempInt;
        } else if (sscanf(line, "period_size=%d", &tempInt) == 1) {
            ALOGD("period_size=%d", tempInt);
            mPeriodSize = tempInt;
        } else if (sscanf(line, "period_count=%d", &tempInt) == 1) {
            ALOGD("period_count=%d", tempInt);
            mPeriodCount = tempInt;
        } else if (1 == sscanf(line, "Headset detection=%[0-9a-zA-Z _]s", name)) {
            ALOGD("Headset detection=%s", name);
            /* headset detection method, input event or uevent. */
            if (0 == strcmp(name, "Input event") ||
                    0 == strcmp(name, "input event")) {
                f_hs = fopen(INPUT_EVENT_PATH, "r");
                ALOGD("input event");
            } else {
                f_hs = fopen(UEVENT_PATH, "r");
                ALOGD("uevent");
            }

            /* get output device, speaker or headset */
            if (NULL == f_hs) {
                ALOGW("%s,line:%d: can't open file node.", __func__, __LINE__);
                out_device = SPK;
            } else {
                fread((void*)&hs_state, sizeof(char), 1, f_hs);
                fclose(f_hs);
                if ('0' == hs_state)
                    out_device = SPK;
                else
                    out_device = HEADSET;
            }
            ALOGD("boot music out_device=%d", out_device);

        } else {
            if (SPK == out_device) {
                is_mixer = sscanf(line, "mixer \"%[0-9a-zA-Z _]s\"", name);
            } else {
                is_mixer = sscanf(line, "headset mixer \"%[0-9a-zA-Z _]s\"", name);
            }
            if (is_mixer) {
                const char* values = strchr(line, '=');
                if (values) {
                    values++;   // skip '='
                    ALOGD("name: \"%s\" = %s", name, values);
                    setMixerValue(mixer, name, values);
                } else {
                    ALOGE("values missing for name: \"%s\"", name);
                }
            }
        }
    }

exit:
    if (line)
        free(line);
    if (pf)
        fclose(pf);
    if (mixer)
    mixer_close(mixer);

    if (mCard >= 0 && mDevice >= 0) {
        return ret;
    }

    return false;
}

void AudioPlayer::playFile(const char *fileName)
{
    ALOGV("%s,line:%d", __func__, __LINE__);

    mfd = open(fileName, O_RDONLY);
    if (mfd < 0) {
        ALOGE("can't open boot music file:%s", fileName);
        return;
    }

    play();
}

bool AudioPlayer::play()
{
    bool moreChunks = true;
    unsigned int ret;

    struct riff_wave_header riffHeader;
    struct chunk_header chunkHeader;
    struct chunk_fmt chunkFmt;

    ALOGV("%s,line:%d", __func__, __LINE__);

    ret = read(mfd, &riffHeader, sizeof(riffHeader));
    if (ret < sizeof(riffHeader) ||
        (riffHeader.riff_id != ID_RIFF) ||
        (riffHeader.wave_id != ID_WAVE)) {
        ALOGE("Error: audio file is not a riff/wave file\n");
        return false;
    }

    do {
        ret = read(mfd, &chunkHeader, sizeof(chunkHeader));
        if (ret < sizeof(chunkHeader)) {
            ALOGE("EOF reading chunk headers");
            return false;
        }

        switch (chunkHeader.id) {
            case ID_FMT:
                ret = read(mfd, &chunkFmt, sizeof(chunkFmt));
                if (ret < sizeof(chunkFmt)) {
                    ALOGE("format not found in WAV file");
                    return false;
                }
                break;
            case ID_DATA:
                /* Stop looking for chunks */
                moreChunks = 0;
                break;
            default:
                /* Unknown chunk, skip bytes */
            break;
        }
    } while (moreChunks);

    struct pcm_config config;
    struct pcm *pcm = NULL;

    memset(&config, 0, sizeof(config));
    config.channels = chunkFmt.num_channels;
    config.rate = chunkFmt.sample_rate;
    config.period_size = mPeriodSize;
    config.period_count = mPeriodCount;
    //config.start_threshold = mPeriodSize / 4;
    //config.stop_threshold = INT_MAX;
    //config.avail_min = config.start_threshold;
    if (chunkFmt.bits_per_sample != 16) {
        ALOGE("only 16 bit WAV files are supported");
        return false;
    }
    config.format = PCM_FORMAT_S16_LE;

    pcm = pcm_open(mCard, mDevice, PCM_OUT, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        ALOGE("Unable to open PCM device (%s)\n", pcm_get_error(pcm));
        return false;
    }

    size_t buf_size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    void *buffer = malloc(buf_size);
    if (!buffer) {
        ALOGE("can't malloc buffer");
        if (pcm)
            pcm_close(pcm);
        return false;
    }

    while(1) {
        ret = read(mfd, buffer, buf_size);
        if (!ret) {
            ALOGV("%s,line:%d", __func__, __LINE__);
            break;
        }

        if (pcm_write(pcm, buffer, buf_size)) {
            ALOGE("pcm_write failed (%s)", pcm_get_error(pcm));
            break;
        }
    }
    free(buffer);

    if (pcm)
        pcm_close(pcm);

    return false;
}

} // namespace android
