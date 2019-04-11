## 3.13\. Quick Settings

Android device implementations SHOULD include a Quick Settings UI component that
allow quick access to frequently used or urgently needed actions.

Android includes the [`quicksettings`](https://developer.android.com/reference/android/service/quicksettings/package-summary.html)
API allowing third party apps to implement tiles that can be added by the user
alongside the system-provided tiles in the Quick Settings UI component. If a
device implementation has a Quick Settings UI component, it:

*   MUST allow the user to add or remove tiles from a third-party app to Quick
    Settings.
*   MUST NOT automatically add a tile from a third-party app directly to Quick
    Settings.
*   MUST display all the user-added tiles from third-party apps alongside the
    system-provided quick setting tiles.
