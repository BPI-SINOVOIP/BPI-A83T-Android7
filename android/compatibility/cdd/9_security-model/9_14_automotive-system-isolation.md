## 9.14\. Automotive Vehicle System Isolation

Android Automotive devices are expected to exchange data with critical vehicle
subsystems, e.g., by using the [vehicle HAL](http://source.android.com/devices/automotive.html)
to send and receive messages over vehicle networks such as CAN bus. Android
Automotive device implementations MUST implement security features below the
Android framework layers to prevent malicious or unintentional interaction
between the Android framework or third-party apps and vehicle subsystems. These
security features are as follows:

* Gatekeeping messages from Android framework vehicle subsystems, e.g.,
  whitelisting permitted message types and message sources.
* Watchdog against denial of service attacks from the Android framework or
  third-party apps. This guards against malicious software flooding the vehicle
  network with traffic, which may lead to malfunctioning vehicle subsystems.
