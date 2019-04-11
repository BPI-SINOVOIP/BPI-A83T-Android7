## 7.5\. Cameras

Device implementations SHOULD include a rear-facing camera and MAY include a
front-facing camera. A rear-facing camera is a camera located on the side of
the device opposite the display; that is, it images scenes on the far side of
the device, like a traditional camera. A front-facing camera is a camera
located on the same side of the device as the display; that is, a camera
typically used to image the user, such as for video conferencing and similar
applications.

If a device implementation includes at least one camera, it MUST be possible for
an application to simultaneously allocate 3 RGBA_8888 bitmaps equal to the size
of the images produced by the largest-resolution camera sensor on the device,
while camera is open for the purpose of basic preview and still capture.

### 7.5.1\. Rear-Facing Camera

Device implementations SHOULD include a rear-facing camera. If a device
implementation includes at least one rear-facing camera, it:

*   MUST report the feature flag android.hardware.camera and
android.hardware.camera.any.
*   MUST have a resolution of at least 2 megapixels.
*   SHOULD have either hardware auto-focus or software auto-focus implemented
in the camera driver (transparent to application software).
*   MAY have fixed-focus or EDOF (extended depth of field) hardware.
*   MAY include a flash. If the Camera includes a flash, the flash lamp MUST
NOT be lit while an android.hardware.Camera.PreviewCallback instance has been
registered on a Camera preview surface, unless the application has explicitly
enabled the flash by enabling the FLASH_MODE_AUTO or FLASH_MODE_ON attributes
of a Camera.Parameters object. Note that this constraint does not apply to the
device’s built-in system camera application, but only to third-party
applications using Camera.PreviewCallback.

### 7.5.2\. Front-Facing Camera

Device implementations MAY include a front-facing camera. If a device
implementation includes at least one front-facing camera, it:

