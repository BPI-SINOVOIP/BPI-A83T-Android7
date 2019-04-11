## 3.9\. Device Administration

Android includes features that allow security-aware applications to perform
device administration functions at the system level, such as enforcing password
policies or performing remote wipe, through the
[Android Device Administration API](http://developer.android.com/guide/topics/admin/device-admin.html)].
Device implementations MUST provide an implementation of the
[DevicePolicyManager](http://developer.android.com/reference/android/app/admin/DevicePolicyManager.html)
class. Device implementations that supports a secure lock screen MUST implement
the full range of
[device administration](http://developer.android.com/guide/topics/admin/device-admin.html)
policies defined in the Android SDK documentation and report the platform
feature android.software.device_admin.

### 3.9.1 Device Provisioning

#### 3.9.1.1 Device owner provisioning

If a device implementation declares the `android.software.device_admin` feature
then it MUST implement the provisioning of the [Device Owner app](http://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#isDeviceOwnerApp(java.lang.String))
of a Device Policy Client (DPC) application as indicated below:

*   When the device implementation has no user data configured yet, it:
    *    MUST report `true` for [`DevicePolicyManager.isProvisioningAllowed(ACTION_PROVISION_MANAGED_DEVICE)`](https://developer.android.com/reference/android/app/admin/DevicePolicyManager.html\#isProvisioningAllowed\(java.lang.String\)).
    *    MUST enroll the DPC application as the Device Owner app in response to
         the intent action [`android.app.action.PROVISION_MANAGED_DEVICE`](http://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#ACTION_PROVISION_MANAGED_DEVICE).
    *    MUST enroll the DPC application as the Device Owner app if the device
         declares Near-Field Communications (NFC) support via the feature flag
         `android.hardware.nfc` and receives an NFC message containing a record
         with MIME type [`MIME_TYPE_PROVISIONING_NFC`](https://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#MIME_TYPE_PROVISIONING_NFC).
*   When the device implementation has user data, it:
    *    MUST report `false` for the [`DevicePolicyManager.isProvisioningAllowed(ACTION_PROVISION_MANAGED_DEVICE)`](https://developer.android.com/reference/android/app/admin/DevicePolicyManager.html\#isProvisioningAllowed\(java.lang.String\)).
    *    MUST not enroll any DPC application as the Device Owner App any more.

Device implementations MAY have a preinstalled application performing device
administration functions but this application MUST NOT be set as the Device
Owner app without explicit consent or action from the user or the administrator
of the device.

#### 3.9.1.2 Managed profile provisioning

If a device implementation declares the android.software.managed_users, it MUST
be possible to enroll a Device Policy Controller (DPC) application as the
[owner of a new Managed Profile](http://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#isProfileOwnerApp(java.lang.String)).

The managed profile provisioning process (the flow initiated by
[android.app.action.PROVISION_MANAGED_PROFILE](http://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#ACTION_PROVISION_MANAGED_PROFILE))
user experience MUST align with the AOSP implementation.

Device implementations MUST provide the following user affordances within the
Settings user interface to indicate to the user when a particular system function
has been disabled by the Device Policy Controller (DPC):

*    A consistent icon or other user affordance (for example the upstream AOSP
     info icon) to represent when a particular setting is restricted by a
     Device Admin.
*    A short explanation message, as provided by the Device Admin via the
     [`setShortSupportMessage`](https://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#setShortSupportMessage%28android.content.ComponentName, java.lang.CharSequence%29).
*    The DPC application’s icon.

## 3.9.2 Managed Profile Support

Managed profile capable devices are those devices that:

*   Declare android.software.device_admin (see [section 3.9 Device Administration](#3_9_device_administration)).
*   Are not low RAM devices (see [section 7.6.1](#7_6_1_minimum_memory_and_storage)).
*   Allocate internal (non-removable) storage as shared storage (see [section 7.6.2](#7_6_2_application_shared_storage)).

Managed profile capable devices MUST:

*   Declare the platform feature flag `android.software.managed_users`.
*   Support managed profiles via the `android.app.admin.DevicePolicyManager` APIs.
*   Allow one and only [one managed profile to be created](http://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#ACTION_PROVISION_MANAGED_PROFILE).
*   Use an icon badge (similar to the AOSP upstream work badge) to represent the
    managed applications and widgets and other badged UI elements like
    Recents &amp; Notifications.
*   Display a notification icon (similar to the AOSP upstream work badge) to
    indicate when user is within a managed profile application.
*   Display a toast indicating that the user is in the managed profile if and
    when the device wakes up (ACTION_USER_PRESENT) and the foreground
    application is within the managed profile.
*   Where a managed profile exists, show a visual affordance in the Intent
    'Chooser' to allow the user to forward the intent from the managed profile
    to the primary user or vice versa, if enabled by the Device Policy
    Controller.
*   Where a managed profile exists, expose the following user affordances for
    both the primary user and the managed profile:
    *   Separate accounting for battery, location, mobile data and storage usage
        for the primary user and managed profile.
    *   Independent management of VPN Applications installed within the primary
        user or managed profile.
    *   Independent management of applications installed within the primary user
        or managed profile.
    *   Independent management of accounts within the primary user or managed
        profile.
*   Ensure the preinstalled dialer, contacts and messaging applications can
    search for and look up caller information from the managed profile (if one
    exists) alongside those from the primary profile, if the Device Policy
    Controller permits it. When contacts from the managed profile are displayed
    in the preinstalled call log, in-call UI, in-progress and missed-call
    notifications, contacts and messaging apps they SHOULD be badged with the
    same badge used to indicate managed profile applications.
*   MUST ensure that it satisfies all the security requirements applicable for a
    device with multiple users enabled (see[section 9.5](#9_5_multi-user_support)),
    even though the managed profile is not counted as another user in addition
    to the primary user.
*   Support the ability to specify a separate lock screen meeting the following
    requirements to grant access to apps running in a managed profile.
    *   Device implementations MUST honor the
        [`DevicePolicyManager.ACTION_SET_NEW_PASSWORD`](https://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#ACTION_SET_NEW_PASSWORD)
        intent and show an interface to configure a separate lock screen
        credential for the managed profile.
    *   The lock screen credentials of the managed profile MUST use the same
        credential storage and management mechanisms as the parent profile,
        as documented on the
        [Android Open Source Project Site](http://source.android.com/security/authentication/index.html)
    *   The DPC [password policies](https://developer.android.com/guide/topics/admin/device-admin.html#pwd)
        MUST apply to only the managed profile's lock screen credentials unless
        called upon the `DevicePolicyManager` instance returned by
        <a href="https://developer.android.com/reference/android/app/admin/DevicePolicyManager.html#getParentProfileInstance%28android.content.ComponentName%29">getParentProfileInstance</a>.

