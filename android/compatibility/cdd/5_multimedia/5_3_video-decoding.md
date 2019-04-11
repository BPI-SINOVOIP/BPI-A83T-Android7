## 5.3\. Video Decoding

<div class="note">
Video codecs are optional for Android Watch device implementations.
</div>

Device implementations&mdash;

*   MUST support dynamic video resolution and frame rate switching through the
    standard Android APIs within the same stream for all VP8, VP9, H.264, and
    H.265 codecs in real time and up to the maximum resolution supported by each
    codec on the device.

*   Implementations that support the Dolby Vision decoder&mdash;
   *   MUST provide a Dolby Vision-capable extractor.
   *   MUST properly display Dolby Vision content on the device screen or on a
       standard video output port (e.g., HDMI).

*   Implementations that provide a Dolby Vision-capable extractor MUST set the
    track index of backward-compatible base-layer(s) (if present) to be the same
    as the combined Dolby Vision layer's track index.

### 5.3.1\. MPEG-2

Android device implementations with MPEG-2 decoders must support the Main
Profile High Level.

### 5.3.2\. H.263

Android device implementations with H.263 decoders MUST support Baseline Profile
Level 30 and Level 45.

### 5.3.3\. MPEG-4
Android device implementations with MPEG-4 decoders MUST support Simple Profile
Level 3.

### 5.3.4\. H.264

Android device implementations with H.264 decoders:

*   MUST support Main Profile Level 3.1 and Baseline Profile.<br>
    Support for ASO (Arbitrary Slice Ordering), FMO (Flexible Macroblock Ordering)
    and RS (Redundant Slices) is OPTIONAL.
*   MUST be capable of decoding videos with the SD (Standard Definition)
    profiles listed in the following table and encoded with the Baseline Profile and
    Main Profile Level 3.1 (including 720p30).
*   SHOULD be capable of decoding videos with the HD (High Definition) profiles
    as indicated in the following table.
*   In addition, Android Television devices&mdash;
    *   MUST support High Profile Level 4.2 and the HD 1080p60 decoding profile.
    *   MUST be capable of decoding videos with both HD profiles as indicated
        in the following table and encoded with either the Baseline Profile, Main
        Profile, or the High Profile Level 4.2

<table>
 <tr>
    <th></th>
    <th>SD (Low quality)</th>
    <th>SD (High quality)</th>
    <th>HD 720p<sup>1</sup></th>
    <th>HD 1080p<sup>1</sup></th>
 </tr>
 <tr>
    <th>Video resolution</th>
    <td>320 x 240 px</td>
    <td>720 x 480 px</td>
    <td>1280 x 720 px</td>
    <td>1920 x 1080 px</td>
 </tr>
 <tr>
    <th>Video frame rate</th>
    <td>30 fps</td>
    <td>30 fps</td>
    <td>60 fps</td>
    <td>30 fps (60 fps<sup>2</sup>)</td>
 </tr>
 <tr>
    <th>Video bitrate</th>
    <td>800 Kbps </td>
    <td>2 Mbps</td>
    <td>8 Mbps</td>
    <td>20 Mbps</td>
 </tr>
</table>


<p class="table_footnote">1 REQUIRED for when the height as reported by the
Display.getSupportedModes() method is equal or greater than the video resolution.</p>

<p class="table_footnote">2 REQUIRED for Android Television device
implementations.</p>

### 5.3.5\. H.265 (HEVC)

