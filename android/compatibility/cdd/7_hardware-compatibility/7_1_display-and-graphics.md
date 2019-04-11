## 7.1\. Display and Graphics

Android includes facilities that automatically adjust application assets and UI
layouts appropriately for the device to ensure that third-party applications
run well on a [variety of hardware configurations](http://developer.android.com/guide/practices/screens_support.html).
Devices MUST properly implement these APIs and behaviors, as detailed in this
section.

The units referenced by the requirements in this section are defined as follows:

*   **physical diagonal size**. The distance in inches between two opposing
corners of the illuminated portion of the display.
*   **dots per inch (dpi)**. The number of pixels encompassed by a linear
horizontal or vertical span of 1”. Where dpi values are listed, both horizontal
and vertical dpi must fall within the range.
*   **aspect ratio**. The ratio of the pixels of the longer dimension to the
shorter dimension of the screen. For example, a display of 480x854 pixels would
be 854/480 = 1.779, or roughly “16:9”.
*   **density-independent pixel (dp)**. The virtual pixel unit normalized to a
160 dpi screen, calculated as: pixels = dps * (density/160).

### 7.1.1\. Screen Configuration

#### 7.1.1.1\. Screen Size

<div class="note">

Android Watch devices (detailed in <a href="#2_device_types">section 2</a>) MAY have
smaller screen sizes as described in this section.

</div>

The Android UI framework supports a variety of different screen sizes, and
allows applications to query the device screen size (aka “screen layout") via
android.content.res.Configuration.screenLayout with the SCREENLAYOUT_SIZE_MASK.
Device implementations MUST report the correct [screen size](http://developer.android.com/guide/practices/screens_support.html) as
defined in the Android SDK documentation and determined by the upstream Android
platform. Specifically, device implementations MUST report the correct screen
size according to the following logical density-independent pixel (dp) screen
dimensions.

*   Devices MUST have screen sizes of at least 426 dp x 320 dp (‘small’),
unless it is an Android Watch device.
*   Devices that report screen size ‘normal’ MUST have screen sizes of at least
480 dp x 320 dp.
*   Devices that report screen size ‘large’ MUST have screen sizes of at least
640 dp x 480 dp.
*   Devices that report screen size ‘xlarge’ MUST have screen sizes of at least
960 dp x 720 dp.

In addition:

*   Android Watch devices MUST have a screen with the physical diagonal size in
the range from 1.1 to 2.5 inches.
*   Android Automotive devices MUST have a screen with the physical diagonal
size greater than or equal to 6 inches.
* Android Automotive devices MUST have a screen size of at least 750 dp x
480 dp.
*   Other types of Android device implementations, with a physically integrated
screen, MUST have a screen at least 2.5 inches in physical diagonal size.

Devices MUST NOT change their reported screen size at any time.

Applications optionally indicate which screen sizes they support via the
&lt;supports-screens&gt; attribute in the AndroidManifest.xml file. Device
implementations MUST correctly honor applications' stated support for small,
normal, large, and xlarge screens, as described in the Android SDK
documentation.

#### 7.1.1.2\. Screen Aspect Ratio

<div class="note">

Android Watch devices MAY have an aspect ratio of 1.0 (1:1).

</div>

The screen aspect ratio MUST be a value from 1.3333 (4:3) to 1.86 (roughly
16:9), but Android Watch devices MAY have an aspect ratio of 1.0 (1:1) because
such a device implementation will use a UI_MODE_TYPE_WATCH as the
android.content.res.Configuration.uiMode.

#### 7.1.1.3\. Screen Density

The Android UI framework defines a set of standard logical densities to help
application developers target application resources. Device implementations
MUST report only one of the following logical Android framework densities
through the android.util.DisplayMetrics APIs, and MUST execute applications at
this standard density and MUST NOT change the value at at any time for the
default display.

*   120 dpi (ldpi)
*   160 dpi (mdpi)
*   213 dpi (tvdpi)
*   240 dpi (hdpi)
*   280 dpi (280dpi)
*   320 dpi (xhdpi)
*   360 dpi (360dpi)
*   400 dpi (400dpi)
*   420 dpi (420dpi)
*   480 dpi (xxhdpi)
*   560 dpi (560dpi)
*   640 dpi (xxxhdpi)

Device implementations SHOULD define the standard Android framework density
that is numerically closest to the physical density of the screen, unless that
logical density pushes the reported screen size below the minimum supported. If
the standard Android framework density that is numerically closest to the
physical density results in a screen size that is smaller than the smallest
supported compatible screen size (320 dp width), device implementations SHOULD
report the next lowest standard Android framework density.

Device implementations are STRONGLY RECOMMENDED to provide users a setting to change
the display size. If there is an implementation to change the display size of the device,
it MUST align with the AOSP implementation as indicated below:

*  The display size MUST NOT be scaled any larger than 1.5 times the native density or
   produce an effective minimum screen dimension smaller than 320dp (equivalent
   to resource qualifier sw320dp), whichever comes first.
*  Display size MUST NOT be scaled any smaller than 0.85 times the native density.
*  To ensure good usability and consistent font sizes, it is RECOMMENDED that the
   following scaling of Native Display options be provided (while complying with the limits
   specified above)
   *  Small: 0.85x
   *  Default: 1x (Native display scale)
   *  Large: 1.15x
   *  Larger: 1.3x
   *  Largest 1.45x

### 7.1.2\. Display Metrics

Device implementations MUST report correct values for all display metrics
defined in
[android.util.DisplayMetrics](http://developer.android.com/reference/android/util/DisplayMetrics.html)
and MUST report the same values regardless of whether the embedded or external
screen is used as the default display.

### 7.1.3\. Screen Orientation

Devices MUST report which screen orientations they support
(android.hardware.screen.portrait and/or android.hardware.screen.landscape) and
MUST report at least one supported orientation. For example, a device with a
fixed orientation landscape screen, such as a television or laptop, SHOULD only
report android.hardware.screen.landscape.

Devices that report both screen orientations MUST support dynamic orientation
by applications to either portrait or landscape screen orientation. That is,
the device must respect the application’s request for a specific screen
orientation. Device implementations MAY select either portrait or landscape
orientation as the default.

Devices MUST report the correct value for the device’s current orientation,
whenever queried via the android.content.res.Configuration.orientation,
android.view.Display.getOrientation(), or other APIs.

Devices MUST NOT change the reported screen size or density when changing orientation.

### 7.1.4\. 2D and 3D Graphics Acceleration

Device implementations MUST support both OpenGL ES 1.0 and 2.0, as embodied and
detailed in the Android SDK documentations. Device implementations SHOULD
support OpenGL ES 3.0, 3.1, or 3.2 on devices capable of supporting it. Device
implementations MUST also support [Android RenderScript](http://developer.android.com/guide/topics/renderscript/),
as detailed in the Android SDK documentation.

Device implementations MUST also correctly identify themselves as supporting
OpenGL ES 1.0, OpenGL ES 2.0, OpenGL ES 3.0, OpenGL 3.1, or OpenGL 3.2\. That is:

*   The managed APIs (such as via the GLES10.getString() method) MUST report
support for OpenGL ES 1.0 and OpenGL ES 2.0.
*   The native C/C++ OpenGL APIs (APIs available to apps via libGLES_v1CM.so,
libGLES_v2.so, or libEGL.so) MUST report support for OpenGL ES 1.0 and OpenGL
ES 2.0.
*   Device implementations that declare support for OpenGL ES 3.0, 3.1, or 3.2 MUST
support the corresponding managed APIs and include support for native C/C++
APIs. On device implementations that declare support for OpenGL ES 3.0, 3.1, or
3.2 libGLESv2.so MUST export the corresponding function symbols in addition to
the OpenGL ES 2.0 function symbols.

Android provides an OpenGL ES [extension pack](https://developer.android.com/reference/android/opengl/GLES31Ext.html)
with Java interfaces and native support for advanced graphics functionality
such as tessellation and the ASTC texture compression format. Android device
implementations MUST support the extension pack if the device supports OpenGL
ES 3.2 and MAY support it otherwise. If the extension pack is supported in its
entirety, the device MUST identify the support through the
`android.hardware.opengles.aep` feature flag.

Also, device implementations MAY implement any desired OpenGL ES extensions.
However, device implementations MUST report via the OpenGL ES managed and
native APIs all extension strings that they do support, and conversely MUST NOT
report extension strings that they do not support.

Note that Android includes support for applications to optionally specify that
they require specific OpenGL texture compression formats. These formats are
typically vendor-specific. Device implementations are not required by Android
to implement any specific texture compression format. However, they SHOULD
accurately report any texture compression formats that they do support, via the
getString() method in the OpenGL API.

Android includes a mechanism for applications to declare that they want to
enable hardware acceleration for 2D graphics at the Application, Activity,
Window, or View level through the use of a manifest tag
[android:hardwareAccelerated](http://developer.android.com/guide/topics/graphics/hardware-accel.html)
or direct API calls.

Device implementations MUST enable hardware acceleration by default, and MUST
disable hardware acceleration if the developer so requests by setting
android:hardwareAccelerated="false” or disabling hardware acceleration directly
through the Android View APIs.

In addition, device implementations MUST exhibit behavior consistent with the
Android SDK documentation on [hardware
acceleration](http://developer.android.com/guide/topics/graphics/hardware-accel.html).

Android includes a TextureView object that lets developers directly integrate
hardware-accelerated OpenGL ES textures as rendering targets in a UI hierarchy.
Device implementations MUST support the TextureView API, and MUST exhibit
consistent behavior with the upstream Android implementation.

Android includes support for EGL_ANDROID_RECORDABLE, an EGLConfig attribute
that indicates whether the EGLConfig supports rendering to an ANativeWindow
that records images to a video. Device implementations MUST support
[EGL_ANDROID_RECORDABLE](https://www.khronos.org/registry/egl/extensions/ANDROID/EGL_ANDROID_recordable.txt)
extension.

### 7.1.5\. Legacy Application Compatibility Mode

Android specifies a “compatibility mode” in which the framework operates in a
'normal' screen size equivalent (320dp width) mode for the benefit of legacy
applications not developed for old versions of Android that pre-date
screen-size independence.

*   Android Automotive does not support legacy compatibility mode.
*   All other device implementations MUST include support for legacy
application compatibility mode as implemented by the upstream Android open
source code. That is, device implementations MUST NOT alter the triggers or
thresholds at which compatibility mode is activated, and MUST NOT alter the
behavior of the compatibility mode itself.

### 7.1.6\. Screen Technology

The Android platform includes APIs that allow applications to render rich
graphics to the display. Devices MUST support all of these APIs as defined by
the Android SDK unless specifically allowed in this document.

*   Devices MUST support displays capable of rendering 16-bit color graphics
and SHOULD support displays capable of 24-bit color graphics.
*   Devices MUST support displays capable of rendering animations.
*   The display technology used MUST have a pixel aspect ratio (PAR) between
0.9 and 1.15\. That is, the pixel aspect ratio MUST be near square (1.0) with a
10 ~ 15% tolerance.

### 7.1.7\. Secondary Displays

Android includes support for secondary display to enable media sharing
capabilities and developer APIs for accessing external displays. If a device
supports an external display either via a wired, wireless, or an embedded
additional display connection then the device implementation MUST implement the
[display manager
API](http://developer.android.com/reference/android/hardware/display/DisplayManager.html)
as described in the Android SDK documentation.
