## 7.8\. Audio

### 7.8.1\. Microphone

<div class="note">

Android Handheld, Watch, and Automotive implementations MUST include a
microphone.

</div>

Device implementations MAY omit a microphone. However, if a device
implementation omits a microphone, it MUST NOT report the
android.hardware.microphone feature constant, and MUST implement the audio
recording API at least as no-ops, per [section 7](#7_hardware_compatibility).
Conversely, device implementations that do possess a microphone:

*   MUST report the android.hardware.microphone feature constant.
*   MUST meet the audio recording requirements in [section 5.4](#5_4_audio_recording).
*   MUST meet the audio latency requirements in [section 5.6](#5_6_audio_latency).
*   STRONGLY RECOMMENDED to support near-ultrasound recording as described in
[section 7.8.3](#7_8_3_near_ultrasound).

### 7.8.2\. Audio Output

<div class="note">

Android Watch devices MAY include an audio output.

</div>

Device implementations including a speaker or with an audio/multimedia output
port for an audio output peripheral as a headset or an external speaker:

*   MUST report the android.hardware.audio.output feature constant.
*   MUST meet the audio playback requirements in [section 5.5](#5_5_audio_playback).
*   MUST meet the audio latency requirements in [section 5.6](#5_6_audio_latency).
*   STRONGLY RECOMMENDED to support near-ultrasound playback as described in
[section 7.8.3](#7_8_3_near_ultrasound).

Conversely, if a device implementation does not include a speaker or audio
output port, it MUST NOT report the android.hardware.audio output feature, and
MUST implement the Audio Output related APIs as no-ops at least.

Android Watch device implementation MAY but SHOULD NOT have audio output, but
other types of Android device implementations MUST have an audio output and
declare android.hardware.audio.output.

#### 7.8.2.1\. Analog Audio Ports

In order to be compatible with the
[headsets and other audio accessories](http://source.android.com/accessories/headset-spec.html)
using the 3.5mm audio plug across the Android ecosystem, if a device
implementation includes one or more analog audio ports, at least one of the
audio port(s) SHOULD be a 4 conductor 3.5mm audio jack. If a device
implementation has a 4 conductor 3.5mm audio jack, it:

*   MUST support audio playback to stereo headphones and stereo headsets with a
microphone, and SHOULD support audio recording from stereo headsets with a
microphone.
*   MUST support TRRS audio plugs with the CTIA pin-out order, and SHOULD
support audio plugs with the OMTP pin-out order.
*   MUST support the detection of microphone on the plugged in audio accessory,
if the device implementation supports a microphone, and broadcast the
android.intent.action.HEADSET_PLUG with the extra value microphone set as 1.
*   MUST support the detection and mapping to the keycodes for the following
3 ranges of equivalent impedance between the microphone and ground conductors
on the audio plug:
    *   **70 ohm or less**: KEYCODE_HEADSETHOOK
    *   **210-290 Ohm**: KEYCODE_VOLUME_UP
    *   **360-680 Ohm**: KEYCODE_VOLUME_DOWN
*   STRONGLY RECOMMENDED to detect and map to the keycode for the following
range of equivalent impedance between the microphone and ground conductors
on the audio plug:
    *   **110-180 Ohm:** KEYCODE_VOICE_ASSIST
*   MUST trigger ACTION_HEADSET_PLUG upon a plug insert, but only after all
contacts on plug are touching their relevant segments on the jack.
*   MUST be capable of driving at least 150mV ± 10% of output voltage on a 32
Ohm speaker impedance.
*   MUST have a microphone bias voltage between 1.8V ~ 2.9V.

### 7.8.3\. Near-Ultrasound

Near-Ultrasound audio is the 18.5 kHz to 20 kHz band. Device implementations
MUST correctly report the support of near-ultrasound audio capability via the
[AudioManager.getProperty](http://developer.android.com/reference/android/media/AudioManager.html#getProperty%28java.lang.String%29)
API as follows:

*   If
[PROPERTY_SUPPORT_MIC_NEAR_ULTRASOUND](http://developer.android.com/reference/android/media/AudioManager.html#PROPERTY_SUPPORT_MIC_NEAR_ULTRASOUND)
is "true", then the following requirements must be met by the
VOICE_RECOGNITION and UNPROCESSED audio sources:
    *   The microphone's mean power response in the 18.5 kHz to 20 kHz band
MUST be no more than 15 dB below the response at 2 kHz.
    * The microphone's unweighted signal to noise ratio over 18.5 kHz to 20 kHz
for a 19 kHz tone at -26 dBFS MUST be no lower than 50 dB.
*   If
[PROPERTY_SUPPORT_SPEAKER_NEAR_ULTRASOUND](http://developer.android.com/reference/android/media/AudioManager.html#PROPERTY_SUPPORT_SPEAKER_NEAR_ULTRASOUND)
is "true", then the speaker's mean response in 18.5 kHz - 20 kHz MUST be no
lower than 40 dB below the response at 2 kHz.