Android device implementations, when supporting H.265 codec as described in
[section 5.1.3](#5_1_3_video_codecs):

*   MUST support the Main Profile Level 3 Main tier and the SD video decoding profiles
    as indicated in the following table.
*   SHOULD support the HD decoding profiles as indicated in the following table.
*   MUST support the HD decoding profiles as indicated in the following table
    if there is a hardware decoder.
*   In addition, Android Television devices:
   *   MUST support the HD 720p decoding profile.
   *   STRONGLY RECOMMENDED to support the HD 1080p decoding profile. If the HD 1080p
       decoding profile is supported, it MUST support the Main Profile Level 4.1 Main tier.
   *   SHOULD support the UHD decoding profile. If the UHD decoding profile is supported the 
    codec MUST support Main10 Level 5 Main Tier profile.

<table>
 <tr>
    <th></th>
    <th>SD (Low quality)</th>
    <th>SD (High quality)</th>
    <th>HD 720p</th>
    <th>HD 1080p</th>
    <th>UHD</th>
 </tr>
 <tr>
    <th>Video resolution</th>
    <td>352 x 288 px</td>
    <td>720 x 480 px</td>
    <td>1280 x 720 px</td>
    <td>1920 x 1080 px</td>
    <td>3840 x 2160 px</td>
 </tr>
 <tr>
    <th>Video frame rate</th>
    <td>30 fps</td>
    <td>30 fps</td>
    <td>30 fps</td>
    <td>30 fps (60 fps<sup>1</sup>)</td>
    <td>60 fps</td>
 </tr>
 <tr>
    <th>Video bitrate</th>
    <td>600 Kbps </td>
    <td>1.6 Mbps</td>
    <td>4 Mbps</td>
    <td>10 Mbps</td>
    <td>20 Mbps</td>
 </tr>
</table>

<p class="table_footnote">1 REQUIRED for Android Television device
implementations with H.265 hardware decoding.</p>


### 5.3.6\. VP8
Android device implementations, when supporting VP8 codec as described in
[section 5.1.3](https://source.android.com/compatibility/android-cdd.html#5_1_3_video_codecs):

*   MUST support the SD decoding profiles in the following table.
*   SHOULD support the HD decoding profiles in the following table.
*   Android Television devices MUST support the HD 1080p60 decoding profile.

<table>
 <tr>
    <th></th>
    <th>SD (Low quality)</th>
    <th>SD (High quality)</th>
    <th>HD 720p<sup>1</sup></th>
    <th>HD 1080p<sup>1</sup></th>
 </tr>
 <tr>
    <th>Video resolution</th>
    <td>320 x 180 px</td>
    <td>640 x 360 px</td>
    <td>1280 x 720 px</td>
    <td>1920 x 1080 px</td>
 </tr>
 <tr>
    <th>Video frame rate</th>
    <td>30 fps</td>
    <td>30 fps</td>
    <td>30 fps (60 fps<sup>2</sup>)</td>
    <td>30 (60 fps<sup>2</sup>)</td>
 </tr>
 <tr>
    <th>Video bitrate</th>
    <td>800 Kbps </td>
    <td>2 Mbps</td>
    <td>8 Mbps</td>
    <td>20 Mbps</td>
 </tr>
</table>

<p class="table_footnote">1 REQUIRED for when the height as reported by the
Display.getSupportedModes() method is equal or greater than the video resolution.</p>

<p class="table_footnote">2 REQUIRED for Android Television device
implementations.</p>

### 5.3.7\. VP9

Android device implementations, when supporting VP9 codec as described in
[section 5.1.3](https://source.android.com/compatibility/android-cdd.html#5_1_3_video_codecs):

*   MUST support the SD video decoding profiles as indicated in the following table.
*   SHOULD support the HD decoding profiles as indicated in the following table.
*   MUST support the HD decoding profiles as indicated in the following table,
    if there is a hardware decoder.
*   In addition, Android Television devices:

    *   MUST support the HD 720p decoding profile.
    *   STRONGLY RECOMMENDED to support the HD 1080p decoding profile.
    *   SHOULD support the UHD decoding profile. If the UHD video decoding
        profile is supported, it MUST support 8-bit color depth and SHOULD
        support VP9 Profile 2 (10-bit).

<table>
 <tr>
    <th></th>
    <th>SD (Low quality)</th>
    <th>SD (High quality)</th>
    <th>HD 720p</th>
    <th>HD 1080p</th>
    <th>UHD</th>
 </tr>
 <tr>
    <th>Video resolution</th>
    <td>320 x 180 px</td>
    <td>640 x 360 px</td>
    <td>1280 x 720 px</td>
    <td>1920 x 1080 px</td>
    <td>3840 x 2160 px</td>
 </tr>
 <tr>
    <th>Video frame rate</th>
    <td>30 fps</td>
    <td>30 fps</td>
    <td>30 fps</td>
    <td>30 fps (60 fps<sup>1</sup>)</td>
    <td>60 fps</td>
 </tr>
 <tr>
    <th>Video bitrate</th>
    <td>600 Kbps</td>
    <td>1.6 Mbps</td>
    <td>4 Mbps</td>
    <td>5 Mbps</td>
    <td>20 Mbps</td>
 </tr>
</table>

<p class="table_footnote">1 REQUIRED for Android Television
device implementations with VP9 hardware decoding.</p>
