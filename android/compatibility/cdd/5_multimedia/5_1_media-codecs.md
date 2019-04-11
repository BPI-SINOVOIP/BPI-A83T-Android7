## 5.1\. Media Codecs

Device implementations—

*   MUST support the [core media
formats](http://developer.android.com/guide/appendix/media-formats.html)
specified in the Android SDK documentation, except where explicitly permitted
in this document.

*   MUST support the media formats, encoders, decoders, file types, and
container formats defined in the tables below and reported via
[MediaCodecList](http://developer.android.com/reference/android/media/MediaCodecList.html).

*   MUST also be able to decode all profiles reported in its
[CamcorderProfile](http://developer.android.com/reference/android/media/CamcorderProfile.html)

*   MUST be able to decode all formats it can encode. This includes all
    bitstreams that its encoders generate.

Codecs SHOULD aim for minimum codec latency, in other words, codecs—

*   SHOULD NOT consume and store input buffers and return input buffers only
once processed
*   SHOULD NOT hold onto decoded buffers for longer than as specified by the
standard (e.g. SPS).
*   SHOULD NOT hold onto encoded buffers longer than required by the GOP
structure.

All of the  codecs listed in the table below are provided as software
implementations in the preferred Android implementation from the Android Open
Source Project.

Please note that neither Google nor the Open Handset Alliance make any
representation that these codecs are free from third-party patents. Those
intending to use this source code in hardware or software products are advised
that implementations of this code, including in open source software or
shareware, may require patent licenses from the relevant patent holders.



### 5.1.1\. Audio Codecs

<table>
 <tr>
    <th>Format/Codec</th>
    <th>Encoder</th>
    <th>Decoder</th>
    <th>Details</th>
    <th>Supported File Types/Container Formats</th>
 </tr>
 <tr>
    <td>MPEG-4 AAC Profile<br />

(AAC LC)</td>
    <td>REQUIRED<sup>1</sup></td>
    <td>REQUIRED</td>
    <td>Support for mono/stereo/5.0/5.1<sup>2</sup> content with standard
    sampling rates from 8 to 48 kHz.</td>
    <td>
    <ul>
    <li class="table_list">3GPP (.3gp)</li>
    <li class="table_list">MPEG-4 (.mp4, .m4a)</li>
    <li class="table_list">ADTS raw AAC (.aac, decode in Android 3.1+, encode in
    Android 4.0+, ADIF not supported)</li>
    <li class="table_list">MPEG-TS (.ts, not seekable, Android 3.0+)</li></ul>
    </td>
 </tr>
 <tr>
    <td>MPEG-4 HE AAC Profile (AAC+)</td>
    <td>REQUIRED<sup>1</sup><br>(Android 4.1+)</td>
    <td>REQUIRED</td>
    <td>Support for mono/stereo/5.0/5.1<sup>2</sup> content with standard
    sampling rates from 16 to 48 kHz.</td>
    <td></td>
 </tr>
 <tr>
    <td>MPEG-4 HE AACv2<br />

Profile (enhanced AAC+)</td>
    <td> </td>
    <td>REQUIRED</td>
    <td>Support for mono/stereo/5.0/5.1<sup>2</sup> content with standard
    sampling rates from 16 to 48 kHz.</td>
    <td></td>
 </tr>
 <tr>
    <td>AAC ELD (enhanced low delay AAC)</td>
    <td>REQUIRED<sup>1</sup> <br />

(Android 4.1+)</td>
    <td>REQUIRED<br />

(Android 4.1+)</td>
    <td>Support for mono/stereo content with standard sampling rates from 16 to
    48 kHz.</td>
    <td></td>
 </tr>
 <tr>
    <td>AMR-NB</td>
    <td>REQUIRED<sup>3</sup></td>
    <td>REQUIRED<sup>3</sup></td>
    <td>4.75 to 12.2 kbps sampled @ 8 kHz</td>
    <td>3GPP (.3gp)</td>
 </tr>
 <tr>
    <td>AMR-WB</td>
    <td>REQUIRED<sup>3</sup></td>
    <td>REQUIRED<sup>3</sup></td>
    <td>9 rates from 6.60 kbit/s to 23.85 kbit/s sampled @ 16 kHz</td>
    <td></td>
 </tr>
 <tr>
    <td>FLAC</td>
    <td></td>
    <td>REQUIRED <br>(Android 3.1+)</td>
    <td>Mono/Stereo (no multichannel). Sample rates up to 48 kHz (but up to 44.1
    kHz is RECOMMENDED on devices with 44.1 kHz output, as the 48 to 44.1 kHz
    downsampler does not include a low-pass filter). 16-bit RECOMMENDED; no
    dither applied for 24-bit.</td>
    <td>FLAC (.flac) only</td>
 </tr>
 <tr>
    <td>MP3</td>
    <td></td>
    <td>REQUIRED</td>
    <td>Mono/Stereo 8-320Kbps constant (CBR) or variable bitrate (VBR)</td>
    <td>MP3 (.mp3)</td>
 </tr>
 <tr>
    <td>MIDI</td>
    <td></td>
    <td>REQUIRED</td>
    <td>MIDI Type 0 and 1. DLS Version 1 and 2. XMF and Mobile XMF. Support for
    ringtone formats RTTTL/RTX, OTA, and iMelody</td>
    <td><ul>
    <li class="table_list">Type 0 and 1 (.mid, .xmf, .mxmf)</li>
    <li class="table_list">RTTTL/RTX (.rtttl, .rtx)</li>
    <li class="table_list">OTA (.ota)</li>
    <li class="table_list">iMelody (.imy)</li></ul></td>
 </tr>
 <tr>
    <td>Vorbis</td>
    <td></td>
    <td>REQUIRED</td>
    <td></td>
    <td><ul>
    <li class="table_list">Ogg (.ogg)</li>
    <li class="table_list">Matroska (.mkv, Android 4.0+)</li></ul></td>
 </tr>
 <tr>
    <td>PCM/WAVE</td>
    <td>REQUIRED<sup>4</sup><br> (Android 4.1+)</td>
    <td>REQUIRED</td>
    <td>16-bit linear PCM (rates up to limit of hardware). Devices MUST support
    sampling rates for raw PCM recording at 8000, 11025, 16000, and 44100 Hz
    frequencies.</td>
    <td>WAVE (.wav)</td>
 </tr>
 <tr>
    <td>Opus</td>
    <td></td>
    <td>REQUIRED<br> (Android 5.0+)</td>
    <td></td>
    <td>Matroska (.mkv), Ogg(.ogg)</td>
 </tr>
</table>


<p class="table_footnote"> 1 Required for device implementations that define
android.hardware.microphone but optional for Android Watch device
implementations.</p>

<p class="table_footnote">2 Recording or playback MAY be performed in mono
or stereo, but the decoding of AAC input buffers of multichannel streams
(i.e. more than two channels) to PCM through the default AAC audio decoder
in the android.media.MediaCodec API, the following MUST be supported:
<ul>
<li>decoding is performed without downmixing (e.g. a 5.0 AAC stream must be
decoded to five channels of PCM, a 5.1 AAC stream must be decoded to six
channels of PCM),</li>
<li>dynamic range metadata, as defined in "Dynamic Range Control (DRC)"
in ISO/IEC 14496-3, and the android.media.MediaFormat DRC keys to
configure the dynamic range-related behaviors of the audio decoder. The
AAC DRC keys were introduced in API 21,and are:
KEY_AAC_DRC_ATTENUATION_FACTOR, KEY_AAC_DRC_BOOST_FACTOR,
KEY_AAC_DRC_HEAVY_COMPRESSION, KEY_AAC_DRC_TARGET_REFERENCE_LEVEL and
KEY_AAC_ENCODED_TARGET_LEVEL</li>
</ul></p>

<p class="table_footnote">3 Required for Android Handheld device
implementations.</p>

<p class="table_footnote">4 Required for device implementations that define
android.hardware.microphone, including Android Watch device implementations.</p>

### 5.1.2\. Image Codecs

<table>
 <tr>
    <th>Format/Codec</th>
    <th>Encoder</th>
    <th>Decoder</th>
    <th>Details</th>
    <th>Supported File Types/Container Formats</th>
 </tr>
 <tr>
    <td>JPEG</td>
    <td>REQUIRED</td>
    <td>REQUIRED</td>
    <td>Base+progressive</td>
    <td>JPEG (.jpg)</td>
 </tr>
 <tr>
    <td>GIF</td>
    <td></td>
    <td>REQUIRED</td>
    <td></td>
    <td>GIF (.gif)</td>
 </tr>
 <tr>
    <td>PNG</td>
    <td>REQUIRED</td>
    <td>REQUIRED</td>
    <td></td>
    <td>PNG (.png)</td>
 </tr>
 <tr>
    <td>BMP</td>
    <td></td>
    <td>REQUIRED</td>
    <td></td>
    <td>BMP (.bmp)</td>
 </tr>
 <tr>
    <td>WebP</td>
    <td>REQUIRED</td>
    <td>REQUIRED</td>
    <td></td>
    <td>WebP (.webp)</td>
 </tr>
 <tr>
    <td>Raw</td>
    <td></td>
    <td>REQUIRED</td>
    <td></td>
    <td>ARW (.arw), CR2 (.cr2), DNG (.dng), NEF (.nef), NRW (.nrw), ORF (.orf),
        PEF (.pef), RAF (.raf), RW2 (.rw2), SRW (.srw)</td>
 </tr>
</table>


<h3 id="5_1_3_video_codecs">5.1.3. Video Codecs</h3>


*   Codecs advertising HDR profile support MUST support HDR static metadata
parsing and handling.

*   If a media codec advertises intra refresh support, then it MUST support the
refresh periods in the range of 10 - 60 frames and accurately operate within
20% of configured refresh period.

*   Video codecs MUST support output and input bytebuffer sizes that
accommodate the largest feasible compressed and uncompressed frame as dictated
by the standard and configuration but also not overallocate.

*   Video encoders and decoders MUST support YUV420 flexible color format
(COLOR_FormatYUV420Flexible).


<table>
 <tr>
    <th>Format/Codec</th>
    <th>Encoder</th>
    <th>Decoder</th>
    <th>Details</th>
    <th>Supported File Types/<br>Container Formats</th>
 </tr>
 <tr>
    <td>H.263</td>
    <td>MAY</td>
    <td>MAY</td>
    <td></td>
    <td><ul>
    <li class="table_list">3GPP (.3gp)</li>
    <li class="table_list">MPEG-4 (.mp4)</li></ul></td>
 </tr>
 <tr>
    <td>H.264 AVC</td>
    <td>REQUIRED<sup>2</sup></td>
    <td>REQUIRED<sup>2</sup></td>
    <td>See <a href="#5_2_video_encoding">section 5.2 </a>and
    <a href="#5_3_video_decoding">5.3</a> for details</td>
    <td><ul>
    <li class="table_list">3GPP (.3gp)</li>
    <li class="table_list">MPEG-4 (.mp4)</li>
    <li class="table_list">MPEG-2 TS (.ts, AAC audio only, not seekable, Android
    3.0+)</li></ul></td>
 </tr>
 <tr>
    <td>H.265 HEVC</td>
    <td></td>
    <td>REQUIRED<sup>5</sup></td>
    <td>See <a href="#5_3_video_decoding">section 5.3</a> for details</td>
    <td>MPEG-4 (.mp4)</td>
 </tr>
<tr>
  <td>MPEG-2</td>
  <td></td>
  <td>STRONGLY RECOMMENDED<sup>6</sup></td>
  <td>Main Profile</td>
  <td>MPEG2-TS</td>
</tr>
 <tr>
    <td>MPEG-4 SP</td>
    <td></td>
    <td>REQUIRED<sup>2</sup></td>
    <td></td>
    <td>3GPP (.3gp)</td>
 </tr>
 <tr>
    <td>VP8<sup>3</sup></td>
    <td>REQUIRED<sup>2</sup><br />

(Android 4.3+)</td>
    <td>REQUIRED<sup>2</sup><br />

(Android 2.3.3+)</td>
    <td>See <a href="#5_2_video_encoding">section 5.2</a> and
    <a href="#5_3_video_decoding">5.3</a> for details</td>
    <td><ul>
    <li class="table_list"><a href="http://www.webmproject.org/">WebM
    (.webm)</a></li>
    <li class="table_list">Matroska (.mkv, Android 4.0+)<sup>4</sup></li></ul>
    </td>
 </tr>
 <tr>
    <td>VP9</td>
    <td></td>
    <td>REQUIRED<sup>2</sup><br> (Android 4.4+)</td>
    <td>See <a href="#5_3_video_decoding">section 5.3</a> for details</td>
    <td><ul>
    <li class="table_list"><a href="http://www.webmproject.org/">WebM
    (.webm)</a></li>
    <li class="table_list">Matroska (.mkv, Android 4.0+)<sup>4</sup></li></ul>
    </td>
 </tr>
</table>


<p class="table_footnote">1 Required for device implementations that include
camera hardware and define android.hardware.camera or
android.hardware.camera.front.</p>

<p class="table_footnote">2 Required for device implementations except Android
Watch devices. </p>

<p class="table_footnote">3 For acceptable quality of web video streaming and
video-conference services, device implementations SHOULD use a hardware VP8
codec that meets the
<a href="http://www.webmproject.org/hardware/rtc-coding-requirements/">requirements</a>.
</p>

<p class="table_footnote">4 Device implementations SHOULD support writing
Matroska WebM files.</p>

<p class="table_footnote">5 STRONGLY RECOMMENDED for Android Automotive,
optional for Android Watch, and required for all other device types.</p>

<p class="table_footnote">6 Applies only to Android Television device
implementations.</p>




