# 4\. Application Packaging Compatibility

Device implementations MUST install and run Android “.apk” files as generated
by the “aapt” tool included in the [official Android SDK](http://developer.android.com/tools/help/index.html).
For this reason device implementations SHOULD use the reference implementation’s
package management system.

The package manager MUST support verifying “.apk” files using the
[APK Signature Scheme v2](https://source.android.com/security/apksigning/v2.html).

Devices implementations MUST NOT extend either the
[.apk](http://developer.android.com/guide/components/fundamentals.html),
[Android Manifest](http://developer.android.com/guide/topics/manifest/manifest-intro.html),
[Dalvik bytecode](https://android.googlesource.com/platform/dalvik/), or
RenderScript bytecode formats in such a way that would prevent those files from
installing and running correctly on other compatible devices.

Device implementations MUST NOT allow apps other than the current
"installer of record" for the package to silently uninstall the app without any
prompt, as documented in the SDK for the [`DELETE_PACKAGE`](https://developer.android.com/reference/android/Manifest.permission.html#DELETE_PACKAGES)
permission. The only exceptions are the system package verifier app handling
[PACKAGE_NEEDS_VERIFICATION](https://developer.android.com/reference/android/content/Intent.html#ACTION_PACKAGE_NEEDS_VERIFICATION)
intent and the storage manager app handling [ACTION_MANAGE_STORAGE](https://developer.android.com/reference/android/os/storage/StorageManager.html#ACTION_MANAGE_STORAGE)
intent.
