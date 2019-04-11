## 5.2\. Video Encoding

<div class="note">
Video codecs are optional for Android Watch device implementations.
</div>

H.264, VP8, VP9 and HEVC video encoders—

*   MUST support dynamically configurable bitrates.
*   SHOULD support variable frame rates, where video encoder SHOULD determine
instantaneous frame duration based on the timestamps of input buffers, and
allocate its bit bucket based on that frame duration.

H.263 and MPEG-4 video encoder SHOULD support dynamically configurable
bitrates.

All video encoders SHOULD meet the following bitrate targets over two sliding
windows:

*   It SHOULD be not more than ~15% over the bitrate between intraframe
(I-frame) intervals.
*   It SHOULD be not more than ~100% over the bitrate over a sliding window of
1 second.

### 5.2.1\. H.263

Android device implementations with H.263 encoders MUST support Baseline Profile Level 45.

### 5.2.2\. H-264

Android device implementations with H.264 codec support:

*   MUST support Baseline Profile Level 3.<br>
    However, support for ASO (Arbitrary Slice Ordering), FMO (Flexible Macroblock
    Ordering) and RS (Redundant Slices) is OPTIONAL. Moreover, to maintain
    compatibility with other Android devices, it is RECOMMENDED that ASO, FMO
    and RS are not used for Baseline Profile by encoders.
*   MUST support the  SD (Standard Definition) video encoding profiles in the following table.
*   SHOULD support Main Profile Level 4.
*   SHOULD support the  HD (High Definition) video encoding profiles as indicated in the following table.
*   In addition, Android Television devices are STRONGLY RECOMMENDED to encode HD 1080p video at 30 fps.

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
    <td>20 fps</td>
    <td>30 fps</td>
    <td>30 fps</td>
    <td>30 fps</td>
 </tr>
 <tr>
    <th>Video bitrate</th>
    <td>384 Kbps</td>
    <td>2 Mbps</td>
    <td>4 Mbps</td>
    <td>10 Mbps</td>
 </tr>
</table>


<p class="table_footnote">1 When supported by hardware, but STRONGLY RECOMMENDED
for Android Television devices.</p>

### 5.2.3\. VP8

Android device implementations with VP8 codec support MUST support the SD video
encoding profiles and SHOULD support the following HD (High Definition) video encoding profiles.

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
    <td>30 fps</td>
    <td>30 fps</td>
 </tr>
 <tr>
    <th>Video bitrate</th>
    <td>800 Kbps </td>
    <td>2 Mbps</td>
    <td>4 Mbps</td>
    <td>10 Mbps</td>
 </tr>
</table>

<p class="table_footnote">1 When supported by hardware.</p>


