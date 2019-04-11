## 5.8\. Secure Media

Device implementations that support secure video output and are capable of
supporting secure surfaces MUST declare support for Display.FLAG_SECURE. Device
implementations that declare support for Display.FLAG_SECURE, if they support a
wireless display protocol, MUST secure the link with a cryptographically strong
mechanism such as HDCP 2.x or higher for Miracast wireless displays. Similarly
if they support a wired external display, the device implementations MUST
support HDCP 1.2 or higher. Android Television device implementations MUST
support HDCP 2.2 for devices supporting 4K resolution and HDCP 1.4 or above for
lower resolutions. The upstream Android open source implementation includes
support for wireless (Miracast) and wired (HDMI) displays that satisfies this
requirement.
