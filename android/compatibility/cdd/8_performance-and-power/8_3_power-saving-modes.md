## 8.3\. Power-Saving Modes

Android 6.0 introduced App Standby and Doze power-saving modes to optimize
battery usage.  All Apps exempted from these modes MUST be made visible to the
end user. Further, the triggering, maintenance, wakeup algorithms and the use of
global system settings of these power-saving modes MUST not deviate from the
Android Open Source Project.

In addition to the power-saving modes, Android device implementations MAY
implement any or all of the 4 sleeping power states as defined by the Advanced
Configuration and Power Interface (ACPI), but if it implements S3 and S4
power states, it can only enter these states when closing a lid that is
physically part of the device.
