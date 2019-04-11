## 3.4\. Web Compatibility

### 3.4.1\. WebView Compatibility

<div class="note">
Android Watch devices MAY, but all other device implementations MUST provide a
complete implementation of the android.webkit.Webview API.
</div>

The platform feature android.software.webview MUST be reported on any device
that provides a complete implementation of the android.webkit.WebView API, and
MUST NOT be reported on devices without a complete implementation of the API.
The Android Open Source implementation uses code from the Chromium Project to
implement the
[android.webkit.WebView](http://developer.android.com/reference/android/webkit/WebView.html).
Because it is not feasible to develop a comprehensive test suite for a web
rendering system, device implementers MUST use the specific upstream build of
Chromium in the WebView implementation. Specifically:

*   Device android.webkit.WebView implementations MUST be based on the
    [Chromium](http://www.chromium.org/) build from the upstream Android Open
    Source Project for Android ANDROID_VERSION. This build includes a specific
    set of functionality and security fixes for the WebView.
*   The user agent string reported by the WebView MUST be in this format:

    Mozilla/5.0 (Linux; Android $(VERSION); $(MODEL) Build/$(BUILD); wv)
    AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 $(CHROMIUM_VER) Mobile
    Safari/537.36

    *   The value of the $(VERSION) string MUST be the same as the value for android.os.Build.VERSION.RELEASE.
    *   The value of the $(MODEL) string MUST be the same as the value for android.os.Build.MODEL.
    *   The value of the $(BUILD) string MUST be the same as the value for android.os.Build.ID.
    *   The value of the $(CHROMIUM_VER) string MUST be the version of Chromium in the upstream Android Open Source Project.
    *   Device implementations MAY omit Mobile in the user agent string.

The WebView component SHOULD include support for as many HTML5 features as
possible and if it supports the feature SHOULD conform to the 
[HTML5 specification](http://html.spec.whatwg.org/multipage/).

### 3.4.2\. Browser Compatibility

<div class="note">

Android Television, Watch, and Android Automotive implementations MAY omit a
browser application, but MUST support the public intent patterns as described in
<a href="#3_2_3_1_core_application_intents">section 3.2.3.1</a>. All other types of device
implementations MUST include a standalone Browser application for general user
web browsing.

</div>

The standalone Browser MAY be based on a browser technology other than WebKit.
However, even if an alternate Browser application is used, the
android.webkit.WebView component provided to third-party applications MUST be
based on WebKit, as described in [section 3.4.1](#3_4_1_webview_compatibility).

Implementations MAY ship a custom user agent string in the standalone Browser application.

The standalone Browser application (whether based on the upstream WebKit Browser
application or a third-party replacement) SHOULD include support for as much of
[HTML5](http://html.spec.whatwg.org/multipage/) as possible. Minimally, device
implementations MUST support each of these APIs associated with HTML5:

*   [application cache/offline operation](http://www.w3.org/html/wg/drafts/html/master/browsers.html#offline)
*   [&lt;video&gt; tag](http://www.w3.org/html/wg/drafts/html/master/semantics.html#video)
*   [geolocation](http://www.w3.org/TR/geolocation-API/)

Additionally, device implementations MUST support the HTML5/W3C
[webstorage API](http://www.w3.org/TR/webstorage/) and SHOULD support the HTML5/W3C
[IndexedDB API](http://www.w3.org/TR/IndexedDB/). Note that as the web
development standards bodies are transitioning to favor IndexedDB over
webstorage, IndexedDB is expected to become a required component in a future
version of Android.
