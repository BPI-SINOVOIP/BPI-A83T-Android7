## 9.10\. Device Integrity

The following requirements ensures there is transparancy to the status of the
device integrity.

Device implementations MUST correctly report through the System API method
PersistentDataBlockManager.getFlashLockState() whether their bootloader state
permits flashing of the system image. The `FLASH_LOCK_UNKNOWN` state is reserved
for device implementations upgrading from an earlier version of Android where this
new system API method did not exist.

Verified boot is a feature that guarantees the integrity of the device
software. If a device implementation supports the feature, it MUST:

*   Declare the platform feature flag android.software.verified_boot.
*   Perform verification on every boot sequence.
*   Start verification from an immutable hardware key that is the root of trust
and go all the way up to the system partition.
*   Implement each stage of verification to check the integrity and
authenticity of all the bytes in the next stage before executing the code in
the next stage.
*   Use verification algorithms as strong as current recommendations from NIST
for hashing algorithms (SHA-256) and public key sizes (RSA-2048).
*   MUST NOT allow boot to complete when system verification fails, unless the
user consents to attempt booting anyway, in which case the data from any
non-verified storage blocks MUST not be used.
*   MUST NOT allow verified partitions on the device to be modified unless the
user has explicitly unlocked the boot loader.

The upstream Android Open Source Project provides a preferred implementation of
this feature based on the Linux kernel feature dm-verity.

Starting from Android 6.0, device implementations with Advanced Encryption
Standard (AES) crypto performance above 50 MiB/seconds MUST support verified boot
for device integrity.

If a device implementation is already launched without supporting verified boot
on an earlier version of Android, such a device can not add support for this feature
with a system software update and thus are exempted from the requirement.

