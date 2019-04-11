## 5.6\. Audio Latency

Audio latency is the time delay as an audio signal passes through a system.
Many classes of applications rely on short latencies, to achieve real-time
sound effects.

For the purposes of this section, use the following definitions:

*   **output latency**. The interval between when an application writes a frame
of PCM-coded data and when the corresponding sound is presented to environment at an on-device transducer
or signal leaves the device via a port and can be observed externally.
*   **cold output latency**. The output latency for the first frame, when the
audio output system has been idle and powered down prior to the request.
*   **continuous output latency**. The output latency for subsequent frames,
after the device is playing audio.
*   **input latency**. The interval between when a sound is presented by environment to device
at an on-device transducer or signal enters the device via a port
and when an application reads the corresponding frame of
PCM-coded data.
*   **lost input**. The initial portion of an input signal that is unusable or unavailable.
*   **cold input latency**. The sum of lost input time and the input latency
for the first frame, when the audio input system has been idle and powered down
prior to the request.
*   **continuous input latency**. The input latency for subsequent frames,
while the device is capturing audio.
*   **cold output jitter**. The variability among separate measurements of cold
output latency values.
*   **cold input jitter**. The variability among separate measurements of cold
input latency values.
*   **continuous round-trip latency**. The sum of continuous input latency plus
continuous output latency plus one buffer period. The buffer period allows
time for the app to process the signal and time for the app to mitigate phase difference
between input and output streams.
*   **OpenSL ES PCM buffer queue API**. The set of PCM-related OpenSL ES APIs
within [Android NDK](https://developer.android.com/ndk/index.html).

Device implementations that declare android.hardware.audio.output are STRONGLY
RECOMMENDED to meet or exceed these audio output requirements:

*   cold output latency of 100 milliseconds or less
*   continuous output latency of 45 milliseconds or less
*   minimize the cold output jitter

If a device implementation meets the requirements of this section after any
initial calibration when using the OpenSL ES PCM buffer queue API, for
continuous output latency and cold output latency over at least one supported
audio output device, it is STRONGLY RECOMMENDED to report support for
low-latency audio, by reporting the feature android.hardware.audio.low_latency
via the
[android.content.pm.PackageManager](http://developer.android.com/reference/android/content/pm/PackageManager.html)
class. Conversely, if the device implementation does not meet these
requirements it MUST NOT report support for low-latency audio.

Device implementations that include android.hardware.microphone are STRONGLY
RECOMMENDED to meet these input audio requirements:

*   cold input latency of 100 milliseconds or less
*   continuous input latency of 30 milliseconds or less
*   continuous round-trip latency of 50 milliseconds or less
*   minimize the cold input jitter
