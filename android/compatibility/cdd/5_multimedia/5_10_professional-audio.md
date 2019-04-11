## 5.10\. Professional Audio

If a device implementation meets _all_ of the following requirements, it is
STRONGLY RECOMMENDED to report support for feature android.hardware.audio.pro
via the
[android.content.pm.PackageManager](http://developer.android.com/reference/android/content/pm/PackageManager.html)
class.

*   The device implementation MUST report support for feature
android.hardware.audio.low_latency.
*   The continuous round-trip audio latency, as defined in section 5.6 Audio
Latency, MUST be 20 milliseconds or less and SHOULD be 10 milliseconds or less
over at least one supported path.
*   If the device includes a 4 conductor 3.5mm audio jack, the continuous
round-trip audio latency MUST be 20 milliseconds or less over the audio jack
path, and SHOULD be 10 milliseconds or less over at the audio jack path.
*   The device implementation MUST include a USB port(s) supporting USB host
mode and USB peripheral mode.
*   The USB host mode MUST implement the USB audio class.
*   If the device includes an HDMI port, the device implementation MUST support
output in stereo and eight channels at 20-bit or 24-bit depth and 192 kHz
without bit-depth loss or resampling.
*   The device implementation MUST report support for feature
android.software.midi.
*   If the device includes a 4 conductor 3.5mm audio jack, the device
implementation is STRONGLY RECOMMENDED to comply with section [Mobile device
(jack) specifications](https://source.android.com/accessories/headset/specification.html#mobile_device_jack_specifications)
of the [Wired Audio Headset Specification (v1.1)](https://source.android.com/accessories/headset/specification.html).

Latencies and USB audio requirements MUST be met using the
[OpenSL ES](https://developer.android.com/ndk/guides/audio/opensl-for-android.html)
PCM buffer queue API.

In addition, a device implementation that reports support for this feature SHOULD:

*   Provide a sustainable level of CPU performance while audio is active.
*   Minimize audio clock inaccuracy and drift relative to standard time.
*   Minimize audio clock drift relative to the CPU `CLOCK_MONOTONIC` when both are active.
*   Minimize audio latency over on-device transducers.
*   Minimize audio latency over USB digital audio.
*   Document audio latency measurements over all paths.
*   Minimize jitter in audio buffer completion callback entry times, as this affects usable percentage of full CPU bandwidth by the callback.
*   Provide zero audio underruns (output) or overruns (input) under normal use at reported latency.
*   Provide zero inter-channel latency difference.
*   Minimize MIDI mean latency over all transports.
*   Minimize MIDI latency variability under load (jitter) over all transports.
*   Provide accurate MIDI timestamps over all transports.
*   Minimize audio signal noise over on-device transducers, including the period immediately after cold start.
*   Provide zero audio clock difference between the input and output sides of corresponding
    end-points, when both are active.  Examples of corresponding end-points include
    the on-device microphone and speaker, or the audio jack input and output.
*   Handle audio buffer completion callbacks for the input and output sides of corresponding
    end-points on the same thread when both are active, and enter the output callback immediately
    after the return from the input callback.  Or if it is not feasible to handle the callbacks
    on the same thread, then enter the output callback shortly after entering the input callback
    to permit the application to have a consistent timing of the input and output sides.
*   Minimize the phase difference between HAL audio buffering for the input and output
    sides of corresponding end-points.
*   Minimize touch latency.
*   Minimize touch latency variability under load (jitter).
