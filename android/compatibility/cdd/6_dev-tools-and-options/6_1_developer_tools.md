## 6.1\. Developer Tools

Device implementations MUST support the Android Developer Tools provided in the
Android SDK. Android compatible devices MUST be compatible with:

*   [**Android Debug Bridge (adb)**](http://developer.android.com/tools/help/adb.html)
    *   Device implementations MUST support all adb functions as documented in
the Android SDK including
[dumpsys](https://source.android.com/devices/input/diagnostics.html).
    *   The device-side adb daemon MUST be inactive by default and there MUST
be a user-accessible mechanism to turn on the Android Debug Bridge. If a device
implementation omits USB peripheral mode, it MUST implement the Android Debug
Bridge via local-area network (such as Ethernet or 802.11).
    *   Android includes support for secure adb. Secure adb enables adb on
known authenticated hosts. Device implementations MUST support secure adb.
*   [**Dalvik Debug Monitor Service (ddms)**](http://developer.android.com/tools/debugging/ddms.html)
    *   Device implementations MUST support all ddms features as documented in the Android SDK.
    *   As ddms uses adb, support for ddms SHOULD be inactive by default, but MUST be supported whenever the user has activated the Android Debug Bridge, as above.
*   [**Monkey**](http://developer.android.com/tools/help/monkey.html) Device
implementations MUST include the Monkey framework, and make it available for
applications to use.
*   [**SysTrace**](http://developer.android.com/tools/help/systrace.html)
    *   Device implementations MUST support systrace tool as documented in the
Android SDK. Systrace must be inactive by default, and there MUST be a
user-accessible mechanism to turn on Systrace.
    *   Most Linux-based systems and Apple Macintosh systems recognize Android
devices using the standard Android SDK tools, without additional support;
however Microsoft Windows systems typically require a driver for new Android
devices. (For instance, new vendor IDs and sometimes new device IDs require
custom USB drivers for Windows systems.)
    *   If a device implementation is unrecognized by the adb tool as provided
in the standard Android SDK, device implementers MUST provide Windows drivers
allowing developers to connect to the device using the adb protocol. These
drivers MUST be provided for Windows XP, Windows Vista, Windows 7, Windows 8,
and Windows 10 in both 32-bit and 64-bit versions.
