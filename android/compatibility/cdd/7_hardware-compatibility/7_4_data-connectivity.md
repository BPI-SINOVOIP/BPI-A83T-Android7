## 7.4\. Data Connectivity

### 7.4.1\. Telephony

“Telephony” as used by the Android APIs and this document refers specifically
to hardware related to placing voice calls and sending SMS messages via a GSM
or CDMA network. While these voice calls may or may not be packet-switched,
they are for the purposes of Android considered independent of any data
connectivity that may be implemented using the same network. In other words,
the Android “telephony” functionality and APIs refer specifically to voice
calls and SMS. For instance, device implementations that cannot place calls or
send/receive SMS messages MUST NOT report the android.hardware.telephony
feature or any subfeatures, regardless of whether they use a cellular network
for data connectivity.

Android MAY be used on devices that do not include telephony hardware. That is,
Android is compatible with devices that are not phones. However, if a device
implementation does include GSM or CDMA telephony, it MUST implement full
support for the API for that technology. Device implementations that do not
include telephony hardware MUST implement the full APIs as no-ops.

#### 7.4.1.1\. Number Blocking Compatibility

Android Telephony device implementations MUST include number blocking support
and:

*   MUST fully implement [BlockedNumberContract](http://developer.android.com/reference/android/provider/BlockedNumberContract.html)
and the corresponding API as described in the SDK documentation.
*   MUST block all calls and messages from a phone number in
'BlockedNumberProvider' without any interaction with apps. The only exception
is when number blocking is temporarily lifted as described in the SDK
documentation.
*   MUST NOT write to the [platform call log provider](http://developer.android.com/reference/android/provider/CallLog.html)
for a blocked call.
*   MUST NOT write to the [Telephony provider](http://developer.android.com/reference/android/provider/Telephony.html)
for a blocked message.
*   MUST implement a blocked numbers management UI, which is opened with the
intent returned by TelecomManager.createManageBlockedNumbersIntent() method.
*   MUST NOT allow secondary users to view or edit the blocked numbers on the
device as the Android platform assumes the primary user to have full control
of the telephony services, a single instance, on the device. All blocking
related UI MUST be hidden for secondary users and the blocked list MUST still
be respected.
*   SHOULD migrate the blocked numbers into the provider when a device updates
to Android 7.0.

### 7.4.2\. IEEE 802.11 (Wi-Fi)

All Android device implementations SHOULD include support for one or more forms
of 802.11\. If a device implementation does include support for 802.11 and exposes the
functionality to a third-party application, it MUST implement the corresponding
Android API and:

*   MUST report the hardware feature flag android.hardware.wifi.
*   MUST implement the [multicast API](http://developer.android.com/reference/android/net/wifi/WifiManager.MulticastLock.html)
as described in the SDK documentation.
*   MUST support multicast DNS (mDNS) and MUST NOT filter mDNS packets
(224.0.0.251) at any time of operation including:
    *   Even when the screen is not in an active state.
    *   For Android Television device implementations, even when in standby
power states.

#### 7.4.2.1\. Wi-Fi Direct

Device implementations SHOULD include support for Wi-Fi Direct (Wi-Fi
peer-to-peer). If a device implementation does include support for Wi-Fi
Direct, it MUST implement the
[corresponding Android API](http://developer.android.com/reference/android/net/wifi/p2p/WifiP2pManager.html)
as described in the SDK documentation. If a device implementation includes
support for Wi-Fi Direct, then it:

*   MUST report the hardware feature android.hardware.wifi.direct.
*   MUST support regular Wi-Fi operation.
*   SHOULD support concurrent Wi-Fi and Wi-Fi Direct operation.

#### 7.4.2.2\. Wi-Fi Tunneled Direct Link Setup

Device implementations SHOULD include support for [Wi-Fi
Tunneled Direct Link Setup (TDLS)](http://developer.android.com/reference/android/net/wifi/WifiManager.html)
as described in the Android SDK Documentation. If a device
implementation does include support for TDLS and TDLS is enabled by the
WiFiManager API, the device:

*   SHOULD use TDLS only when it is possible AND beneficial.
*   SHOULD have some heuristic and NOT use TDLS when its performance might be
worse than going through the Wi-Fi access point.

### 7.4.3\. Bluetooth

<div class="note">

Android Watch implementations MUST support Bluetooth. Android Television
implementations MUST support Bluetooth and Bluetooth LE. Android Automotive
implementations MUST support Bluetooth and SHOULD support Bluetooth LE.

</div>

Device implementations that support `android.hardware.vr.high_performance` feature MUST
support Bluetooth 4.2 and Bluetooth LE Data Length Extension.

Android includes support for
[Bluetooth and Bluetooth Low Energy](http://developer.android.com/reference/android/bluetooth/package-summary.html).
Device implementations that include support for Bluetooth and Bluetooth Low
Energy MUST declare the relevant platform features (android.hardware.bluetooth
and android.hardware.bluetooth_le respectively) and implement the platform APIs.
Device implementations SHOULD implement relevant Bluetooth profiles such as
A2DP, AVCP, OBEX, etc. as appropriate for the device.

Android Automotive implementations SHOULD support Message Access Profile (MAP).
Android Automotive implementations MUST support the following Bluetooth
profiles:

* Phone calling over Hands-Free Profile (HFP).
* Media playback over Audio Distribution Profile (A2DP).
* Media playback control over Remote Control Profile (AVRCP).
* Contact sharing using the Phone Book Access Profile (PBAP).

Device implementations including support for Bluetooth Low Energy:

*   MUST declare the hardware feature android.hardware.bluetooth_le.
*   MUST enable the GATT (generic attribute profile) based Bluetooth APIs as
described in the SDK documentation and
[android.bluetooth](http://developer.android.com/reference/android/bluetooth/package-summary.html).
*   are STRONGLY RECOMMENDED to implement a Resolvable Private Address (RPA)
timeout no longer than 15 minutes and rotate the address at timeout to protect
user privacy.
*   SHOULD support offloading of the filtering logic to the bluetooth chipset
when implementing the [ScanFilter API](https://developer.android.com/reference/android/bluetooth/le/ScanFilter.html),
and MUST report the correct value of where the filtering logic is implemented
whenever queried via the
android.bluetooth.BluetoothAdapter.isOffloadedFilteringSupported() method.
*   SHOULD support offloading of the batched scanning to the bluetooth chipset,
but if not supported, MUST report ‘false’ whenever queried via the
android.bluetooth.BluetoothAdapter.isOffloadedScanBatchingSupported() method.
*   SHOULD support multi advertisement with at least 4 slots, but if not
supported, MUST report ‘false’ whenever queried via the
android.bluetooth.BluetoothAdapter.isMultipleAdvertisementSupported() method.

### 7.4.4\. Near-Field Communications

Device implementations SHOULD include a transceiver and related hardware for
Near-Field Communications (NFC). If a device implementation does include NFC
hardware and plans to make it available to third-party apps, then it:

*   MUST report the android.hardware.nfc feature from the
[android.content.pm.PackageManager.hasSystemFeature() method](http://developer.android.com/reference/android/content/pm/PackageManager.html).
*   MUST be capable of reading and writing NDEF messages via the following NFC
standards:
    *   MUST be capable of acting as an NFC Forum reader/writer (as defined by
the NFC Forum technical specification NFCForum-TS-DigitalProtocol-1.0) via the
following NFC standards:
        *   NfcA (ISO14443-3A)
        *   NfcB (ISO14443-3B)
        *   NfcF (JIS X 6319-4)
        *   IsoDep (ISO 14443-4)
        *   NFC Forum Tag Types 1, 2, 3, 4 (defined by the NFC Forum)
    *   STRONGLY RECOMMENDED to be capable of reading and writing NDEF messages
as well as raw data via the following NFC standards. Note that while the NFC
standards below are stated as STRONGLY RECOMMENDED, the Compatibility
Definition for a future version is planned to change these to MUST. These
standards are optional in this version but will be required in future versions.
Existing and new devices that run this version of Android are very strongly
encouraged to meet these requirements now so they will be able to upgrade to
the future platform releases.
        *   NfcV (ISO 15693)
    *   SHOULD be capable of reading the barcode and URL (if encoded) of
[Thinfilm NFC Barcode](http://developer.android.com/reference/android/nfc/tech/NfcBarcode.html)
products.
    *   MUST be capable of transmitting and receiving data via the following
peer-to-peer standards and protocols:
        *   ISO 18092
        *   LLCP 1.2 (defined by the NFC Forum)
        *   SDP 1.0 (defined by the NFC Forum)
        *   [NDEF Push Protocol](http://static.googleusercontent.com/media/source.android.com/en/us/compatibility/ndef-push-protocol.pdf)
        *   SNEP 1.0 (defined by the NFC Forum)
    *   MUST include support for [Android Beam](http://developer.android.com/guide/topics/connectivity/nfc/nfc.html).
    *   MUST implement the SNEP default server. Valid NDEF messages
received by the default SNEP server MUST be dispatched to applications using
the android.nfc.ACTION_NDEF_DISCOVERED intent. Disabling Android Beam in
settings MUST NOT disable dispatch of incoming NDEF message.
    *   MUST honor the android.settings.NFCSHARING_SETTINGS intent to show
[NFC sharing settings](http://developer.android.com/reference/android/provider/Settings.html#ACTION_NFCSHARING_SETTINGS).
    *   MUST implement the NPP server. Messages received by the NPP server
MUST be processed the same way as the SNEP default server.
    *   MUST implement a SNEP client and attempt to send outbound P2P NDEF
to the default SNEP server when Android Beam is enabled. If no default SNEP
server is found then the client MUST attempt to send to an NPP server.
    *   MUST allow foreground activities to set the outbound P2P NDEF
message using android.nfc.NfcAdapter.setNdefPushMessage, and
android.nfc.NfcAdapter.setNdefPushMessageCallback, and
android.nfc.NfcAdapter.enableForegroundNdefPush.
    *   SHOULD use a gesture or on-screen confirmation, such as 'Touch to
Beam', before sending outbound P2P NDEF messages.
    *   SHOULD enable Android Beam by default and MUST be able to send and
receive using Android Beam, even when another proprietary NFC P2p mode is
turned on.
    *   MUST support NFC Connection handover to Bluetooth when the device
supports Bluetooth Object Push Profile. Device implementations MUST support
connection handover to Bluetooth when using
android.nfc.NfcAdapter.setBeamPushUris, by implementing the
“[Connection Handover version 1.2](http://members.nfc-forum.org/specs/spec_list/#conn_handover)” and
“[Bluetooth Secure Simple Pairing Using NFC version 1.0](http://members.nfc-forum.org/apps/group_public/download.php/18688/NFCForum-AD-BTSSP_1_1.pdf)”
specs from the NFC Forum. Such an implementation MUST implement the handover
LLCP service with service name “urn:nfc:sn:handover” for exchanging the
handover request/select records over NFC, and it MUST use the Bluetooth Object
Push Profile for the actual Bluetooth data transfer. For legacy reasons (to
remain compatible with Android 4.1 devices), the implementation SHOULD still
accept SNEP GET requests for exchanging the handover request/select records
over NFC. However an implementation itself SHOULD NOT send SNEP GET requests
for performing connection handover.
    *   MUST poll for all supported technologies while in NFC discovery mode.
    *   SHOULD be in NFC discovery mode while the device is awake with the
screen active and the lock-screen unlocked.

(Note that publicly available links are not available for the JIS, ISO, and NFC
Forum specifications cited above.)

Android includes support for NFC Host Card Emulation (HCE) mode. If a device
implementation does include an NFC controller chipset capable of HCE (for NfcA 
and/or NfcB) and it supports Application ID (AID) routing, then it:

*   MUST report the android.hardware.nfc.hce feature constant.
*   MUST support [NFC HCE
APIs](http://developer.android.com/guide/topics/connectivity/nfc/hce.html) as
defined in the Android SDK.

If a device implementation does include an NFC controller chipset capable of HCE
for NfcF, and it implements the feature for third-party applications, then it:

*   MUST report the android.hardware.nfc.hcef feature constant.
*   MUST implement the [NfcF Card Emulation APIs]
(https://developer.android.com/reference/android/nfc/cardemulation/NfcFCardEmulation.html)
as defined in the Android SDK.

Additionally, device implementations MAY include reader/writer support for the
following MIFARE technologies.

*   MIFARE Classic
*   MIFARE Ultralight
*   NDEF on MIFARE Classic

Note that Android includes APIs for these MIFARE types. If a device
implementation supports MIFARE in the reader/writer role, it:

*   MUST implement the corresponding Android APIs as documented by the Android SDK.
*   MUST report the feature com.nxp.mifare from the
[android.content.pm.PackageManager.hasSystemFeature()](http://developer.android.com/reference/android/content/pm/PackageManager.html)
method. Note that this is not a standard Android feature and as such does not
appear as a constant in the android.content.pm.PackageManager class.
*   MUST NOT implement the corresponding Android APIs nor report the
com.nxp.mifare feature unless it also implements general NFC support as
described in this section.

If a device implementation does not include NFC hardware, it MUST NOT declare
the android.hardware.nfc feature from the
[android.content.pm.PackageManager.hasSystemFeature()](http://developer.android.com/reference/android/content/pm/PackageManager.html)
method, and MUST implement the Android NFC API as a no-op.

As the classes android.nfc.NdefMessage and android.nfc.NdefRecord represent a
protocol-independent data representation format, device implementations MUST
implement these APIs even if they do not include support for NFC or declare the
android.hardware.nfc feature.

### 7.4.5\. Minimum Network Capability

Device implementations MUST include support for one or more forms of data
networking. Specifically, device implementations MUST include support for at
least one data standard capable of 200Kbit/sec or greater. Examples of
technologies that satisfy this requirement include EDGE, HSPA, EV-DO, 802.11g,
Ethernet, Bluetooth PAN, etc.

Device implementations where a physical networking standard (such as Ethernet)
is the primary data connection SHOULD also include support for at least one
common wireless data standard, such as 802.11 (Wi-Fi).

Devices MAY implement more than one form of data connectivity.

Devices MUST include an IPv6 networking stack and support IPv6 communication
using the managed APIs, such as `java.net.Socket` and `java.net.URLConnection`,
as well as the native APIs, such as `AF_INET6` sockets. The required level of
IPv6 support depends on the network type, as follows:

*   Devices that support Wi-Fi networks MUST support dual-stack and IPv6-only
operation on Wi-Fi.
*   Devices that support Ethernet networks MUST support dual-stack operation on
Ethernet.
*   Devices that support cellular data SHOULD support IPv6 operation (IPv6-only
and possibly dual-stack) on cellular data.
*   When a device is simultaneously connected to more than one network (e.g.,
Wi-Fi and cellular data), it MUST simultaneously meet these requirements on
each network to which it is connected.

IPv6 MUST be enabled by default.

In order to ensure that IPv6 communication is as reliable as IPv4, unicast IPv6
packets sent to the device MUST NOT be dropped, even when the screen is not in
an active state. Redundant multicast IPv6 packets, such as repeated identical
Router Advertisements, MAY be rate-limited in hardware or firmware if doing so
is necessary to save power. In such cases, rate-limiting MUST NOT cause the
device to lose IPv6 connectivity on any IPv6-compliant network that uses RA
lifetimes of at least 180 seconds.

IPv6 connectivity MUST be maintained in doze mode.

### 7.4.6\. Sync Settings

Device implementations MUST have the master auto-sync setting on by default so
that the method
[getMasterSyncAutomatically()](http://developer.android.com/reference/android/content/ContentResolver.html)
returns “true”.

### 7.4.7\. Data Saver

Device implementations with a metered connection are STRONGLY RECOMMENDED to provide the
data saver mode.

If a device implementation provides the data saver mode, it:

*   MUST support all the APIs in the `ConnectivityManager` class as described in the
[SDK documentation](https://developer.android.com/training/basics/network-ops/data-saver.html)

*   MUST provide a user interface in the settings, allowing users to add
    applications to or remove applications from the whitelist.

Conversely if a device implementation does not provide the data saver mode, it:

*   MUST return the value `RESTRICT_BACKGROUND_STATUS_DISABLED` for
    [`ConnectivityManager.getRestrictBackgroundStatus()`](https://developer.android.com/reference/android/net/ConnectivityManager.html#getRestrictBackgroundStatus%28%29)

*   MUST not broadcast `ConnectivityManager.ACTION_RESTRICT_BACKGROUND_CHANGED`

*   MUST have an activity that handles the `Settings.ACTION_IGNORE_BACKGROUND_DATA_RESTRICTIONS_SETTINGS`
    intent but MAY implement it as a no-op.
