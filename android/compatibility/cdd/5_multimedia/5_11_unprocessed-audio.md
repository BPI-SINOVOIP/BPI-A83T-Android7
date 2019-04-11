## 5.11\. Capture for Unprocessed

Starting from Android 7.0,
a new recording source has been added. It can be accessed using
the `android.media.MediaRecorder.AudioSource.UNPROCESSED` audio
source. In OpenSL ES, it can be accessed with the record preset
`SL_ANDROID_RECORDING_PRESET_UNPROCESSED`.

A device MUST satisfy all of the following requirements to report support
of the unprocessed audio source via the `android.media.AudioManager` property
[PROPERTY_SUPPORT_AUDIO_SOURCE_UNPROCESSED](http://developer.android.com/reference/android/media/AudioManager.html#PROPERTY_SUPPORT_AUDIO_SOURCE_UNPROCESSED):

* The device MUST exhibit approximately flat amplitude-versus-frequency
characteristics in the mid-frequency range: specifically &plusmn;10dB from
100 Hz to 7000 Hz.

* The device MUST exhibit amplitude levels in the low frequency range:
specifically from &plusmn;20 dB from 5 Hz to 100 Hz compared to the mid-frequency range.

* The device MUST exhibit amplitude levels in the high frequency range:
specifically from &plusmn;30 dB from 7000 Hz to 22 KHz compared to the mid-frequency range.

* Audio input sensitivity MUST be set such that a 1000 Hz sinusoidal tone
source played at 94 dB Sound Pressure Level (SPL)
yields a response with RMS of 520 for 16
bit-samples (or -36 dB Full Scale for floating point/double precision
samples).

* SNR > 60 dB (difference between 94 dB SPL and equivalent SPL of self
noise, A-weighted).

* Total harmonic distortion MUST be less than 1% for 1 kHZ at 90 dB SPL
input level at the microphone.

* The only signal processing allowed in the path is a level multiplier
to bring the level to desired range. This level multiplier MUST NOT
introduce delay or latency to the signal path.

* No other signal processing is allowed in the path, such as Automatic Gain
Control, High Pass Filter, or Echo Cancellation. If any signal processing
is present in the architecture for any reason, it MUST be disabled and
effectively introduce zero delay or extra latency to the signal path.

All SPL measurements are made directly next to the microphone under test.

For multiple microphone configurations, these requirements apply to each
microphone.

It is STRONGLY RECOMMENDED that a device satisfy as many of the requirements for the signal
path for the unprocessed recording source; however, a device must satisfy _all_ of these
requirements, listed above, if it claims to support the unprocessed audio source.
