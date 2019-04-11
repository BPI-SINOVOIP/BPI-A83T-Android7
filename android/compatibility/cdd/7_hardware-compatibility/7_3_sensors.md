## 7.3\. Sensors

Android includes APIs for accessing a variety of sensor types. Devices
implementations generally MAY omit these sensors, as provided for in the
following subsections. If a device includes a particular sensor type that has a
corresponding API for third-party developers, the device implementation MUST
implement that API as described in the Android SDK documentation and the
Android Open Source documentation on
[sensors](http://source.android.com/devices/sensors/). For example, device
implementations:

*   MUST accurately report the presence or absence of sensors per the
[android.content.pm.PackageManager](http://developer.android.com/reference/android/content/pm/PackageManager.html)
class.
*   MUST return an accurate list of supported sensors via the
SensorManager.getSensorList() and similar methods.
*   MUST behave reasonably for all other sensor APIs (for example, by returning
true or false as appropriate when applications attempt to register listeners,
not calling sensor listeners when the corresponding sensors are not present;
etc.).
*   MUST [report all sensor measurements](http://developer.android.com/reference/android/hardware/SensorEvent.html)
using the relevant International System of Units (metric) values for each
sensor type as defined in the Android SDK documentation.
*   SHOULD [report the event time](http://developer.android.com/reference/android/hardware/SensorEvent.html#timestamp)
in nanoseconds as defined in the Android SDK documentation, representing the
time the event happened and synchronized with the
SystemClock.elapsedRealtimeNano() clock. Existing and new Android devices are
**STRONGLY RECOMMENDED** to meet these requirements so they will be able to
upgrade to the future platform releases where this might become a REQUIRED
component. The synchronization error SHOULD be below 100 milliseconds.
*   MUST report sensor data with a maximum latency of 100 milliseconds + 2 *
sample_time for the case of a sensor streamed with a minimum required latency
of 5 ms + 2 * sample_time when the application processor is active. This delay
does not include any filtering delays.
*   MUST report the first sensor sample within 400 milliseconds + 2 *
sample_time of the sensor being activated. It is acceptable for this sample to
have an accuracy of 0.

The list above is not comprehensive; the documented behavior of the Android SDK
and the Android Open Source Documentations on
[sensors](http://source.android.com/devices/sensors/) is to be considered
authoritative.

Some sensor types are composite, meaning they can be derived from data provided
by one or more other sensors. (Examples include the orientation sensor and the
linear acceleration sensor.) Device implementations SHOULD implement these
sensor types, when they include the prerequisite physical sensors as described
in [sensor types](https://source.android.com/devices/sensors/sensor-types.html). If a
device implementation includes a composite sensor it MUST implement the sensor
as described in the Android Open Source documentation on
[composite sensors](https://source.android.com/devices/sensors/sensor-types.html#composite_sensor_type_summary).

Some Android sensors support a [“continuous” trigger mode](https://source.android.com/devices/sensors/report-modes.html#continuous),
which returns data continuously. For any API indicated by the Android SDK
documentation to be a continuous sensor, device implementations MUST
continuously provide periodic data samples that SHOULD have a jitter below 3%,
where jitter is defined as the standard deviation of the difference of the
reported timestamp values between consecutive events.

Note that the device implementations MUST ensure that the sensor event stream
MUST NOT prevent the device CPU from entering a suspend state or waking up from
a suspend state.

Finally, when several sensors are activated, the power consumption SHOULD NOT
exceed the sum of the individual sensor’s reported power consumption.

### 7.3.1\. Accelerometer

Device implementations SHOULD include a 3-axis accelerometer. Android Handheld
devices, Android Automotive implementations, and Android Watch devices are STRONGLY
RECOMMENDED to include this sensor. If a device implementation does include a
3-axis accelerometer, it:

*   MUST implement and report
[TYPE_ACCELEROMETER sensor](http://developer.android.com/reference/android/hardware/Sensor.html#TYPE_ACCELEROMETER).
*   MUST be able to report events up to a frequency of at least 50 Hz for
Android Watch devices as such devices have a stricter power constraint and 100
Hz for all other device types.
*   SHOULD report events up to at least 200 Hz.
*   MUST comply with the [Android sensor coordinate system](http://developer.android.com/reference/android/hardware/SensorEvent.html)
as detailed in the Android APIs. Android Automotive implementations MUST comply
with the Android [car sensor coordinate system](http://source.android.com/devices/sensors/sensor-types.html#auto_axes).
*   MUST be capable of measuring from freefall up to four times the gravity
(4g) or more on any axis.
*   MUST have a resolution of at least 12-bits and SHOULD have a resolution of
at least 16-bits.
*   SHOULD be calibrated while in use if the characteristics changes over the
life cycle and compensated, and preserve the compensation parameters between
device reboots.
*   SHOULD be temperature compensated.
*   MUST have a standard deviation no greater than 0.05 m/s^, where the
standard deviation should be calculated on a per axis basis on samples
collected over a period of at least 3 seconds at the fastest sampling rate.
*   SHOULD implement the TYPE_SIGNIFICANT_MOTION, TYPE_TILT_DETECTOR,
TYPE_STEP_DETECTOR, TYPE_STEP_COUNTER composite sensors as described in the
Android SDK document. Existing and new Android devices are **STRONGLY
RECOMMENDED** to implement the TYPE_SIGNIFICANT_MOTION composite sensor. If any
of these sensors are implemented, the sum of their power consumption MUST
always be less than 4 mW and SHOULD each be below 2 mW and 0.5 mW for when the
device is in a dynamic or static condition.
*   If a gyroscope sensor is included, MUST implement the TYPE_GRAVITY and
TYPE_LINEAR_ACCELERATION composite sensors and SHOULD implement the
TYPE_GAME_ROTATION_VECTOR composite sensor. Existing and new Android devices
are STRONGLY RECOMMENDED to implement the TYPE_GAME_ROTATION_VECTOR sensor.
*   MUST implement a TYPE_ROTATION_VECTOR composite sensor, if a gyroscope
sensor and a magnetometer sensor is also included.

### 7.3.2\. Magnetometer

Device implementations SHOULD include a 3-axis magnetometer (compass). If a
device does include a 3-axis magnetometer, it:

*   MUST implement the TYPE_MAGNETIC_FIELD sensor and SHOULD also implement
TYPE_MAGNETIC_FIELD_UNCALIBRATED sensor. Existing and new Android devices are
STRONGLY RECOMMENDED to implement the TYPE_MAGNETIC_FIELD_UNCALIBRATED sensor.
*   MUST be able to report events up to a frequency of at least 10 Hz and
SHOULD report events up to at least 50 Hz.
*   MUST comply with the [Android sensor coordinate system](http://developer.android.com/reference/android/hardware/SensorEvent.html)
as detailed in the Android APIs.
*   MUST be capable of measuring between -900 µT and +900 µT on each axis
before saturating.
*   MUST have a hard iron offset value less than 700 µT and SHOULD have a value
below 200 µT, by placing the magnetometer far from dynamic (current-induced)
and static (magnet-induced) magnetic fields.
*   MUST have a resolution equal or denser than 0.6 µT and SHOULD have a
resolution equal or denser than 0.2 µT.
*   SHOULD be temperature compensated.
*   MUST support online calibration and compensation of the hard iron bias, and
preserve the compensation parameters between device reboots.
*   MUST have the soft iron compensation applied—the calibration can be done
either while in use or during the production of the device.
*   SHOULD have a standard deviation, calculated on a per axis basis on samples
collected over a period of at least 3 seconds at the fastest sampling rate, no
greater than 0.5 µT.
*   MUST implement a TYPE_ROTATION_VECTOR composite sensor, if an accelerometer
sensor and a gyroscope sensor is also included.
*   MAY implement the TYPE_GEOMAGNETIC_ROTATION_VECTOR sensor if an
accelerometer sensor is also implemented. However if implemented, it MUST
consume less than 10 mW and SHOULD consume less than 3 mW when the sensor is
registered for batch mode at 10 Hz.

### 7.3.3\. GPS

Device implementations SHOULD include a GPS/GNSS receiver. If a device implementation
does include a GPS/GNSS receiver and reports the capability to applications through the
`android.hardware.location.gps` feature flag:

*   It is STRONGLY RECOMMENDED that the device continue to deliver normal GPS/GNSS
    outputs to applications during an emergency phone call and that location output
    not be blocked during an emergency phone call.
*   It MUST support location outputs at a rate of at least 1 Hz when requested via
    `LocationManager#requestLocationUpdate`.
*   It MUST be able to determine the location in open-sky conditions (strong signals,
    negligible multipath, HDOP < 2) within 10 seconds (fast time to first fix), when
    connected to a 0.5 Mbps or faster data speed internet connection. This requirement
    is typically met by the use of some form of Assisted or Predicted GPS/GNSS technique
    to minimize GPS/GNSS lock-on time (Assistance data includes Reference Time, Reference
    Location and Satellite Ephemeris/Clock).
       * After making such a location calculation, it is STRONGLY RECOMMENDED for the device to
         be able to determine its location, in open sky, within 10 seconds, when location
         requests are restarted, up to an hour after the initial location calculation,
         even when the subsequent request is made without a data connection, and/or after a power
         cycle.
*   In open sky conditions after determining the location, while stationary or moving with less
    than 1 meter per second squared of acceleration:
       * It MUST be able to determine location within 20 meters, and speed within 0.5 meters
         per second, at least 95% of the time.
       * It MUST simultaneously track and report via [GnssStatus.Callback](https://developer.android.com/reference/android/location/GnssStatus.Callback.html#GnssStatus.Callback()')
         at least 8 satellites from one constellation.
       * It SHOULD be able to simultaneously track at least 24 satellites, from multiple
         constellations (e.g. GPS + at least one of Glonass, Beidou, Galileo).
*   It MUST report the GNSS technology generation through the test API ‘getGnssYearOfHardware’.
*   It is STRONGLY RECOMMENDED to meet and MUST meet all requirements below if the GNSS technology
    generation is reported as the year "2016" or newer.
       * It MUST report GPS measurements, as soon as they are found, even if a location calculated
         from GPS/GNSS is not yet reported.
       * It MUST report GPS pseudoranges and pseudorange rates, that, in open-sky conditions
         after determining the location, while stationary or moving with less than 0.2 meter
         per second squared of acceleration, are sufficient to calculate position within
         20 meters, and speed within 0.2 meters per second, at least 95% of the time.

Note that while some of the GPS requirements above are stated as STRONGLY RECOMMENDED, the
Compatibility Definition for the next major version is expected to change these to a MUST.

### 7.3.4\. Gyroscope

Device implementations SHOULD include a gyroscope (angular change sensor).
Devices SHOULD NOT include a gyroscope sensor unless a 3-axis accelerometer is
also included. If a device implementation includes a gyroscope, it:

*   MUST implement the TYPE_GYROSCOPE sensor and SHOULD also implement
TYPE_GYROSCOPE_UNCALIBRATED sensor. Existing and new Android devices are
STRONGLY RECOMMENDED to implement the SENSOR_TYPE_GYROSCOPE_UNCALIBRATED
sensor.
*   MUST be capable of measuring orientation changes up to 1,000 degrees per
second.
*   MUST be able to report events up to a frequency of at least 50 Hz for
Android Watch devices as such devices have a stricter power constraint and 100
Hz for all other device types.
*   SHOULD report events up to at least 200 Hz.
*   MUST have a resolution of 12-bits or more and SHOULD have a resolution of
16-bits or more.
*   MUST be temperature compensated.
*   MUST be calibrated and compensated while in use, and preserve the
compensation parameters between device reboots.
*   MUST have a variance no greater than 1e-7 rad^2 / s^2 per Hz (variance per
Hz, or rad^2 / s). The variance is allowed to vary with the sampling rate, but
must be constrained by this value. In other words, if you measure the variance
of the gyro at 1 Hz sampling rate it should be no greater than 1e-7 rad^2/s^2.
*   MUST implement a TYPE_ROTATION_VECTOR composite sensor, if an accelerometer
sensor and a magnetometer sensor is also included.
*   If an accelerometer sensor is included, MUST implement the TYPE_GRAVITY and
TYPE_LINEAR_ACCELERATION composite sensors and SHOULD implement the
TYPE_GAME_ROTATION_VECTOR composite sensor. Existing and new Android devices
are STRONGLY RECOMMENDED to implement the TYPE_GAME_ROTATION_VECTOR sensor.

### 7.3.5\. Barometer

Device implementations SHOULD include a barometer (ambient air pressure
sensor). If a device implementation includes a barometer, it:

*   MUST implement and report TYPE_PRESSURE sensor.
*   MUST be able to deliver events at 5 Hz or greater.
*   MUST have adequate precision to enable estimating altitude.
*   MUST be temperature compensated.

### 7.3.6\. Thermometer

Device implementations MAY include an ambient thermometer (temperature sensor).
If present, it MUST be defined as SENSOR_TYPE_AMBIENT_TEMPERATURE and it MUST
measure the ambient (room) temperature in degrees Celsius.

Device implementations MAY but SHOULD NOT include a CPU temperature sensor. If
present, it MUST be defined as SENSOR_TYPE_TEMPERATURE, it MUST measure the
temperature of the device CPU, and it MUST NOT measure any other temperature.
Note the SENSOR_TYPE_TEMPERATURE sensor type was deprecated in Android 4.0.

<div class="note">

For Android Automotive implementations, SENSOR_TYPE_AMBIENT_TEMPERATURE MUST
measure the temperature inside the vehicle cabin.

</div>

### 7.3.7\. Photometer

Device implementations MAY include a photometer (ambient light sensor).

### 7.3.8\. Proximity Sensor

Device implementations MAY include a proximity sensor. Devices that can make a
voice call and indicate any value other than PHONE_TYPE_NONE in getPhoneType
SHOULD include a proximity sensor. If a device implementation does include a
proximity sensor, it:

*   MUST measure the proximity of an object in the same direction as the
screen. That is, the proximity sensor MUST be oriented to detect objects close
to the screen, as the primary intent of this sensor type is to detect a phone
in use by the user. If a device implementation includes a proximity sensor with
any other orientation, it MUST NOT be accessible through this API.
*   MUST have 1-bit of accuracy or more.

### 7.3.9\. High Fidelity Sensors

Device implementations supporting a set of higher quality sensors that can meet
all the requirements listed in this section MUST identify the support through
the `android.hardware.sensor.hifi_sensors` feature flag.

A device declaring android.hardware.sensor.hifi_sensors MUST support all of the
following sensor types meeting the quality requirements as below:

*   SENSOR_TYPE_ACCELEROMETER
    *   MUST have a measurement range between at least -8g and +8g.
    *   MUST have a measurement resolution of at least 1024 LSB/G.
    *   MUST have a minimum measurement frequency of 12.5 Hz or lower.
    *   MUST have a maximum measurement frequency of 400 Hz or higher.
    *   MUST have a measurement noise not above 400 uG/√Hz.
    *   MUST implement a non-wake-up form of this sensor with a buffering
        capability of at least 3000 sensor events.
    *   MUST have a batching power consumption not worse than 3 mW.
    *   SHOULD have a stationary noise bias stability of \<15 μg √Hz from 24hr static
        dataset.
    *   SHOULD have a bias change vs. temperature of ≤ +/- 1mg / °C.
    *   SHOULD have a best-fit line non-linearity of ≤ 0.5%, and sensitivity change vs. temperature of ≤
        0.03%/C°.
*   SENSOR_TYPE_GYROSCOPE
    *   MUST have a measurement range between at least -1000 and +1000 dps.
    *   MUST have a measurement resolution of at least 16 LSB/dps.
    *   MUST have a minimum measurement frequency of 12.5 Hz or lower.
    *   MUST have a maximum measurement frequency of 400 Hz or higher.
    *   MUST have a measurement noise not above 0.014°/s/√Hz.
    *   SHOULD have a stationary bias stability of < 0.0002 °/s √Hz from 24-hour static dataset.
    *   SHOULD have a bias change vs. temperature of ≤ +/- 0.05 °/ s / °C.
    *   SHOULD have a sensitivity change vs. temperature of ≤ 0.02% / °C.
    *   SHOULD have a best-fit line non-linearity of ≤ 0.2%.
    *   SHOULD have a noise density of ≤ 0.07 °/s/√Hz.

*   SENSOR_TYPE_GYROSCOPE_UNCALIBRATED with the same quality requirements as
    SENSOR_TYPE_GYROSCOPE.
*   SENSOR_TYPE_GEOMAGNETIC_FIELD
    *   MUST have a measurement range between at least -900 and +900 uT.
    *   MUST have a measurement resolution of at least 5 LSB/uT.
    *   MUST have a minimum measurement frequency of 5 Hz or lower.
    *   MUST have a maximum measurement frequency of 50 Hz or higher.
    *   MUST have a measurement noise not above 0.5 uT.
*   SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED with the same quality requirements
    as SENSOR_TYPE_GEOMAGNETIC_FIELD and in addition:
    *   MUST implement a non-wake-up form of this sensor with a buffering
        capability of at least 600 sensor events.
*   SENSOR_TYPE_PRESSURE
    *   MUST have a measurement range between at least 300 and 1100 hPa.
    *   MUST have a measurement resolution of at least 80 LSB/hPa.
    *   MUST have a minimum measurement frequency of 1 Hz or lower.
    *   MUST have a maximum measurement frequency of 10 Hz or higher.
    *   MUST have a measurement noise not above 2 Pa/√Hz.
    *   MUST implement a non-wake-up form of this sensor with a buffering
        capability of at least 300 sensor events.
    *   MUST have a batching power consumption not worse than 2 mW.
*   SENSOR_TYPE_GAME_ROTATION_VECTOR
    *   MUST implement a non-wake-up form of this sensor with a buffering
        capability of at least 300 sensor events.
    *   MUST have a batching power consumption not worse than 4 mW.
*   SENSOR_TYPE_SIGNIFICANT_MOTION
    *   MUST have a power consumption not worse than 0.5 mW when device is
        static and 1.5 mW when device is moving.
*   SENSOR_TYPE_STEP_DETECTOR
    *   MUST implement a non-wake-up form of this sensor with a buffering
        capability of at least 100 sensor events.
    *   MUST have a power consumption not worse than 0.5 mW when device is
        static and 1.5 mW when device is moving.
    *   MUST have a batching power consumption not worse than 4 mW.
*   SENSOR_TYPE_STEP_COUNTER
    *   MUST have a power consumption not worse than 0.5 mW when device is
        static and 1.5 mW when device is moving.
*   SENSOR_TILT_DETECTOR
    *   MUST have a power consumption not worse than 0.5 mW when device is
        static and 1.5 mW when device is moving.

Also such a device MUST meet the following sensor subsystem requirements:

*   The event timestamp of the same physical event reported by the
Accelerometer, Gyroscope sensor and Magnetometer MUST be within 2.5
milliseconds of each other.
*   The Gyroscope sensor event timestamps MUST be on the same time base as the
camera subsystem and within 1 milliseconds of error.
*   High Fidelity sensors MUST deliver samples to applications within 5
milliseconds from the time when the data is available on the physical sensor
to the application.
*   The power consumption MUST not be higher than 0.5 mW when device is static
and 2.0 mW when device is moving when any combination of the following sensors
are enabled:
    *   SENSOR_TYPE_SIGNIFICANT_MOTION
    *   SENSOR_TYPE_STEP_DETECTOR
    *   SENSOR_TYPE_STEP_COUNTER
    *   SENSOR_TILT_DETECTORS

Note that all power consumption requirements in this section do not include the
power consumption of the Application Processor. It is inclusive of the power
drawn by the entire sensor chain—the sensor, any supporting circuitry, any
dedicated sensor processing system, etc.

The following sensor types MAY also be supported on a device implementation
declaring android.hardware.sensor.hifi_sensors, but if these sensor types are
present they MUST meet the following minimum buffering capability requirement:

*   SENSOR_TYPE_PROXIMITY: 100 sensor events

### 7.3.10\. Fingerprint Sensor

Device implementations with a secure lock screen SHOULD include a fingerprint
sensor. If a device implementation includes a fingerprint sensor and has a
corresponding API for third-party developers, it:

*   MUST declare support for the android.hardware.fingerprint feature.
*   MUST fully implement the
[corresponding API](https://developer.android.com/reference/android/hardware/fingerprint/package-summary.html)
as described in the Android SDK documentation.
*   MUST have a false acceptance rate not higher than 0.002%.
*   Is STRONGLY RECOMMENDED to have a false rejection rate of less than 10%, as
measured on the device
*   Is STRONGLY RECOMMENDED to have a latency below 1 second, measured from
when the fingerprint sensor is touched until the screen is unlocked, for one
enrolled finger.
*   MUST rate limit attempts for at least 30 seconds after five false trials
for fingerprint verification.
*   MUST have a hardware-backed keystore implementation, and perform the
fingerprint matching in a Trusted Execution Environment (TEE) or on a chip with
a secure channel to the TEE.
*   MUST have all identifiable fingerprint data encrypted and cryptographically
authenticated such that they cannot be acquired, read or altered outside of the
Trusted Execution Environment (TEE) as documented in the
[implementation guidelines](https://source.android.com/devices/tech/security/authentication/fingerprint-hal.html)
on the Android Open Source Project site.
*   MUST prevent adding a fingerprint without first establishing a chain of
trust by having the user confirm existing or add a new device credential
(PIN/pattern/password) using the TEE as implemented in the Android Open Source
project.
*   MUST NOT enable 3rd-party applications to distinguish between individual
fingerprints.
*   MUST honor the DevicePolicyManager.KEYGUARD_DISABLE_FINGERPRINT flag.
*   MUST, when upgraded from a version earlier than Android 6.0, have the
fingerprint data securely migrated to meet the above requirements or removed.
*   SHOULD use the Android Fingerprint icon provided in the Android Open Source
Project.

### 7.3.11\. Android Automotive-only sensors

Automotive-specific sensors are defined in the `android.car.CarSensorManager API`.


#### 7.3.11.1\. Current Gear

Android Automotive implementations SHOULD provide current gear as SENSOR_TYPE_GEAR.

#### 7.3.11.2\. Day Night Mode

Android Automotive implementations MUST support day/night mode defined as
SENSOR_TYPE_NIGHT. The value of this flag MUST be consistent with dashboard
day/night mode and SHOULD be based on ambient light sensor input. The
underlying ambient light sensor MAY be the same as
[Photometer](#7_3_7_photometer).

#### 7.3.11.3\. Driving Status

Android Automotive implementations MUST support driving status defined as
SENSOR_TYPE_DRIVING_STATUS, with a default value of DRIVE_STATUS_UNRESTRICTED
when the vehicle is fully stopped and parked. It is the responsibility of device
manufacturers to configure SENSOR_TYPE_DRIVING_STATUS in compliance with all
laws and regulations that apply to markets where the product is shipping.

#### 7.3.11.4\. Wheel Speed

Android Automotive implementations MUST provide vehicle speed defined as
SENSOR_TYPE_CAR_SPEED.

## 7.3.12\. Pose Sensor

Device implementations MAY support pose sensor with 6 degrees of freedom. Android Handheld
devices are RECOMMENDED to support this sensor. If a device implementation does support pose
sensor with 6 degrees of freedom, it:

*   MUST implement and report [`TYPE_POSE_6DOF`](https://developer.android.com/reference/android/hardware/Sensor.html#TYPE_POSE_6DOF)
    sensor.
*   MUST be more accurate than the rotation vector alone.

