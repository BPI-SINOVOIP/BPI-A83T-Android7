## 5.4\. Audio Recording

While some of the requirements outlined in this section are stated as SHOULD
since Android 4.3, the Compatibility Definition for a future version is planned
to change these to MUST. Existing and new Android devices are **STRONGLY
RECOMMENDED** to meet these requirements that are stated as SHOULD, or they
will not be able to attain Android compatibility when upgraded to the future
version.

### 5.4.1\. Raw Audio Capture

Device implementations that declare android.hardware.microphone MUST allow
capture of raw audio content with the following characteristics:

*   **Format**: Linear PCM, 16-bit
*   **Sampling rates**: 8000, 11025, 16000, 44100
*   **Channels**: Mono

The capture for the above sample rates MUST be done without up-sampling, and
any down-sampling MUST include an appropriate anti-aliasing filter.

Device implementations that declare android.hardware.microphone SHOULD allow
capture of raw audio content with the following characteristics:

*   **Format**: Linear PCM, 16-bit
*   **Sampling rates**: 22050, 48000
*   **Channels**: Stereo

If capture for the above sample rates is supported, then the capture MUST be
done without up-sampling at any ratio higher than 16000:22050 or 44100:48000.
Any up-sampling or down-sampling MUST include an appropriate anti-aliasing
filter.

### 5.4.2\. Capture for Voice Recognition

The android.media.MediaRecorder.AudioSource.VOICE_RECOGNITION audio source MUST
support capture at one of the sampling rates, 44100 and 48000.

In addition to the above recording specifications, when an application has
started recording an audio stream using the
android.media.MediaRecorder.AudioSource.VOICE_RECOGNITION audio source:

*   The device SHOULD exhibit approximately flat amplitude versus frequency
    characteristics: specifically, ±3 dB, from 100 Hz to 4000 Hz.
*   Audio input sensitivity SHOULD be set such that a 90 dB sound power level
    (SPL) source at 1000 Hz yields RMS of 2500 for 16-bit samples.
*   PCM amplitude levels SHOULD linearly track input SPL changes over at least a
    30 dB range from -18 dB to +12 dB re 90 dB SPL at the microphone.
*   Total harmonic distortion SHOULD be less than 1% for 1 kHz at 90 dB SPL
    input level at the microphone.
*   Noise reduction processing, if present, MUST be disabled.
*   Automatic gain control, if present, MUST be disabled.

If the platform supports noise suppression technologies tuned for speech
recognition, the effect MUST be controllable from the
android.media.audiofx.NoiseSuppressor API. Moreover, the UUID field for the
noise suppressor’s effect descriptor MUST uniquely identify each implementation
of the noise suppression technology.

### 5.4.3\. Capture for Rerouting of Playback

The android.media.MediaRecorder.AudioSource class includes the REMOTE_SUBMIX
audio source. Devices that declare android.hardware.audio.output MUST properly
implement the REMOTE_SUBMIX audio source so that when an application uses the
android.media.AudioRecord API to record from this audio source, it can capture
a mix of all audio streams except for the following:

*   STREAM_RING
*   STREAM_ALARM
*   STREAM_NOTIFICATION