*   MUST report the feature flag android.hardware.camera.any and
android.hardware.camera.front.
*   MUST have a resolution of at least VGA (640x480 pixels).
*   MUST NOT use a front-facing camera as the default for the Camera API. The
camera API in Android has specific support for front-facing cameras and device
implementations MUST NOT configure the API to to treat a front-facing camera as
the default rear-facing camera, even if it is the only camera on the device.
*   MAY include features (such as auto-focus, flash, etc.) available to
rear-facing cameras as described in [section 7.5.1](#7_5_1_rear-facing_camera).
*   MUST horizontally reflect (i.e. mirror) the stream displayed by an app in a
CameraPreview, as follows:
    *   If the device implementation is capable of being rotated by user (such
as automatically via an accelerometer or manually via user input), the camera
preview MUST be mirrored horizontally relative to the device’s current
orientation.
    *   If the current application has explicitly requested that the Camera
display be rotated via a call to the
[android.hardware.Camera.setDisplayOrientation()](http://developer.android.com/reference/android/hardware/Camera.html#setDisplayOrientation(int))
method, the camera preview MUST be mirrored horizontally relative to the
orientation specified by the application.
    *   Otherwise, the preview MUST be mirrored along the device’s default
horizontal axis.
*   MUST mirror the image displayed by the postview in the same manner as the
camera preview image stream. If the device implementation does not support
postview, this requirement obviously does not apply.
*   MUST NOT mirror the final captured still image or video streams returned to
application callbacks or committed to media storage.

### 7.5.3\. External Camera

Device implementations MAY include support for an external camera that is not
necessarily always connected. If a device includes support for an external camera,
it:


*   MUST declare the platform feature flag `android.hardware.camera.external` and
    `android.hardware camera.any`.
*   MAY support multiple cameras.
*   MUST support USB Video Class (UVC 1.0 or higher) if the external camera
    connects through the USB port.
*   SHOULD support video compressions such as MJPEG to enable transfer of
    high-quality unencoded streams (i.e. raw or independently compressed picture
    streams).
*   MAY support camera-based video encoding. If supported, a simultaneous
    unencoded / MJPEG stream (QVGA or greater resolution) MUST be accessible to
    the device implementation.

### 7.5.4\. Camera API Behavior

Android includes two API packages to access the camera, the newer
android.hardware.camera2 API expose lower-level camera control to the app,
including efficient zero-copy burst/streaming flows and per-frame controls of
exposure, gain, white balance gains, color conversion, denoising, sharpening,
and more.

The older API package, android.hardware.Camera, is marked as deprecated in
Android 5.0 but as it should still be available for apps to use Android device
implementations MUST ensure the continued support of the API as described in
this section and in the Android SDK.

Device implementations MUST implement the following behaviors for the
camera-related APIs, for all available cameras:

*   If an application has never called
android.hardware.Camera.Parameters.setPreviewFormat(int), then the device MUST
use android.hardware.PixelFormat.YCbCr_420_SP for preview data provided to
application callbacks.
*   If an application registers an android.hardware.Camera.PreviewCallback
instance and the system calls the onPreviewFrame() method when the preview
format is YCbCr_420_SP, the data in the byte[] passed into onPreviewFrame()
must further be in the NV21 encoding format. That is, NV21 MUST be the default.
*   For android.hardware.Camera, device implementations MUST support the YV12
format (as denoted by the android.graphics.ImageFormat.YV12 constant) for
camera previews for both front- and rear-facing cameras. (The hardware video
encoder and camera may use any native pixel format, but the device
implementation MUST support conversion to YV12.)
*   For android.hardware.camera2, device implementations must support the
android.hardware.ImageFormat.YUV_420_888 and android.hardware.ImageFormat.JPEG
formats as outputs through the android.media.ImageReader API.

Device implementations MUST still implement the full [Camera
API](http://developer.android.com/reference/android/hardware/Camera.html)
included in the Android SDK documentation, regardless of whether the device
includes hardware autofocus or other capabilities. For instance, cameras that
lack autofocus MUST still call any registered
android.hardware.Camera.AutoFocusCallback instances (even though this has no
relevance to a non-autofocus camera.) Note that this does apply to front-facing
cameras; for instance, even though most front-facing cameras do not support
autofocus, the API callbacks must still be “faked” as described.

Device implementations MUST recognize and honor each parameter name defined as
a constant on the
[android.hardware.Camera.Parameters](http://developer.android.com/reference/android/hardware/Camera.Parameters.html)
class, if the underlying hardware supports the feature. If the device hardware
does not support a feature, the API must behave as documented. Conversely,
device implementations MUST NOT honor or recognize string constants passed to
the android.hardware.Camera.setParameters() method other than those documented
as constants on the android.hardware.Camera.Parameters. That is, device
implementations MUST support all standard Camera parameters if the hardware
allows, and MUST NOT support custom Camera parameter types. For instance,
device implementations that support image capture using high dynamic range
(HDR) imaging techniques MUST support camera parameter Camera.SCENE_MODE_HDR.

Because not all device implementations can fully support all the features of
the android.hardware.camera2 API, device implementations MUST report the proper
level of support with the
[android.info.supportedHardwareLevel](https://developer.android.com/reference/android/hardware/camera2/CameraCharacteristics.html#INFO_SUPPORTED_HARDWARE_LEVEL)
property as described in the Android SDK and report the appropriate
[framework feature flags](http://source.android.com/devices/camera/versioning.html).

Device implementations MUST also declare its Individual camera capabilities of
android.hardware.camera2 via the android.request.availableCapabilities property
and declare the appropriate [feature flags](http://source.android.com/devices/camera/versioning.html);
a device must define the feature flag if any of its attached camera devices
supports the feature.

Device implementations MUST broadcast the Camera.ACTION_NEW_PICTURE intent
whenever a new picture is taken by the camera and the entry of the picture has
been added to the media store.

Device implementations MUST broadcast the Camera.ACTION_NEW_VIDEO intent
whenever a new video is recorded by the camera and the entry of the picture has
been added to the media store.

### 7.5.5\. Camera Orientation

Both front- and rear-facing cameras, if present, MUST be oriented so that the
long dimension of the camera aligns with the screen’s long dimension. That is,
when the device is held in the landscape orientation, cameras MUST capture
images in the landscape orientation. This applies regardless of the device’s
natural orientation; that is, it applies to landscape-primary devices as well
as portrait-primary devices.
