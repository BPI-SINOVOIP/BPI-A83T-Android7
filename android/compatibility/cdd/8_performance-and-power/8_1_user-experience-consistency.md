## 8.1\. User Experience Consistency

Device implementations MUST provide a smooth user interface by ensuring a
consistent frame rate and response times for applications and games. Device
implementations MUST meet the following requirements:

*   **Consistent frame latency**. Inconsistent frame latency or a delay to
render frames MUST NOT happen more often than 5 frames in a second, and SHOULD
be below 1 frames in a second.
*   **User interface latency**. Device implementations MUST ensure low latency
user experience by scrolling a list of 10K list entries as defined by the
Android Compatibility Test Suite (CTS) in less than 36 secs.
*   **Task switching**. When multiple applications have been launched,
re-launching an already-running application after it has been launched MUST
take less than 1 second.

