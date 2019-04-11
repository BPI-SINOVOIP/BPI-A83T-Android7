## 6.2\. Developer Options

Android includes support for developers to configure application
development-related settings. Device implementations MUST honor the
[android.settings.APPLICATION_DEVELOPMENT_SETTINGS](http://developer.android.com/reference/android/provider/Settings.html#ACTION_APPLICATION_DEVELOPMENT_SETTINGS)
intent to show application development-related settings The upstream Android
implementation hides the Developer Options menu by default and enables users to
launch Developer Options after pressing seven (7) times on the **Settings** >
**About Device** > **Build Number** menu item. Device implementations MUST
provide a consistent experience for Developer Options. Specifically, device
implementations MUST hide Developer Options by default and MUST provide a
mechanism to enable Developer Options that is consistent with the upstream
Android implementation.

<div class="note">

Android Automotive implementations MAY limit access to the Developer Options
menu by visually hiding or disabling the menu when the vehicle is in motion.

</div>
