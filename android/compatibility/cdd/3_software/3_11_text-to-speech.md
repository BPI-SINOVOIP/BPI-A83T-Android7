## 3.11\. Text-to-Speech

Android includes APIs that allow applications to make use of text-to-speech
(TTS) services and allows service providers to provide implementations of TTS
services. Device implementations reporting the feature
android.hardware.audio.output MUST meet these requirements related to the
[Android TTS framework](http://developer.android.com/reference/android/speech/tts/package-summary.html).

Android Automotive implementations:

*   MUST support the Android TTS framework APIs.
*   MAY support installation of third-party TTS engines. If supported, partners
    MUST provide a user-accessible interface that allows the user to select a
    TTS engine for use at system level.

All other device implementations:

*   MUST support the Android TTS framework APIs and SHOULD include a TTS engine
    supporting the languages available on the device. Note that the upstream
    Android open source software includes a full-featured TTS engine
    implementation.
*   MUST support installation of third-party TTS engines.
*   MUST provide a user-accessible interface that allows users to select a TTS
    engine for use at the system level.
