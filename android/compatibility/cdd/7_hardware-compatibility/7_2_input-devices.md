## 7.2\. Input Devices

Devices MUST support a touchscreen or meet the requirements listed in 7.2.2 for
non-touch navigation.

### 7.2.1\. Keyboard

<div class="note">

Android Watch and Android Automotive implementations MAY implement a soft
keyboard. All other device implementations MUST implement a soft keyboard and:

</div>

Device implementations:

*   MUST include support for the Input Management Framework (which allows
third-party developers to create Input Method Editors—i.e. soft keyboard) as
detailed at [http://developer.android.com](http://developer.android.com).
*   MUST provide at least one soft keyboard implementation (regardless of
whether a hard keyboard is present) except for Android Watch devices where the
screen size makes it less reasonable to have a soft keyboard.
*   MAY include additional soft keyboard implementations.
*   MAY include a hardware keyboard.
*   MUST NOT include a hardware keyboard that does not match one of the formats
specified in
[android.content.res.Configuration.keyboard](http://developer.android.com/reference/android/content/res/Configuration.html)
(QWERTY or 12-key).

### 7.2.2\. Non-touch Navigation

<div class="note">

Android Television devices MUST support D-pad.

</div>

Device implementations:

*   MAY omit a non-touch navigation option (trackball, d-pad, or wheel) if the
device implementation is not an Android Television device.
*   MUST report the correct value for
[android.content.res.Configuration.navigation](http://developer.android.com/reference/android/content/res/Configuration.html).
*   MUST provide a reasonable alternative user interface mechanism for the
selection and editing of text, compatible with Input Management Engines. The
upstream Android open source implementation includes a selection mechanism
suitable for use with devices that lack non-touch navigation inputs.

### 7.2.3\. Navigation Keys

<div class="note">

The availability and visibility requirement of the Home, Recents, and Back
functions differ between device types as described in this section.

</div>

The Home, Recents, and Back functions (mapped to the key events KEYCODE_HOME,
KEYCODE_APP_SWITCH, KEYCODE_BACK, respectively) are essential to the Android
navigation paradigm and therefore:

*   Android Handheld device implementations MUST provide the Home, Recents, and
    Back functions.
*   Android Television device implementations MUST provide the Home and Back
    functions.
*   Android Watch device implementations MUST have the Home function available
    to the user, and the Back function except for when it is in `UI_MODE_TYPE_WATCH`.
*   Android Watch device implementations, and no other Android device types,
    MAY consume the long press event on the key event [`KEYCODE_BACK`](http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BACK)
    and omit it from being sent to the foreground application.
*   Android Automotive implementations MUST provide the Home function and MAY
    provide Back and Recent functions.
*   All other types of device implementations MUST provide the Home and Back
    functions.

These functions MAY be implemented via dedicated physical buttons (such as
mechanical or capacitive touch buttons), or MAY be implemented using dedicated
software keys on a distinct portion of the screen, gestures, touch panel, etc.
Android supports both implementations. All of these functions MUST be accessible
with a single action (e.g. tap, double-click or gesture) when visible.

Recents function, if provided, MUST have a visible button or icon unless hidden
together with other navigation functions in full-screen mode. This does not
apply to devices upgrading from earlier Android versions that have physical
buttons for navigation and no recents key.

The Home and Back functions, if provided, MUST each have a visible button or
icon unless hidden together with other navigation functions in full-screen mode
or when the uiMode UI_MODE_TYPE_MASK is set to UI_MODE_TYPE_WATCH.

The Menu function is deprecated in favor of action bar since Android 4.0.
Therefore the new device implementations shipping with Android ANDROID_VERSION
and later MUST NOT implement a dedicated physical button for the Menu function.
Older device implementations SHOULD NOT implement a dedicated physical button
for the Menu function, but if the physical Menu button is implemented and the
device is running applications with targetSdkVersion > 10, the device
implementation:

*   MUST display the action overflow button on the action bar when it is visible
and the resulting action overflow menu popup is not empty. For a device
implementation launched before Android 4.4 but upgrading to Android
ANDROID_VERSION, this is RECOMMENDED.
*   MUST NOT modify the position of the action overflow popup displayed by
selecting the overflow button in the action bar.
*   MAY render the action overflow popup at a modified position on the screen
when it is displayed by selecting the physical menu button.

For backwards compatibility, device implementations MUST make the Menu function
available to applications when targetSdkVersion is less than 10, either by a
physical button, a software key, or gestures. This Menu function should be
presented unless hidden together with other navigation functions.

Android device implementations supporting the [Assist action](http://developer.android.com/reference/android/content/Intent.html#ACTION_ASSIST)
and/or [`VoiceInteractionService`](https://developer.android.com/reference/android/service/voice/VoiceInteractionService.html)
MUST be able to launch an assist app with a single interaction (e.g. tap,
double-click, or gesture) when other navigation keys are visible. It is STRONGLY
RECOMMENDED to use long press on home as this interaction. The designated
interaction MUST launch the user-selected assist app, in other words the app
that implements a VoiceInteractionService, or an activity handling the ACTION_ASSIST intent.

Device implementations MAY use a distinct portion of the screen to display the
navigation keys, but if so, MUST meet these requirements:

*   Device implementation navigation keys MUST use a distinct portion of the
screen, not available to applications, and MUST NOT obscure or otherwise
interfere with the portion of the screen available to applications.
*   Device implementations MUST make available a portion of the display to
applications that meets the requirements defined in [section
7.1.1](#7_1_1_screen_configuration).
*   Device implementations MUST display the navigation keys when applications do
not specify a system UI mode, or specify SYSTEM_UI_FLAG_VISIBLE.
*   Device implementations MUST present the navigation keys in an unobtrusive
“low profile” (eg. dimmed) mode when applications specify
SYSTEM_UI_FLAG_LOW_PROFILE.
*   Device implementations MUST hide the navigation keys when applications
specify SYSTEM_UI_FLAG_HIDE_NAVIGATION.

### 7.2.4\. Touchscreen Input

<div class="note">

Android Handhelds and Watch Devices MUST support touchscreen input.

</div>

Device implementations SHOULD have a pointer input system of some kind (either
mouse-like or touch). However, if a device implementation does not support a
pointer input system, it MUST NOT report the android.hardware.touchscreen or
android.hardware.faketouch feature constant. Device implementations that do
include a pointer input system:

*   SHOULD support fully independently tracked pointers, if the device input
system supports multiple pointers.
*   MUST report the value of
[android.content.res.Configuration.touchscreen](http://developer.android.com/reference/android/content/res/Configuration.html)
corresponding to the type of the specific touchscreen on the device.

Android includes support for a variety of touchscreens, touch pads, and fake
touch input devices. [Touchscreen-based device implementations](http://source.android.com/devices/tech/input/touch-devices.html)
are associated with a display such that the user has the impression of directly
manipulating items on screen. Since the user is directly touching the screen,
the system does not require any additional affordances to indicate the objects
being manipulated. In contrast, a fake touch interface provides a user input
system that approximates a subset of touchscreen capabilities. For example, a
mouse or remote control that drives an on-screen cursor approximates touch, but
requires the user to first point or focus then click. Numerous input devices
like the mouse, trackpad, gyro-based air mouse, gyro-pointer, joystick, and
multi-touch trackpad can support fake touch interactions. Android includes the
feature constant android.hardware.faketouch, which corresponds to a
high-fidelity non-touch (pointer-based) input device such as a mouse or trackpad
that can adequately emulate touch-based input (including basic gesture support),
and indicates that the device supports an emulated subset of touchscreen
functionality. Device implementations that declare the fake touch feature MUST
meet the fake touch requirements in [section 7.2.5](#7_2_5_fake_touch_input).

Device implementations MUST report the correct feature corresponding to the type
of input used. Device implementations that include a touchscreen (single-touch
or better) MUST report the platform feature constant
android.hardware.touchscreen. Device implementations that report the platform
feature constant android.hardware.touchscreen MUST also report the platform
feature constant android.hardware.faketouch. Device implementations that do not
include a touchscreen (and rely on a pointer device only) MUST NOT report any
touchscreen feature, and MUST report only android.hardware.faketouch if they
meet the fake touch requirements in [section 7.2.5](#7_2_5_fake_touch_input).

### 7.2.5\. Fake Touch Input

Device implementations that declare support for android.hardware.faketouch:

*   MUST report the [absolute X and Y screen positions](http://developer.android.com/reference/android/view/MotionEvent.html)
of the pointer location and display a visual pointer on the screen.
*   MUST report touch event with the action code that specifies the state change
that occurs on the pointer [going down or up on the
screen](http://developer.android.com/reference/android/view/MotionEvent.html).
*   MUST support pointer down and up on an object on the screen, which allows
users to emulate tap on an object on the screen.
*   MUST support pointer down, pointer up, pointer down then pointer up in the
same place on an object on the screen within a time threshold, which allows
users to [emulate double tap](http://developer.android.com/reference/android/view/MotionEvent.html)
on an object on the screen.
*   MUST support pointer down on an arbitrary point on the screen, pointer move
to any other arbitrary point on the screen, followed by a pointer up, which
allows users to emulate a touch drag.
*   MUST support pointer down then allow users to quickly move the object to a
different position on the screen and then pointer up on the screen, which allows
users to fling an object on the screen.

Devices that declare support for android.hardware.faketouch.multitouch.distinct
MUST meet the requirements for faketouch above, and MUST also support distinct
tracking of two or more independent pointer inputs.

### 7.2.6\. Game Controller Support

Android Television device implementations MUST support button mappings for game
controllers as listed below. The upstream Android implementation includes
implementation for game controllers that satisfies this requirement.

#### 7.2.6.1\. Button Mappings

Android Television device implementations MUST support the following key mappings:

<table>
 <tr>
    <th>Button</th>
    <th>HID Usage<sup>2</sup></th>
    <th>Android Button</th>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BUTTON_A">A</a><sup>1</sup></td>
    <td>0x09 0x0001</td>
    <td>KEYCODE_BUTTON_A (96)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BUTTON_B">B</a><sup>1</sup></td>
    <td>0x09 0x0002</td>
    <td>KEYCODE_BUTTON_B (97)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BUTTON_X">X</a><sup>1</sup></td>
    <td>0x09 0x0004</td>
    <td>KEYCODE_BUTTON_X (99)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BUTTON_Y">Y</a><sup>1</sup></td>
    <td>0x09 0x0005</td>
    <td>KEYCODE_BUTTON_Y (100)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_DPAD_UP">D-pad up</a><sup>1</sup><br />

<a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_DPAD_DOWN">D-pad down</a><sup>1</sup></td>
    <td>0x01 0x0039<sup>3</sup></td>
    <td><a href="http://developer.android.com/reference/android/view/MotionEvent.html#AXIS_HAT_Y">AXIS_HAT_Y</a><sup>4</sup></td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_DPAD_LEFT">D-pad left</a>1<br />

<a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_DPAD_RIGHT">D-pad right</a><sup>1</sup></td>
    <td>0x01 0x0039<sup>3</sup></td>
    <td><a href="http://developer.android.com/reference/android/view/MotionEvent.html#AXIS_HAT_X">AXIS_HAT_X</a><sup>4</sup></td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BUTTON_L1">Left shoulder button</a><sup>1</sup></td>
    <td>0x09 0x0007</td>
    <td>KEYCODE_BUTTON_L1 (102)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BUTTON_R1">Right shoulder button</a><sup>1</sup></td>
    <td>0x09 0x0008</td>
    <td>KEYCODE_BUTTON_R1 (103)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BUTTON_THUMBL">Left stick click</a><sup>1</sup></td>
    <td>0x09 0x000E</td>
    <td>KEYCODE_BUTTON_THUMBL (106)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BUTTON_THUMBR">Right stick click</a><sup>1</sup></td>
    <td>0x09 0x000F</td>
    <td>KEYCODE_BUTTON_THUMBR (107)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_HOME">Home</a><sup>1</sup></td>
    <td>0x0c 0x0223</td>
    <td>KEYCODE_HOME (3)</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_BACK">Back</a><sup>1</sup></td>
    <td>0x0c 0x0224</td>
    <td>KEYCODE_BACK (4)</td>
 </tr>
</table>


<p class="table_footnote">1 <a href="http://developer.android.com/reference/android/view/KeyEvent.html">KeyEvent</a></p>

<p class="table_footnote">2 The above HID usages must be declared within a Game
pad CA (0x01 0x0005).</p>

<p class="table_footnote">3 This usage must have a Logical Minimum of 0, a
Logical Maximum of 7, a Physical Minimum of 0, a Physical Maximum of 315, Units
in Degrees, and a Report Size of 4. The logical value is defined to be the
clockwise rotation away from the vertical axis; for example, a logical value of
0 represents no rotation and the up button being pressed, while a logical value
of 1 represents a rotation of 45 degrees and both the up and left keys being
pressed.</p>

<p class="table_footnote">4 <a
href="http://developer.android.com/reference/android/view/MotionEvent.html">MotionEvent</a></p>

<table>
 <tr>
    <th>Analog Controls<sup>1</sup></th>
    <th>HID Usage</th>
    <th>Android Button</th>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/MotionEvent.html#AXIS_LTRIGGER">Left Trigger</a></td>
    <td>0x02 0x00C5</td>
    <td>AXIS_LTRIGGER </td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/MotionEvent.html#AXIS_THROTTLE">Right Trigger</a></td>
    <td>0x02 0x00C4</td>
    <td>AXIS_RTRIGGER </td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/MotionEvent.html#AXIS_Y">Left Joystick</a></td>
    <td>0x01 0x0030<br />

0x01 0x0031</td>
    <td>AXIS_X<br />

AXIS_Y</td>
 </tr>
 <tr>
    <td><a href="http://developer.android.com/reference/android/view/MotionEvent.html#AXIS_Z">Right Joystick</a></td>
    <td>0x01 0x0032<br />

0x01 0x0035</td>
    <td>AXIS_Z<br />

AXIS_RZ</td>
 </tr>
</table>


<p class="table_footnote">1 <a
href="http://developer.android.com/reference/android/view/MotionEvent.html">MotionEvent</a></p>

### 7.2.7\. Remote Control

Android Television device implementations SHOULD provide a remote control to
allow users to access the TV interface. The remote control MAY be a physical
remote or can be a software-based remote that is accessible from a mobile phone
or tablet. The remote control MUST meet the requirements defined below.

*   **Search affordance**. Device implementations MUST fire KEYCODE_SEARCH when
the user invokes voice search either on the physical or software-based remote.
*   **Navigation**. All Android Television remotes MUST include
[Back, Home, and Select buttons and support for D-pad events](http://developer.android.com/reference/android/view/KeyEvent.html).
