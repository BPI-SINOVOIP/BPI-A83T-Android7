## 3.12\. TV Input Framework

The [Android Television Input Framework (TIF)](http://source.android.com/devices/tv/index.html)
simplifies the delivery of live content to Android Television devices. TIF
provides a standard API to create input modules that control Android Television
devices. Android Television device implementations MUST support TV Input
Framework.

Device implementations that support TIF MUST declare the platform feature
android.software.live_tv.

### 3.12.1\. TV App

Any device implementation that declares support for Live TV MUST have an
installed TV application (TV App). The Android Open Source Project provides an
implementation of the TV App.

The TV App MUST provide facilities to install and use
[TV Channels](http://developer.android.com/reference/android/media/tv/TvContract.Channels.html)
and meet the following requirements:

*   Device implementations MUST allow third-party TIF-based inputs
    ([third-party inputs](https://source.android.com/devices/tv/index.html#third-party_input_example))
    to be installed and managed.
*   Device implementations MAY provide visual separation between pre-installed
    [TIF-based inputs](https://source.android.com/devices/tv/index.html#tv_inputs)
    (installed inputs) and third-party inputs.
*   Device implementations MUST NOT display the third-party inputs more than a
    single navigation action away from the TV App (i.e. expanding a list of
    third-party inputs from the TV App).

#### 3.12.1.1\. Electronic Program Guide

Android Television device implementations MUST show an informational and
interactive overlay, which MUST include an electronic program guide (EPG)
generated from the values in the
[TvContract.Programs](https://developer.android.com/reference/android/media/tv/TvContract.Programs.html)
fields. The EPG MUST meet the following requirements:

*   The EPG MUST display information from all installed inputs and third-party
    inputs.
*   The EPG MAY provide visual separation between the installed inputs and
    third-party inputs.
*   The EPG is STRONGLY RECOMMENDED to display installed inputs and third-party
    inputs with equal prominence. The EPG MUST NOT display the third-party
    inputs more than a single navigation action away from the installed inputs
    on the EPG.
*   On channel change, device implementations MUST display EPG data for the
    currently playing program.

#### 3.12.1.2\. Navigation

The TV App MUST allow navigation for the following functions via the D-pad,
Back, and Home keys on the Android Television device’s input device(s)
(i.e. remote control, remote control application, or game controller):

*   Changing TV channels
*   Opening EPG
*   Configuring and tuning to third-party TIF-based inputs
*   Opening Settings menu

The TV App SHOULD pass key events to HDMI inputs through CEC.

#### 3.12.1.3\. TV input app linking

Android Television device implementations MUST support
[TV input app linking](http://developer.android.com/reference/android/media/tv/TvContract.Channels.html#COLUMN_APP_LINK_INTENT_URI),
which allows all inputs to provide activity links from the current activity to
another activity (i.e. a link from live programming to related content). The TV
App MUST show TV input app linking when it is provided.

#### 3.12.1.4\. Time shifting

Android Television device implementations MUST support time shifting, which
allows the user to pause and resume live content. Device implementations MUST
provide the user a way to pause and resume the currently playing program, if
time shifting for that program
[is available](https://developer.android.com/reference/android/media/tv/TvInputManager.html#TIME_SHIFT_STATUS_AVAILABLE). 

#### 3.12.1.5\. TV recording

Android Television device implementations are STRONGLY RECOMMENDED to support
TV recording. If the TV input supports recording, the EPG MAY provide a way to
[record a program](https://developer.android.com/reference/android/media/tv/TvInputInfo.html#canRecord%28%29)
if the recording of such a program is not
[prohibited](https://developer.android.com/reference/android/media/tv/TvContract.Programs.html#COLUMN_RECORDING_PROHIBITED).
Device implementations SHOULD provide a user interface to play recorded programs.
