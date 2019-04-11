## 3.14\. Vehicle UI APIs

### 3.14.1\.  Vehicle Media UI

Any device implementation that [declares automotive support](https://developer.android.com/reference/android/content/pm/PackageManager.html?#FEATURE_AUTOMOTIVE?)
MUST include a UI framework to support third-party apps consuming the
[MediaBrowser](http://developer.android.com/reference/android/media/browse/MediaBrowser.html)
and
[MediaSession](http://developer.android.com/reference/android/media/session/MediaSession.html)
APIs.

The UI framework supporting third-party apps that depend on MediaBrowser and
MediaSession has the following visual requirements:

* MUST display
  [MediaItem](http://developer.android.com/reference/android/media/browse/MediaBrowser.MediaItem.html)
  icons and notification icons unaltered.
* MUST display those items as described by MediaSession, e.g., metadata, icons,
  imagery.
* MUST show app title.
* MUST have drawer to present [MediaBrowser](http://developer.android.com/reference/android/media/browse/MediaBrowser.html)
  hierarchy.
