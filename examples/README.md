## Dullahan Examples

Some examples of how you might Dullahan. None of them are meant to be full featured, solid applications - just snippets of how you might do something in your own code.

Contributions showing off different features or ideas would be much appreciated.

## Console (Windows)

Windows command line application to render a given URL to a bitmap and save it as a bitmap called `output.bmp` and a PDF called `output.pdf` in the current directory when the page finishes loading or stops animating.

![Screen shot:](../docs/console.png?raw=true "Console example")

Notes:

* Pass a URL as command line parameter to render it
* Some sites are not fully ready when they "complete loading" (when the application saves the bitmap)

## Web Cube (Windows)

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

## OSXGL (macOS)

Simple macOS OpenGL app that displays a 2D bitmap rendered by Dullahan. 

![Screen shot:](../docs/osxgl.png?raw=true "OSXGL example")

Notes:

* Needs some work to improve shutdown correctly (any macOS devs want to help?)


## cef_minimal (Windows)

An unusual C++ example that doesn't use Dullahan. Rather, it just initializes CEF directly and renders a page. Used as a test bed for fixing issues - most recently a crash on shutdown - without requiring consumers download and set up Dullahan.

![Screen shot:](../docs/cef_minimal.png?raw=true "cef_minimal example")

Notes:

* Works on Windows but could be made to work on macOS very easily by replacing the message loop