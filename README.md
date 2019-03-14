# Dullahan

## What is it?

A headless browser SDK that uses the [Chromium Embedded Framework](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework) (CEF). It is designed to make it easier to write applications that render modern web content directly to a memory buffer, inject synthesized mouse and keyboard events as well as interact with features like JavaScript or cookies.

#### **Important note**: 
This is a __very__ early release - more than alpha quality but less than beta. I have high hopes this will develop into something useful but there is still a lot of work to do first.

## What could I use it for?

It might be useful in a number of situations - for example:

* Anywhere you want to render web content in a graphics application - e.g. help pages in an OpenGL based game
* VR based video chat and shared environments (I'm working on an app like this now)
* HTML/JavaScript based user interfaces E.G. a tablet based paradigm for VR
* Integration with JavaScript for automated site testing like [PhantomJS](http://phantomjs.org/)
* Web page contents analysis - E.G:
    * Capture images the most popular 500 web pages and count the top 5 colors in each.
    * Capture a Google Maps traffic map of where you live for 24 hours and see how *really* miserable your commute is :)
* Web page capture to an image or a PDF file
* Lots more I haven't thought of yet

There are some examples included in this repository and there is a description and a screen shot for each [here](./examples/README.md). 

## What is CEF?

From the [CEF Wiki](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework) page: The Chromium Embedded Framework (CEF) is an open source framework for embedding a web browser engine based on the Chromium core. 

An essential site to visit when developing a CEF based application is the [CEF forum](http://magpcss.org/ceforum/) started and run by (I believe) the original CEF author Marshall Greenblatt who is very generous with his time and expertise.

Building CEF locally is quite a dauting task and needs a toolset I do not have on my computer (E.G. Visual Studio 2015) so I used the builds that Spotify kindly hosts [here](http://opensource.spotify.com/cefbuilds/index.html).

## Which version of CEF does Dullahan use?

Dullahan currently builds against the [Spotify Opensource CEF site](http://opensource.spotify.com/cefbuilds/index.html) build of CEF but with some small tweaks to take account of CEF interface changes etc. it should work with slightly older or more recent versions too. For the exact version in use, you can inspect one of the [build batch files](https://bitbucket.org/lindenlab/dullahan/src/default/tools/build_win.bat?at=default&fileviewer=file-view-default#build_win.bat-10).  files.

## What are the origins of this project?

Historically, the first incarnation was [LLCEFLib](https://bitbucket.org/lindenlab/3p-llceflib) - a [Linden Lab](http://lindenlab.com) internal library to render web content content inside [Second Life](http://secondlife.com). LLCEFLib had to support the outdated 32bit macOS version as well as many Second Life specific features so the code became very disjointed and messy. This effort is an attempt to unburden the API from those requirements and provide a more generic version which can be useful in other applications.

## Which platforms are supported?

* Windows 32/64 bit (primary development platform)
* macOS 64 bit (works, is supported but needs a little work - any macOS devs out there want to help?)
* Linux 64 bit (not available yet but hopefully one day - any Linux devs want to help?)

## Which version of the C++ runtime library does it use on Windows?

For historical reasons, both Dullahan and the version of CEF it consumes are built with the C++ runtime library set to `Multi-threaded DLL` /  `Multi-threaded Debug DLL`. 

## Is there a binary version I can download? ##

Not right now. I used to store a pre-built version on the Windows 64 bit version in my DropBox but that became untenable. Once I figure out a sensible place to store a large binary package, I'll bundle up the latest SDK and examples for all the platforms.

## What are the system requirements to build CEF and Dullahan?

* Windows
    * Microsoft Visual Studio 2013 update 4
    * [CMake](https://cmake.org/) version 3.6.3 or later
    * [7-Zip](www.7-zip.org/) 7-Zip or a similar tool that can uncompress `.tar.bz2` files
* macOS
    * [Xcode](https://developer.apple.com/xcode/) version 10 or later 
    * [CMake](https://cmake.org/) version 3.6.3 or later

## How do I build CEF?

Before you can build Dullahan, you will need to build a version of CEF. There are some scripts to help - a [batch](https://bitbucket.org/lindenlab/dullahan/src/default/tools/make_dullahan_cef_pkg.bat?) file for Windows and a [shell script](https://bitbucket.org/lindenlab/dullahan/src/default/tools/make_dullahan_cef_pkg.sh) for macOS. The macOS version is somewhat automated but the lack of built-in tools in Windows to download and uncompress `.tar.bz2` files means it needs some manual steps up front.

* Windows
    * Grab a Windows 32 or 64 bit build of CEF from [http://opensource.spotify.com/cefbuilds/index.html](http://opensource.spotify.com/cefbuilds/index.html)
    * Extract the archive to a local folder - I use the Windows Desktop
    * Edit the `tools/make_dullahan_cef_pkg.bat` batch file - specifically, the lines at the top of the file that set the `CEF_VERSION` (match the version you downloaded), `SRC_DIR_32/64` (location of uncompressed CEF package you just downloaded) and `DST_DIR_32/64` (location where you want to generate a build).
    * From a command prompt, run `tools\make_dullahan_cef_pkg.bat` with a parameter of either 32 or 64 to set the bit-width to build
    * If all goes well, you will end up with a set of libs, headers and binary files in the `DST_DIR_32/64` folder that you can use to build Dullahan
* macOS
    * Edit the `tools/make_dullahan_cef_pkg.sh` shell script and make sure the `CEF_VERSION` variable is set to the version you want and make sure the `DST_DIR` points to somewhere sensible. This is where the build will be generated.
    * From a terminal prompt, run `tools/make_dullahan_cef_pkg.sh`. This will download the right version of CEF, build it and create a set of libs, frameworks, headers etc. that you need to build Dullahan

## Do I need to build Chromium?

The easy answer is no - CEF builds against [Chromium](https://www.chromium.org/) but the [Spotify](http://opensource.spotify.com/cefbuilds/index.html) builds already do that for you. However, you can build Chromium yourself and then build CEF against that in the same way as Spotify does - you might need to do this if you had to change the way part of Chromium works or more likely, you wanted a version of Chromium that has the proprietary media codec support turned on. The Dullahan project used to do this but the toolset required to build Chromium passed what I had available to me (Visual Studio 2015 is a dependency for recent builds) so we stopped doing it. The remnants of a batch file can be found in [here](https://bitbucket.org/lindenlab/dullahan/src/default/tools/cef_download_build.bat) - some pieces of that may still be useful. There is also a CEF [wiki page](https://bitbucket.org/chromiumembedded/cef/wiki/BranchesAndBuilding.md) that has lots of details too.

## How do I build Dullahan?

* Download or clone the Dullahan source code
* Windows
    * Edit the `tools\build_win.bat` filer and change the `CEF_VERSION` variable to match the one you want to use and the `CEF_32_DIR` and `CEF_64_DIR` variables at the top to point to the appropriate build of CEF you just built.
    * From a command prompt, run `tools/build_win.bat` with a parameter of either 32 or 64 to set the bit-width to build
    * If Dullahan builds correctly, Visual Studio 2013 solution file in `build` or `build64` will open and you can run the examples like `WebCube` in the normal way

* macOS
    * Edit the `tools/build_mac.sh` file and edit the `CEF_VERSION` variable to match the one you want to use and the `CEF_BUILDS_DIR` variable to point to the version of CEF you just built.
    * From a terminal prompt, run `tools/build_mac.sh`. 
    * If it builds correctly, you can try to the run the included macOS example application (`open build64/Release/osxgl.app`)
    * You can open the Xcode project file `build64/dullahan.xcodeproj` and edit files if you want to experiment

## Which files do I need in my own project?

The reason to use Dullahan vs raw CEF in your application is that you merely include the `dullahan.h` header file in your application and link against the `Dullahan`, `libcef` and `libcef_dll_wrapper` static libraries. Then, copy over the various CEF/Chromium runtime files alongside your application - see the post-build step for the examples in Visual Studio or the `build_mac.sh` script for more details.

TODO: Create a batch file and shell script that generates a CMake file which creates an empty project with everything set up correctly.

## How do I use it?

The short answer is look at the code in the `examples` folder and `dullahan.h`. Generally speaking, you create an instance of the library, call `init()` and regularly call the `update()` function in your own message loop. You can hook up a callback to be notified when the contents of the page changes and get access to the raw pixels. You can synthesize mouse and keyboard events and send them into the page. Finally, when you want to exit, make sure the `onRequestExit` callback is hooked up and call the `requestExit()` method. When the library and CEF have cleaned everything up, your callback will be triggered and you can call `shutdown()` before exiting normally.

## Are there examples?

Why yes there is - take a look in the `examples` subdirectory of this repo.  There are some screenshots and short descriptions in the [README](https://bitbucket.org/lindenlab/dullahan/src/default/examples/README.md) file too 

## What is left to do before the first real release?

* Implement proper Dullahan <--> JavaScript interoperability
* Improve the documentation
* Make sure all the features are used in the examples to help understand usage
* Unit tests
* Add support for different pages on each side of the Web Cube example
* Lots and lots more... please help if you can.

## Alternatives

* The most exciting alternative in this space is from Google themselves - [headless Chromium](https://chromium.googlesource.com/chromium/src/+/lkgr/headless/README.md) appears to offer much of what Dullahan does albeit better because they have more than one person working on it :) It's very new and still developing but definitely worth a look and will likely supersede Dullahan one day. 

## Why the name?

[Seemed appropriate](https://www.google.com/search?q=dullahan)
