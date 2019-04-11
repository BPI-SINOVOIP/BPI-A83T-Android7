# 7\. Hardware Compatibility

If a device includes a particular hardware component that has a corresponding
API for third-party developers, the device implementation MUST implement that
API as described in the Android SDK documentation. If an API in the SDK
interacts with a hardware component that is stated to be optional and the
device implementation does not possess that component:

*   Complete class definitions (as documented by the SDK) for the component
APIs MUST still be presented.
*   The API’s behaviors MUST be implemented as no-ops in some reasonable
fashion.
*   API methods MUST return null values where permitted by the SDK
documentation.
*   API methods MUST return no-op implementations of classes where null values
are not permitted by the SDK documentation.
*   API methods MUST NOT throw exceptions not documented by the SDK
documentation.

A typical example of a scenario where these requirements apply is the telephony
API: Even on non-phone devices, these APIs must be implemented as reasonable
no-ops.

Device implementations MUST consistently report accurate hardware configuration
information via the getSystemAvailableFeatures() and hasSystemFeature(String)
methods on the
[android.content.pm.PackageManager](http://developer.android.com/reference/android/content/pm/PackageManager.html)
class for the same build fingerprint.

