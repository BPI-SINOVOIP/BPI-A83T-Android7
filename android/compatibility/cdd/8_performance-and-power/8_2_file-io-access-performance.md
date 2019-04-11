## 8.2\. File I/O Access Performance

Device implementations MUST ensure internal storage file access performance
consistency for read and write operations.

*   **Sequential write**. Device implementations MUST ensure a sequential write
performance of at least 5MB/s for a 256MB file using 10MB write buffer.
*   **Random write**. Device implementations MUST ensure a random write
performance of at least 0.5MB/s for a 256MB file using 4KB write buffer.
*   **Sequential read**. Device implementations MUST ensure a sequential read
performance of at least 15MB/s for a 256MB file using 10MB write buffer.
*   **Random read**. Device implementations MUST ensure a random read
performance of at least 3.5MB/s for a 256MB file using 4KB write buffer.

