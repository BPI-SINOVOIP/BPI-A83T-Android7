## 3.3\. Native API Compatibility

Native code compatibility is challenging. For this reason, device implementers
are **STRONGLY RECOMMENDED** to use the implementations of the libraries listed
below from the upstream Android Open Source Project.

### 3.3.1\. Application Binary Interfaces

Managed Dalvik bytecode can call into native code provided in the application
.apk file as an ELF .so file compiled for the appropriate device hardware
architecture. As native code is highly dependent on the underlying processor
technology, Android defines a number of Application Binary Interfaces (ABIs) in
the Android NDK. Device implementations MUST be compatible with one or more
defined ABIs, and MUST implement compatibility with the Android NDK, as below.

If a device implementation includes support for an Android ABI, it:

*   MUST include support for code running in the managed environment to call
    into native code, using the standard Java Native Interface (JNI) semantics.
*   MUST be source-compatible (i.e. header compatible) and binary-compatible
    (for the ABI) with each required library in the list below.
*   MUST support the equivalent 32-bit ABI if any 64-bit ABI is supported.
*   MUST accurately report the native Application Binary Interface (ABI)
    supported by the device, via the android.os.Build.SUPPORTED_ABIS,
    android.os.Build.SUPPORTED_32_BIT_ABIS, and
    android.os.Build.SUPPORTED_64_BIT_ABIS parameters, each a comma separated
    list of ABIs ordered from the most to the least preferred one.
*   MUST report, via the above parameters, only those ABIs documented and
    described in the latest version of the [Android NDK ABI Management documentation](https://developer.android.com/ndk/guides/abis.html), and MUST
    include support for the [Advanced SIMD](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0388f/Beijfcja.html)
    (a.k.a. NEON) extension.
*   SHOULD be built using the source code and header files available in the
    upstream Android Open Source Project

Note that future releases of the Android NDK may introduce support for
additional ABIs. If a device implementation is not compatible with an existing
predefined ABI, it MUST NOT report support for any ABIs at all.

The following native code APIs MUST be available to apps that include native code:

*   libandroid.so (native Android activity support)
*   libc (C library)
*   libcamera2ndk.so
*   libdl (dynamic linker)
*   libEGL.so (native OpenGL surface management)
*   libGLESv1\_CM.so (OpenGL ES 1.x)
*   libGLESv2.so (OpenGL ES 2.0)
*   libGLESv3.so (OpenGL ES 3.x)
*   libicui18n.so
*   libicuuc.so
*   libjnigraphics.so
*   liblog (Android logging)
*   libmediandk.so (native media APIs support)
*   libm (math library)
*   libOpenMAXAL.so (OpenMAX AL 1.0.1 support)
*   libOpenSLES.so (OpenSL ES 1.0.1 audio support)
*   libRS.so
*   libstdc++ (Minimal support for C++)
*   libvukan.so (Vulkan)
*   libz (Zlib compression)
*   JNI interface
*   Support for OpenGL, as described below

For the native libraries listed above, the device implementation MUST NOT add
or remove the public functions.

Native libraries not listed above but implemented and provided in AOSP as system
libraries are reserved and MUST NOT be exposed to third-party apps targeting API
level 24 or higher.

Device implementations MAY add non-AOSP libraries and expose them directly as
an API to third-party apps but the additional libraries SHOULD be in
`/vendor/lib` or `/vendor/lib64` and MUST be listed in
`/vendor/etc/public.libraries.txt`.

Note that device implementations MUST include libGLESv3.so and in turn, MUST export
all the OpenGL ES 3.1 and [Android Extension Pack](http://developer.android.com/guide/topics/graphics/opengl.html#aep)
function symbols as defined in the NDK release android-24. Although all the
symbols must be present, only the corresponding functions for OpenGL ES versions
and extensions actually supported by the device must be fully implemented.

#### 3.3.1.1\. Graphic Libraries

[Vulkan](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html)
is a low-overhead, cross-platform API for high-performance 3D graphics. Device
implementations, even if not including support of the Vulkan APIs, MUST satisfy
the following requirements:

*   It MUST always provide a native library named `libvulkan.so` which exports
    function symbols for the core Vulkan 1.0 API as well as the `VK_KHR_surface`,
    `VK_KHR_android_surface`, and `VK_KHR_swapchain` extensions.

Device implementations, if including support of the Vulkan APIs:

*   MUST report, one or more `VkPhysicalDevices` through the
    `vkEnumeratePhysicalDevices` call.
*   Each enumerated `VkPhysicalDevices` MUST fully implement the Vulkan 1.0 API.
*   MUST report the correct
    [`PackageManager#FEATURE_VULKAN_HARDWARE_LEVEL`](https://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_VULKAN_HARDWARE_LEVEL)
    and [`PackageManager#FEATURE_VULKAN_HARDWARE_VERSION`](https://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_VULKAN_HARDWARE_VERSION)
    feature flags.
*   MUST enumerate layers, contained in native libraries named `libVkLayer*.so`
    in the application package’s native library directory, through the
    `vkEnumerateInstanceLayerProperties` and `vkEnumerateDeviceLayerProperties`
    functions in `libvulkan.so`
*   MUST NOT enumerate layers provided by libraries outside of the application
    package, or provide other ways of tracing or intercepting the Vulkan API,
    unless the application has the `android:debuggable=”true”` attribute.

Device implementations, if not including support of the Vulkan APIs:

*   MUST report 0 `VkPhysicalDevices` through the `vkEnumeratePhysicalDevices`
    call.
*   MUST NOT delare any of the Vulkan feature flags
    [`PackageManager#FEATURE_VULKAN_HARDWARE_LEVEL`](https://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_VULKAN_HARDWARE_LEVEL)
    and [`PackageManager#FEATURE_VULKAN_HARDWARE_VERSION`](https://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_VULKAN_HARDWARE_VERSION).


### 3.3.2. 32-bit ARM Native Code Compatibility

The ARMv8 architecture deprecates several CPU operations, including some
operations used in existing native code. On 64-bit ARM devices, the following
deprecated operations MUST remain available to 32-bit native ARM code, either
through native CPU support or through software emulation:

*   SWP and SWPB instructions
*   SETEND instruction
*   CP15ISB, CP15DSB, and CP15DMB barrier operations

Legacy versions of the Android NDK used /proc/cpuinfo to discover CPU features
from 32-bit ARM native code. For compatibility with applications built using
this NDK, devices MUST include the following lines in /proc/cpuinfo when it is
read by 32-bit ARM applications:

*   "Features: ", followed by a list of any optional ARMv7 CPU features supported by the device.
*   "CPU architecture: ", followed by an integer describing the device's highest
    supported ARM architecture (e.g., "8" for ARMv8 devices).

These requirements only apply when /proc/cpuinfo is read by 32-bit ARM
applications. Devices SHOULD not alter /proc/cpuinfo when read by 64-bit ARM or
non-ARM applications.

