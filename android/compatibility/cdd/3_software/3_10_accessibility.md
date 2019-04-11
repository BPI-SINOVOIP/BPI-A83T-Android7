## 3.10\. Accessibility

Android provides an accessibility layer that helps users with disabilities to
navigate their devices more easily. In addition, Android provides platform APIs
that enable [accessibility service implementations](http://developer.android.com/reference/android/accessibilityservice/AccessibilityService.html)
to receive callbacks for user and system events and generate alternate feedback
mechanisms, such as text-to-speech, haptic feedback, and trackball/d-pad
navigation.

Device implementations include the following requirements:

*   Android Automotive implementations SHOULD provide an implementation of the
    Android accessibility framework consistent with the default Android
    implementation.
*   Device implementations (Android Automotive excluded) MUST provide an
    implementation of the Android accessibility framework consistent with the
    default Android implementation.
*   Device implementations (Android Automotive excluded) MUST support
    third-party accessibility service implementations through the
    [android.accessibilityservice APIs](http://developer.android.com/reference/android/view/accessibility/package-summary.html).
*   Device implementations (Android Automotive excluded) MUST generate
    AccessibilityEvents and deliver these events to all registered
    AccessibilityService implementations in a manner consistent with the default
    Android implementation
*   Device implementations (Android Automotive and Android Watch devices with no
    audio output excluded), MUST provide a user-accessible mechanism to enable
    and disable accessibility services, and MUST display this interface in
    response to the android.provider.Settings.ACTION_ACCESSIBILITY_SETTINGS
    intent.


* Android device implementations with audio output are STRONGLY RECOMMENDED to provide
  implementations of accessibility services on the device comparable in or exceeding functionality
  of the TalkBack** and Switch Access accessibility services (https://github.com/google/talkback).
* Android Watch devices with audio output SHOULD provide implementations of an accessibility service
  on the device comparable in or exceeding functionality of the TalkBack accessibility service
  (https://github.com/google/talkback).
* Device implementations SHOULD provide a mechanism in the out-of-box setup flow for users to enable
  relevant accessibility services, as well as options to adjust the font size, display size and
  magnification gestures.

** For languages supported by Text-to-speech.

Also, note that if there is a preloaded accessibility service, it MUST be a Direct Boot aware
{directBootAware} app if the device has encrypted storage using File Based
Encryption (FBE).
