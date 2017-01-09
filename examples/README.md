## Dullahan Examples

Some examples of how you might Dullahan. None of them are meant to be full featured, solid applications - just snippets of how you might do something in your own code.

Contributions showing off different features or ideas would be much appreciated.

## Web Cube

Slightly more complex OpenGL application using native Win32 because some of the calls to CEF (and therefore Dullahan) require native Windows messages.

![Screen shot:](../docs/webcube.png?raw=true "Web Cube example")

Notes:

* Displays same URL on each side of a cube
  * It would be easy to add a different one to each side but make the code longer
  * Start page is a local HTML page in `tools` folder
* Move the cube by holding down SHIFT key and using mouse and mouse wheel
* Interact with pages via mouse and keyboard (somewhat) normally
* Press ESC key to exit
* Enter any URL into the top edit control
* Expose some features via menu - e.g. Print to PDF

## Console

Command line application to render a given URL to a bitmap and save it as a file called  `output.bmp` in the current directory when the page finishes loading.

![Screen shot:](../docs/console.png?raw=true "Console example")

Notes:

* Pass a URL as command line parameter to render it
* Some sites are not fully complete when they complete loading (when the application saves the bitmap)

Example output from the `console` example:

[Google News](http://news.google.com) [(Original output)](../docs/googlenews.png)

![Screen shot:](../docs/googlenews_thumb.png?raw=true)

[Hacker News](http://news.ycombinator.com) [(Original output)](../docs/hackernews.png)

![Screen shot:](../docs/hackernews_thumb.png?raw=true)

[HTML5 Test](http://html5test.com) [(Original output)](../docs/html5test.png)

![Screen shot:](../docs/html5test_thumb.png?raw=true)

[PDR renderer](https://www.irs.gov/pub/irs-pdf/fw4.pdf) [(Original output)](../docs/pdf.png)

![Screen shot:](../docs/pdf_thumb.png?raw=true)

[Google Streetview](https://www.google.com/maps/place/La+Rambla,+08002+Barcelona,+Spain/@41.3852695,2.1700326,3a,75y,129.04h,84.9t/data=!3m7!1e1!3m5!1sUD8PPXaeTUpTf8LKBXPcMQ!2e0!6s%2F%2Fgeo0.ggpht.com%2Fcbk%3Fpanoid%3DUD8PPXaeTUpTf8LKBXPcMQ%26output%3Dthumbnail%26cb_client%3Dsearch.TACTILE.gps%26thumb%3D2%26w%3D392%26h%3D106%26yaw%3D335.08578%26pitch%3D0!7i13312!8i6656!4m5!3m4!1s0x12a4a258081714c9:0x24bef7ac8842bdc4!8m2!3d41.3806562!4d2.1738529) [(Original output)](../docs/streetview.png)

![Screen shot:](../docs/streetview_thumb.png?raw=true)

[WebGL](http://alteredqualia.com/xg/examples/car_zastava.html) [(Original output)](../docs/webgl.png)

![Screen shot:](../docs/webgl_thumb.png?raw=true)

[YouTube](https://www.youtube.com/watch?v=79DijItQXMM) [(Original output)](../docs/youtube.png)

![Screen shot:](../docs/youtube_thumb.png?raw=true)
