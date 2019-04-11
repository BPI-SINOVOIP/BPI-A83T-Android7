## 9.9\. Data Storage Encryption

<div class="note">

Optional for Android device implementations without a secure lock screen.

</div>

If the device implementation supports a secure lock screen as described in section 9.11.1,
then the device MUST support data storage encryption of the application private data (/data partition), as well as the
application shared storage partition (/sdcard partition) if it is a permanent,
non-removable part of the device.

For device implementations supporting data storage encryption and with Advanced
Encryption Standard (AES) crypto performance above 50MiB/sec, the data storage
encryption MUST be enabled by default at the time the user has completed the
out-of-box setup experience. If a device implementation is already launched on
an earlier Android version with encryption disabled by default, such
a device cannot meet the requirement through a system software update and thus
MAY be exempted.

Device implementations SHOULD meet the above data storage encryption requirement
via implementing [File Based Encryption](https://source.android.com/security/encryption/file-based.html)
(FBE).

### 9.9.1\. Direct Boot

All devices MUST implement the
[Direct Boot mode](http://developer.android.com/preview/features/direct-boot.html) APIs even
if they do not support Storage Encryption. In particular, the
[LOCKED_BOOT_COMPLETED](https://developer.android.com/reference/android/content/Intent.html#LOCKED_BOOT_COMPLETED)
and
[ACTION_USER_UNLOCKED](https://developer.android.com/reference/android/content/Intent.html#ACTION_USER_UNLOCKED)
Intents must still be broadcast to signal Direct Boot aware applications that
Device Encrypted (DE) and Credential Encrypted (CE) storage locations are
available for user.

### 9.9.2\. File Based Encryption

Device implementations supporting FBE:

- MUST boot up without challenging the user for credentials and allow Direct
  Boot aware apps to access to the Device Encrypted (DE) storage after the
  LOCKED_BOOT_COMPLETED message is broadcasted.
- MUST only allow access to Credential Encrypted (CE) storage after the user 
  has unlocked the device by supplying their credentials (eg. passcode, pin,
  pattern or fingerprint) and the ACTION_USER_UNLOCKED message is broadcasted.
  Device implementations MUST NOT offer any
  method to unlock the CE protected storage without the user supplied
  credentials.
- MUST support Verified Boot and ensure that DE keys are cryptographically
  bound to the device's hardware root of trust.
- MUST support encrypting file contents using AES with a key length of 256-bits
  in XTS mode.
- MUST support encrypting file name using AES with a key length of 256-bits in
  CBC-CTS mode.
- MAY support alternative ciphers, key lengths and modes for file content and
  file name encryption, but MUST use the mandatorily supported ciphers,
  key lengths and modes by default.
- SHOULD make preloaded essential apps (e.g. Alarm, Phone, Messenger)
  Direct Boot aware.

The keys protecting CE and DE storage areas:

- MUST be cryptographically bound to a hardware-backed Keystore. CE keys
  must be bound to a user's lock screen credentials. If the user has
  specified no lock screen credentials then the CE keys MUST be bound to
  a default passcode.
- MUST be unique and distinct, in other words no user's CE or DE key
  may match any other user's CE or DE keys.

The upstream Android Open Source project provides a preferred implementation of
this feature based on the Linux kernel ext4 encryption feature.

### 9.9.3\. Full Disk Encryption
  Device implementations supporting [full disk encryption](http://source.android.com/devices/tech/security/encryption/index.html)
  (FDE). MUST use AES with a key of 128-bits
  (or greater) and a mode designed for storage (for example, AES-XTS,
  AES-CBC-ESSIV). The encryption key MUST NOT be written to storage at any time
  without being encrypted. The user MUST be provided with the possibility to AES
  encrypt the encryption key, except when it is in active use, with the lock
  screen credentials stretched using a slow stretching algorithm
  (e.g. PBKDF2 or scrypt). If the user has not specified a lock screen
  credentials or has disabled use of the passcode for encryption, the system
  SHOULD use a default passcode to wrap the encryption key. If the device
  provides a hardware-backed keystore, the password stretching algorithm MUST
  be cryptographically bound to that keystore. The encryption key MUST NOT be
  sent off the device (even when wrapped with the user passcode and/or hardware
  bound key). The upstream Android Open Source project provides a preferred
  implementation of this feature based on the Linux kernel feature dm-crypt.