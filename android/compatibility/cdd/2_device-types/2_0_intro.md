# 2\. Device Types

While the Android Open Source Project has been used in the implementation of a
variety of device types and form factors, many aspects of the architecture and
compatibility requirements were optimized for handheld devices. Starting from
Android 5.0, the Android Open Source Project aims to embrace a wider variety of
device types as described in this section.

**Android Handheld device** refers to an Android device implementation that is
typically used by holding it in the hand, such as mp3 players, phones, and
tablets. Android Handheld device implementations:

*   MUST have a touchscreen embedded in the device.
*   MUST have a power source that provides mobility, such as a battery.

**Android Television device** refers to an Android device implementation that
is an entertainment interface for consuming digital media, movies, games, apps,
and/or live TV for users sitting about ten feet away (a “lean back” or “10-foot
user interface”). Android Television devices:

*   MUST have an embedded screen OR include a video output port, such as VGA,
    HDMI, or a wireless port for display.
*   MUST declare the features
    [android.software.leanback](http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_LEANBACK)
    and android.hardware.type.television.

**Android Watch device** refers to an Android device implementation intended to
be worn on the body, perhaps on the wrist, and:

*   MUST have a screen with the physical diagonal length in the range from 1.1
    to 2.5 inches.
*   MUST declare the feature android.hardware.type.watch.
*   MUST support uiMode =
    [UI_MODE_TYPE_WATCH](http://developer.android.com/reference/android/content/res/Configuration.html#UI_MODE_TYPE_WATCH).

**Android Automotive implementation** refers to a vehicle head unit running
Android as an operating system for part or all of the system and/or
infotainment functionality. Android Automotive implementations:

*   MUST have a screen with the physical diagonal length equal to or greater
    than 6 inches.
*   MUST declare the feature android.hardware.type.automotive.
*   MUST support uiMode =
    [UI_MODE_TYPE_CAR](http://developer.android.com/reference/android/content/res/Configuration.html#UI_MODE_TYPE_CAR).
*   Android Automotive implementations MUST support all public APIs in the
`android.car.*` namespace.

All Android device implementations that do not fit into any of the above device
types still MUST meet all requirements in this document to be Android
ANDROID_VERSION compatible, unless the requirement is explicitly described to
be only applicable to a specific Android device type from above.
