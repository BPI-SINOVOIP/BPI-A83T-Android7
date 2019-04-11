## 9.2\. UID and Process Isolation

Device implementations MUST support the Android application sandbox model, in
which each application runs as a unique Unixstyle UID and in a separate
process. Device implementations MUST support running multiple applications as
the same Linux user ID, provided that the applications are properly signed and
constructed, as defined in the
[Security and Permissions reference](http://developer.android.com/guide/topics/security/permissions.html).
