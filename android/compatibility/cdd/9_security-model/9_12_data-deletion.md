## 9.12\. Data Deletion

Devices MUST provide users with a mechanism to perform a "Factory Data Reset"
that allows logical and physical deletion of all data except for the following:

   * The system image
   * Any operating system files required by the system image

All user-generated data MUST be deleted. This MUST satisfy relevant industry
standards for data deletion such as NIST SP800-88\. This MUST be used for the
implementation of the wipeData() API (part of the Android Device Administration
API) described in [section 3.9 Device Administration](#3_9_device_administration).

Devices MAY provide a fast data wipe that conducts a logical data erase.
