## 7.6\. Memory and Storage

### 7.6.1\. Minimum Memory and Storage

<div class="note">

Android Television devices MUST have at least 4GB of non-volatile storage
available for application private data.

</div>

The memory available to the kernel and userspace on device implementations MUST
be at least equal or larger than the minimum values specified by the following
table. (See [section 7.1.1](#7_1_1_screen_configuration) for screen size and
density definitions.)

<table>
 <tr>
    <th>Density and screen size</th>
    <th>32-bit device</th>
    <th>64-bit device</th>
 </tr>
 <tr>
    <td>Android Watch devices (due to smaller screens)</td>
    <td>416MB</td>
    <td>Not applicable</td>
 </tr>
 <tr>
    <td><ul>
    <li class="table_list">280dpi or lower on small/normal screens</li>
    <li class="table_list">mdpi or lower on large screens</li>
    <li class="table_list">ldpi or lower on extra large screens</li>
    </ul></td>
    <td>512MB</td>
    <td>816MB</td>
 </tr>
 <tr>
    <td><ul>
    <li class="table_list">xhdpi or higher on small/normal screens</li>
    <li class="table_list">hdpi or higher on large screens</li>
    <li class="table_list">mdpi or higher on extra large screens</li></ul></td>
    <td>608MB</td>
    <td>944MB</td>
 </tr>
 <tr>
    <td><ul>
    <li class="table_list">400dpi or higher on small/normal screens</li>
    <li class="table_list">xhdpi or higher on large screens</li>
     <li class="table_list">tvdpi or higher on extra large screens</li></ul></td>
    <td>896MB</td>
    <td>1280MB</td>
 </tr>
 <tr>
    <td><ul>
    <li class="table_list">560dpi or higher on small/normal screens</li>
    <li class="table_list">400dpi or higher on large screens</li>
    <li class="table_list">xhdpi or higher on extra large screens</li></ul></td>
    <td>1344MB</td>
    <td>1824MB</td>
 </tr>
</table>

The minimum memory values MUST be in addition to any memory space already
dedicated to hardware components such as radio, video, and so on that is not
under the kernel’s control.

Device implementations with less than 512MB of memory available to the kernel
and userspace, unless an Android Watch, MUST return the value "true" for
ActivityManager.isLowRamDevice().

Android Television devices MUST have at least 4GB and other device
implementations MUST have at least 3GB of non-volatile storage available for
application private data. That is, the /data partition MUST be at least 4GB for
Android Television devices and at least 3GB for other device implementations.
Device implementations that run Android are **STRONGLY RECOMMENDED** to have at
least 4GB of non-volatile storage for application private data so they will be
able to upgrade to the future platform releases.

The Android APIs include a [Download Manager](http://developer.android.com/reference/android/app/DownloadManager.html)
that applications MAY use to download data files. The device implementation of
the Download Manager MUST be capable of downloading individual files of at
least 100MB in size to the default “cache” location.

### 7.6.2\. Application Shared Storage

Device implementations MUST offer shared storage for applications also often
referred as “shared external storage”.

Device implementations MUST be configured with shared storage mounted by
default, “out of the box”. If the shared storage is not mounted on the
Linuxpath /sdcard, then the device MUST include a Linux symbolic link from
/sdcard to the actual mount point.

Device implementations MAY have hardware for user-accessible removable storage,
such as a Secure Digital (SD) card slot. If this slot is used to satisfy the
shared storage requirement, the device implementation:

*   MUST implement a toast or pop-up user interface warning the user when there
is no SD card.
*   MUST include a FAT-formatted SD card 1GB in size or larger OR show on the
box and other material available at time of purchase that the SD card has to be
separately purchased.
*   MUST mount the SD card by default.

Alternatively, device implementations MAY allocate internal (non-removable)
storage as shared storage for apps as included in the upstream Android Open
Source Project; device implementations SHOULD use this configuration and
software implementation. If a device implementation uses internal
(non-removable) storage to satisfy the shared storage requirement, while that
storage MAY share space with the application private data, it MUST be at least
1GB in size and mounted on /sdcard (or /sdcard MUST be a symbolic link to the
physical location if it is mounted elsewhere).

Device implementations MUST enforce as documented the
android.permission.WRITE_EXTERNAL_STORAGE permission on this shared storage.
Shared storage MUST otherwise be writable by any application that obtains that
permission.

Device implementations that include multiple shared storage paths (such as both
an SD card slot and shared internal storage) MUST allow only pre-installed &amp;
privileged Android applications with the WRITE_EXTERNAL_STORAGE permission to
write to the secondary external storage, except when writing to their
package-specific directories or within the `URI` returned by firing the
`ACTION_OPEN_DOCUMENT_TREE` intent.

However, device implementations SHOULD expose content from both storage paths
transparently through Android’s media scanner service and
android.provider.MediaStore.

Regardless of the form of shared storage used, if the device implementation has
a USB port with USB peripheral mode support, it MUST provide some mechanism to
access the contents of shared storage from a host computer. Device
implementations MAY use USB mass storage, but SHOULD use Media Transfer
Protocol to satisfy this requirement. If the device implementation supports
Media Transfer Protocol, it:

*   SHOULD be compatible with the reference Android MTP host,
[Android File Transfer](http://www.android.com/filetransfer).
*   SHOULD report a USB device class of 0x00.
*   SHOULD report a USB interface name of 'MTP'.

### 7.6.3\. Adoptable Storage

Device implementations are STRONGLY RECOMMENDED to implement
[adoptable storage](http://source.android.com/devices/storage/adoptable.html) if the
removable storage device port is in a long-term stable location, such as within
the battery compartment or other protective cover.

Device implementations such as a television, MAY enable adoption through USB
ports as the device is expected to be static and not mobile. But for other
device implementations that are mobile in nature, it is STRONGLY RECOMMENDED to
implement the adoptable storage in a long-term stable location, since
accidentally disconnecting them can cause data loss/corruption.
