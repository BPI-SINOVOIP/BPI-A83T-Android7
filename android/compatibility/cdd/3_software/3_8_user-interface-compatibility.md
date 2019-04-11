## 3.8\. User Interface Compatibility

### 3.8.1\. Launcher (Home Screen)

Android includes a launcher application (home screen) and support for
third-party applications to replace the device launcher (home screen). Device
implementations that allow third-party applications to replace the device home
screen MUST declare the platform feature android.software.home_screen.

### 3.8.2\. Widgets

<div class="note">

Widgets are optional for all Android device implementations, but SHOULD be
supported on Android Handheld devices.

</div>

Android defines a component type and corresponding API and lifecycle that allows
applications to expose an
[“AppWidget”](http://developer.android.com/guide/practices/ui_guidelines/widget_design.html)
to the end user, a feature that is STRONGLY RECOMMENDED to be supported on
Handheld Device implementations. Device implementations that support embedding
widgets on the home screen MUST meet the following requirements and declare
support for platform feature android.software.app_widgets.

*   Device launchers MUST include built-in support for AppWidgets and expose
    user interface affordances to add, configure, view, and remove AppWidgets
    directly within the Launcher.
*   Device implementations MUST be capable of rendering widgets that are 4 x 4
    in the standard grid size. See the [App Widget Design
    Guidelines](http://developer.android.com/guide/practices/ui_guidelines/widget_design.html)
    in the Android SDK documentation for details.
*   Device implementations that include support for lock screen MAY support
    application widgets on the lock screen.
*   SHOULD trigger the fast-switch action between the two most recently used apps,
    when the recents function key is tapped twice.
*   SHOULD trigger the split-screen multiwindow-mode, if supported, when the recents
    functions key is long pressed.


### 3.8.3\. Notifications

Android includes APIs that allow developers to [notify users of notable events](http://developer.android.com/guide/topics/ui/notifiers/notifications.html)
using hardware and software features of the device.

Some APIs allow applications to perform notifications or attract attention using
hardware—specifically sound, vibration, and light. Device implementations MUST
support notifications that use hardware features, as described in the SDK
documentation, and to the extent possible with the device implementation
hardware. For instance, if a device implementation includes a vibrator, it MUST
correctly implement the vibration APIs. If a device implementation lacks
hardware, the corresponding APIs MUST be implemented as no-ops. This behavior is
further detailed in [section 7](#7_hardware_compatibility).

Additionally, the implementation MUST correctly render all
[resources](https://developer.android.com/guide/topics/resources/available-resources.html)
(icons, animation files etc.) provided for in the APIs, or in the Status/System
Bar [icon style guide](http://developer.android.com/design/style/iconography.html), which in the
case of an Android Television device includes the possibility to not display the
notifications. Device implementers MAY provide an alternative user experience
for notifications than that provided by the reference Android Open Source
implementation; however, such alternative notification systems MUST support
existing notification resources, as above.

<div class="note">

Android Automotive implementations MAY manage the visibility and timing of
notifications to mitigate driver distraction, but MUST display
notifications that use
<a href="https://developer.android.com/reference/android/app/Notification.CarExtender.html">CarExtender</a>
when requested by applications.

</div>

Android includes support for various notifications, such as:

*   **Rich notifications**. Interactive Views for ongoing notifications.
*   **Heads-up notifications**. Interactive Views users can act on or dismiss without leaving the current app.
*   **Lock screen notifications**. Notifications shown over a lock screen with granular control on visibility.

Android device implementations, when such notifications are made visible, MUST
properly execute Rich and Heads-up notifications and include the title/name,
icon, text as [documented in the Android APIs](https://developer.android.com/design/patterns/notifications.html).

Android includes Notification Listener Service APIs that allow apps (once
explicitly enabled by the user) to receive a copy of all notifications as they
are posted or updated. Device implementations MUST correctly and promptly send
notifications in their entirety to all such installed and user-enabled listener
services, including any and all metadata attached to the Notification object.

Handheld device implementations MUST support the behaviors of updating,
removing, replying to, and bundling notifications as described in this
[section](https://developer.android.com/guide/topics/ui/notifiers/notifications.html#Managing).

Also, handheld device implementations MUST provide:

*   The ability to control notifications directly in the notification shade.
*   The visual affordance to trigger the control panel in the notification shade.
*   The ability to BLOCK, MUTE and RESET notification preference from a
    package, both in the inline control panel as well as in the settings app.

All 6 direct subclasses of the `Notification.Style class` MUST be supported as
described in the [SDK documents](https://developer.android.com/reference/android/app/Notification.Style.html).

Device implementations that support the DND (Do not Disturb) feature MUST meet
the following requirements:

*   MUST implement an activity where the user can grant or deny the app access
    to DND policy configurations in response to the intent [ACTION_NOTIFICATION_POLICY_ACCESS_SETTINGS](https://developer.android.com/reference/android/provider/Settings.html#ACTION_NOTIFICATION_POLICY_ACCESS_SETTINGS).
*   MUST display [Automatic DND rules](https://developer.android.com/reference/android/app/NotificationManager.html#addAutomaticZenRule%28android.app.AutomaticZenRule%29)
    created by applications alongside the user-created and pre-defined rules.
*   MUST honor the [`suppressedVisualEffects`](https://developer.android.com/reference/android/app/NotificationManager.Policy.html#suppressedVisualEffects)
    values passed along the [`NotificationManager.Policy`](https://developer.android.com/reference/android/app/NotificationManager.Policy.html#NotificationManager.Policy%28int, int, int, int%29).

### 3.8.4\. Search

Android includes APIs that allow developers to
[incorporate search](http://developer.android.com/reference/android/app/SearchManager.html)
into their applications and expose their application’s data into the global
system search. Generally speaking, this functionality consists of a single,
system-wide user interface that allows users to enter queries, displays
suggestions as users type, and displays results. The Android APIs allow
developers to reuse this interface to provide search within their own apps and
allow developers to supply results to the common global search user interface.

Android device implementations SHOULD include global search, a single, shared,
system-wide search user interface capable of real-time suggestions in response
to user input. Device implementations SHOULD implement the APIs that allow
developers to reuse this user interface to provide search within their own
applications. Device implementations that implement the global search interface
MUST implement the APIs that allow third-party applications to add suggestions
to the search box when it is run in global search mode. If no third-party
applications are installed that make use of this functionality, the default
behavior SHOULD be to display web search engine results and suggestions.

Android device implementations SHOULD, and Android Automotive implementations
MUST, implement an assistant on the device to
handle the [Assist action](http://developer.android.com/reference/android/content/Intent.html#ACTION_ASSIST).

Android also includes the [Assist APIs](https://developer.android.com/reference/android/app/assist/package-summary.html)
to allow applications to elect how much information of the current context is
shared with the assistant on the device. Device implementations supporting the
Assist action MUST indicate clearly to the end user when the context is
shared by displaying a white light around the edges of the screen. To ensure
clear visibility to the end user, the indication MUST meet or exceed the
duration and brightness of the Android Open Source Project implementation.

This indication MAY be disabled by default for preinstalled apps using the Assist and
VoiceInteractionService API, if all following requirements are met:

*   The preinstalled app MUST request the context to be shared only when the
    user invoked the app by one of the following means, and the app is running in the
    foreground:
    *   hotword invocation
    *   input of the ASSIST navigation key/button/gesture

*   The device implementation MUST provide an affordance to enable the
    indication, less than two navigations away from
    (the default voice input and assistant app settings menu)
    [section 3.2.3.5](#3_2_3_5_default_app_settings).

### 3.8.5\. Toasts

Applications can use the [“Toast” API](http://developer.android.com/reference/android/widget/Toast.html) to
display short non-modal strings to the end user that disappear after a brief
period of time. Device implementations MUST display Toasts from applications to
end users in some high-visibility manner.

### 3.8.6\. Themes

Android provides “themes” as a mechanism for applications to apply styles across
an entire Activity or application.

Android includes a “Holo” theme family as a set of defined styles for
application developers to use if they want to match the
[Holo theme look and feel](http://developer.android.com/guide/topics/ui/themes.html)
as defined by the Android SDK. Device implementations MUST NOT alter any of the
[Holo theme attributes](http://developer.android.com/reference/android/R.style.html)
exposed to applications.

Android includes a “Material” theme family as a set of defined styles for
application developers to use if they want to match the design theme’s look and
feel across the wide variety of different Android device types. Device
implementations MUST support the “Material” theme family and MUST NOT alter any
of the [Material theme attributes](http://developer.android.com/reference/android/R.style.html#Theme_Material)
or their assets exposed to applications.

Android also includes a “Device Default” theme family as a set of defined styles
for application developers to use if they want to match the look and feel of the
device theme as defined by the device implementer. Device implementations MAY
modify the [Device Default theme attributes](http://developer.android.com/reference/android/R.style.html) exposed
to applications.

Android supports a variant theme with translucent system bars, which allows
application developers to fill the area behind the status and navigation bar
with their app content. To enable a consistent developer experience in this
configuration, it is important the status bar icon style is maintained across
different device implementations. Therefore, Android device implementations MUST
use white for system status icons (such as signal strength and battery level)
and notifications issued by the system, unless the icon is indicating a
problematic status or an app requests a light status bar using the
SYSTEM_UI_FLAG_LIGHT_STATUS_BAR flag. When an app requests a light status bar,
Android device implementations MUST change the color of the system status icons
to black (for details, refer to
[R.style](http://developer.android.com/reference/android/R.style.html)).

### 3.8.7\. Live Wallpapers

Android defines a component type and corresponding API and lifecycle that allows
applications to expose one or more
[“Live Wallpapers”](http://developer.android.com/reference/android/service/wallpaper/WallpaperService.html)
to the end user. Live wallpapers are animations, patterns, or similar images
with limited input capabilities that display as a wallpaper, behind other
applications.

Hardware is considered capable of reliably running live wallpapers if it can run
all live wallpapers, with no limitations on functionality, at a reasonable frame
rate with no adverse effects on other applications. If limitations in the
hardware cause wallpapers and/or applications to crash, malfunction, consume
excessive CPU or battery power, or run at unacceptably low frame rates, the
hardware is considered incapable of running live wallpaper. As an example, some
live wallpapers may use an OpenGL 2.0 or 3.x context to render their content.
Live wallpaper will not run reliably on hardware that does not support multiple
OpenGL contexts because the live wallpaper use of an OpenGL context may conflict
with other applications that also use an OpenGL context.

Device implementations capable of running live wallpapers reliably as described
above SHOULD implement live wallpapers, and when implemented MUST report the
platform feature flag android.software.live_wallpaper.

### 3.8.8\. Activity Switching

<div class="note">

As the Recent function navigation key is OPTIONAL, the requirement to implement
the overview screen is OPTIONAL for Android Watch and Android Automotive implementations,
and RECOMMENDED for Android Television devices. There SHOULD still be a
method to switch between activities on Android Automotive implementations.

</div>

The upstream Android source code includes the
[overview screen](http://developer.android.com/guide/components/recents.html), a
system-level user interface for task switching and displaying recently accessed
activities and tasks using a thumbnail image of the application’s graphical
state at the moment the user last left the application. Device implementations
including the recents function navigation key as detailed in
[section 7.2.3](#7_2_3_navigation_keys) MAY alter the interface but MUST meet the
following requirements:

*   MUST support at least up to 20 displayed activities.
*   MUST at least display the title of 4 activities at a time.
*   MUST implement the [screen pinning behavior](http://developer.android.com/about/versions/android-5.0.html#ScreenPinning)
    and provide the user with a settings menu to toggle the feature.
*   SHOULD display highlight color, icon, screen title in recents.
*   SHOULD display a closing affordance ("x") but MAY delay this until user interacts with screens.
*   SHOULD implement a shortcut to switch easily to the previous activity
*   MAY display affiliated recents as a group that moves together.

Device implementations are STRONGLY RECOMMENDED to use the upstream Android user
interface (or a similar thumbnail-based interface) for the overview screen.

### 3.8.9\. Input Management

Android includes support for 
[Input Management](http://developer.android.com/guide/topics/text/creating-input-method.html)
and support for third-party input method editors. Device implementations that
allow users to use third-party input methods on the device MUST declare the
platform feature android.software.input_methods and support IME APIs as defined
in the Android SDK documentation.

Device implementations that declare the android.software.input_methods feature
MUST provide a user-accessible mechanism to add and configure third-party input
methods. Device implementations MUST display the settings interface in response
to the android.settings.INPUT_METHOD_SETTINGS intent.

### 3.8.10\. Lock Screen Media Control

The Remote Control Client API is deprecated from Android 5.0 in favor of the
[Media Notification Template](http://developer.android.com/reference/android/app/Notification.MediaStyle.html)
that allows media applications to integrate with playback controls that are
displayed on the lock screen. Device implementations that support a lock screen,
unless an Android Automotive or Watch implementation, MUST display the
Lock screen Notifications including the Media Notification Template.

### 3.8.11\. Screen savers (previously Dreams)

Android includes support for [interactivescreensavers](http://developer.android.com/reference/android/service/dreams/DreamService.html),
previously referred to as Dreams. Screen savers allow users to interact with
applications when a device connected to a power source is idle or docked in a
desk dock.  Android Watch devices MAY implement screen savers, but other types
of device implementations SHOULD include support for screen savers and provide
a settings option for users toconfigure screen savers in response to the
`android.settings.DREAM_SETTINGS` intent.

### 3.8.12\. Location

When a device has a hardware sensor (e.g. GPS) that is capable of providing the
location coordinates, [location modes](http://developer.android.com/reference/android/provider/Settings.Secure.html#LOCATION_MODE)
MUST be displayed in the Location menu within Settings.

### 3.8.13\. Unicode and Font

Android includes support for the emoji characters defined in
[Unicode 9.0](http://www.unicode.org/versions/Unicode9.0.0/). All device
implementations MUST be capable of rendering these emoji characters
in color glyph and when Android device implementations include an IME,
it SHOULD provide an input method to the user for these emoji characters. 

Android handheld devices SHOULD support the skin tone and diverse family emojis
as specified in the [Unicode Technical Report #51](http://unicode.org/reports/tr51).

Android includes support for Roboto 2 font with different
weights—sans-serif-thin, sans-serif-light, sans-serif-medium, sans-serif-black,
sans-serif-condensed, sans-serif-condensed-light—which MUST all be included for
the languages available on the device and full Unicode 7.0 coverage of Latin,
Greek, and Cyrillic, including the Latin Extended A, B, C, and D ranges, and all
glyphs in the currency symbols block of Unicode 7.0.

### 3.8.14\. Multi-windows

A device implementation MAY choose not to implement any multi-window modes, but
if it has the capability to display multiple activities at the same time it
MUST implement such multi-window mode(s) in accordance with the application
behaviors and APIs described in the Android SDK
[multi-window mode support documentation](https://developer.android.com/preview/features/multi-window.html)
and meet the following requirements:

*   Applications can indicate whether they are capable of operating in
    multi-window mode in the AndroidManifest.xml file, either explicitly via the
    [`android:resizeableActivity`](https://developer.android.com/reference/android/R.attr.html#resizeableActivity)
    attribute or implicitly by having the targetSdkVersion > 24. Apps that
    explicitly set this attribute to false in their manifest MUST not be
    launched in multi-window mode. Apps that don't set the attribute in their
    manifest file (targetSdkVersion < 24) can be launched in multi-window mode,
    but the system MUST provide warning that the app may not work as expected in
    multi-window mode.
*   Device implementations MUST NOT offer split-screen or freeform mode
    if both the screen height and width is less than 440 dp.
*   Device implementations with screen size `xlarge` SHOULD support freeform mode.
*   Android Television device implementations MUST support picture-in-picture (PIP) mode multi-window
    and place the PIP multi-window in the top right corner when PIP is ON.
*   Device implementations with PIP mode multi-window support
    MUST allocate at least 240x135 dp for the PIP window.
*   If the PIP multi-window mode is supported the [`KeyEvent.KEYCODE_WINDOW`](https://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_WINDOW)
    key MUST be used to control the PIP window; otherwise, the key MUST be
    available to the foreground activity.
