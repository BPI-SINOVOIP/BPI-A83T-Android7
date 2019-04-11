## 9.13\. Safe Boot Mode

Android provides a mode enabling users to boot up into a mode where only
preinstalled system apps are allowed to run and all third-party apps are
disabled. This mode, known as "Safe Boot Mode", provides the user the
capability to uninstall potentially harmful third-party apps.

Android device implementations are STRONGLY RECOMENDED to implement Safe Boot
Mode and meet following requirements:

*  Device implementations SHOULD provide the user an option to enter Safe Boot
   Mode from the boot menu which is reachable through a workflow that is different
   from that of normal boot.

*  Device implementations MUST provide the user an option to enter Safe Boot Mode
   in such a way that is uninterruptible from third-party apps installed on
   the device, except for when the third party app is a Device Policy Controller
   and has set the [`UserManager.DISALLOW_SAFE_BOOT`](https://developer.android.com/reference/android/os/UserManager.html#DISALLOW_SAFE_BOOT)
   flag as true.

*  Device implementations MUST provide the user the capability to uninstall
   any third-party apps within Safe Mode.
