## 3.5\. API Behavioral Compatibility

The behaviors of each of the API types (managed, soft, native, and web) must be
consistent with the preferred implementation of the upstream
[Android Open Source Project](http://source.android.com/). Some specific areas of
compatibility are:

*   Devices MUST NOT change the behavior or semantics of a standard intent.
*   Devices MUST NOT alter the lifecycle or lifecycle semantics of a particular
    type of system component (such as Service, Activity, ContentProvider, etc.).
*   Devices MUST NOT change the semantics of a standard permission.

The above list is not comprehensive. The Compatibility Test Suite (CTS) tests
significant portions of the platform for behavioral compatibility, but not all.
It is the responsibility of the implementer to ensure behavioral compatibility
with the Android Open Source Project. For this reason, device implementers
SHOULD use the source code available via the Android Open Source Project where
possible, rather than re-implement significant parts of the system.
