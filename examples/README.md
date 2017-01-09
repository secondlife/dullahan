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

Google News. [(Original output)](../docs/googlenews.png)

![Screen shot:](../docs/googlenews_thumb.png?raw=true "http://news.google.com")



