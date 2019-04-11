## 8.5\. Consistent Performance

Performance can fluctuate dramatically for high-performance long-running apps,
either because of the other apps running in the background or the CPU throttling
due to temperature limits. Android includes programmatic interfaces so that when
the device is capable, the top foreground application can request that the system
optimize the allocation of the resources to address such fluctuations.

Device implementations SHOULD support Sustained Performance Mode which can
provide the top foreground application a consistent level of performance for a
prolonged amount of time when requested through the
[`Window.setSustainedPerformanceMode()`](https://developer.android.com/reference/android/view/Window.html#setSustainedPerformanceMode%28boolean%29)
API method. A Device implementation MUST report the support of Sustained
Performance Mode accurately through the
[`PowerManager.isSustainedPerformanceModeSupported()`](https://developer.android.com/reference/android/os/PowerManager.html#isSustainedPerformanceModeSupported%28%29)
API method.

Device implementations with two or more CPU cores SHOULD provide at least one exclusive core that
can be reserved by the top foreground application. If provided, implementations MUST meet the
following requirements:

   * Implementations MUST report through the
     [`Process.getExclusiveCores()`](https://developer.android.com/reference/android/os/Process.html#getExclusiveCores%28%29)
     API method the id numbers of the exclusive cores that can be reserved by the top foreground
     application.
   * Device implementations MUST not allow any user space processes except the device drivers used
     by the application to run on the exclusive cores, but MAY allow some kernel processes to run
     as necessary.

If a device implementation does not support an exclusive core, it MUST return an
empty list through the
[`Process.getExclusiveCores()`](https://developer.android.com/reference/android/os/Process.html#getExclusiveCores%28%29)
API method.
