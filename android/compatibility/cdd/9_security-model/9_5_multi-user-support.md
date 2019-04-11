## 9.5\. Multi-User Support

<div class="note">

This feature is optional for all device types.

</div>

Android includes [support for multiple users](http://developer.android.com/reference/android/os/UserManager.html) and
provides support for full user isolation. Device implementations MAY enable
multiple users, but when enabled MUST meet the following requirements related
to [multi-user support](http://source.android.com/devices/storage/traditional.html):

*   Android Automotive device implementations with multi-user support enabled
MUST include a guest account that allows all functions provided by the vehicle
system without requiring a user to log in.
*   Device implementations that do not declare the android.hardware.telephony
feature flag MUST support restricted profiles, a feature that allows device
owners to manage additional users and their capabilities on the device. With
restricted profiles, device owners can quickly set up separate environments for
additional users to work in, with the ability to manage finer-grained
restrictions in the apps that are available in those environments.
*   Conversely device implementations that declare the
android.hardware.telephony feature flag MUST NOT support restricted profiles
but MUST align with the AOSP implementation of controls to enable /disable
other users from accessing the voice calls and SMS.
*   Device implementations MUST, for each user, implement a security model
consistent with the Android platform security model as defined in
[Security and Permissions reference document](http://developer.android.com/guide/topics/security/permissions.html)
in the APIs.
*   Each user instance on an Android device MUST have separate and isolated
external storage directories. Device implementations MAY store multiple users'
data on the same volume or filesystem. However, the device implementation MUST
ensure that applications owned by and running on behalf a given user cannot
list, read, or write to data owned by any other user. Note that removable
media, such as SD card slots, can allow one user to access another’s data by
means of a host PC. For this reason, device implementations that use removable
media for the external storage APIs MUST encrypt the contents of the SD card if
multiuser is enabled using a key stored only on non-removable media accessible
only to the system. As this will make the media unreadable by a host PC, device
implementations will be required to switch to MTP or a similar system to
provide host PCs with access to the current user’s data. Accordingly, device
implementations MAY but SHOULD NOT enable multi-user if they use
[removable media](http://developer.android.com/reference/android/os/Environment.html) for
primary external storage.
