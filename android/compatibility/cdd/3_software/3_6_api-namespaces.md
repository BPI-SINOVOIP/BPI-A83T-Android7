## 3.6\. API Namespaces

Android follows the package and class namespace conventions defined by the Java
programming language. To ensure compatibility with third-party applications,
device implementers MUST NOT make any prohibited modifications (see below) to
these package namespaces:

*   java.*
*   javax.*
*   sun.*
*   android.*
*   com.android.*

**Prohibited modifications include**:

*   Device implementations MUST NOT modify the publicly exposed APIs on the
    Android platform by changing any method or class signatures, or by removing
    classes or class fields.
*   Device implementers MAY modify the underlying implementation of the APIs,
    but such modifications MUST NOT impact the stated behavior and Java-language
    signature of any publicly exposed APIs.
*   Device implementers MUST NOT add any publicly exposed elements (such as
    classes or interfaces, or fields or methods to existing classes or
    interfaces) to the APIs above.

A “publicly exposed element” is any construct that is not decorated with
the“@hide” marker as used in the upstream Android source code. In other words,
device implementers MUST NOT expose new APIs or alter existing APIs in the
namespaces noted above. Device implementers MAY make internal-only
modifications, but those modifications MUST NOT be advertised or otherwise
exposed to developers.

Device implementers MAY add custom APIs, but any such APIs MUST NOT be in a
namespace owned by or referring to another organization. For instance, device
implementers MUST NOT add APIs to the com.google.* or similar namespace: only
Google may do so. Similarly, Google MUST NOT add APIs to other companies'
namespaces. Additionally, if a device implementation includes custom APIs
outside the standard Android namespace, those APIs MUST be packaged in an
Android shared library so that only apps that explicitly use them (via the
&lt;uses-library&gt; mechanism) are affected by the increased memory usage of such
APIs.

If a device implementer proposes to improve one of the package namespaces above
(such as by adding useful new functionality to an existing API, or adding a new
API), the implementer SHOULD visit
[source.android.com](http://source.android.com/) and begin the process for
contributing changes and code, according to the information on that site.

Note that the restrictions above correspond to standard conventions for naming
APIs in the Java programming language; this section simply aims to reinforce
those conventions and make them binding through inclusion in this Compatibility
Definition.
