## 9.4\. Alternate Execution Environments

Device implementations MAY include runtime environments that execute
applications using some other software or technology than the Dalvik Executable
Format or native code. However, such alternate execution environments MUST NOT
compromise the Android security model or the security of installed Android
applications, as described in this section.

Alternate runtimes MUST themselves be Android applications, and abide by the
standard Android security model, as described elsewhere in
[section 9](#9_security_model_compatibility).

Alternate runtimes MUST NOT be granted access to resources protected by
permissions not requested in the runtime’s AndroidManifest.xml file via the
&lt;uses-permission&gt; mechanism.

Alternate runtimes MUST NOT permit applications to make use of features
protected by Android permissions restricted to system applications.

Alternate runtimes MUST abide by the Android sandbox model. Specifically,
alternate runtimes:

*   SHOULD install apps via the PackageManager into separate Android sandboxes
(Linux user IDs, etc.).
*   MAY provide a single Android sandbox shared by all applications using the
alternate runtime.
*   Installed applications using an alternate runtime MUST NOT reuse the
sandbox of any other app installed on the device, except through the standard
Android mechanisms of shared user ID and signing certificate.
*   MUST NOT launch with, grant, or be granted access to the sandboxes
corresponding to other Android applications.
*   MUST NOT be launched with, be granted, or grant to other applications any
privileges of the superuser (root), or of any other user ID.

The .apk files of alternate runtimes MAY be included in the system image of a
device implementation, but MUST be signed with a key distinct from the key used
to sign other applications included with the device implementation.

When installing applications, alternate runtimes MUST obtain user consent for
the Android permissions used by the application. If an application needs to
make use of a device resource for which there is a corresponding Android
permission (such as Camera, GPS, etc.), the alternate runtime MUST inform the
user that the application will be able to access that resource. If the runtime
environment does not record application capabilities in this manner, the
runtime environment MUST list all permissions held by the runtime itself when
installing any application using that runtime.

