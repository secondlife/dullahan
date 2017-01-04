# Dullahan

## What is it?

A headless browser SDK that uses the [Chromium Embedded Framework](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework) (CEF). It is designed to make it easier to write applications that render modern web content directly to a memory buffer, inject synthesized mouse and keyboard events as well as interact with web based features like JavaScript or cookies.

#### **Important note**: 
This is __very__ much an alpha release / first code drop. I have high hopes this will develop into something useful but there is still a lot of work to do first.

## What could I use it for?

It might be useful in a number of situations - for example:

* Anywhere you want to render web content in a graphics application - e.g. help pages in an OpenGL based game
* VR based video chat and shared environments (I'm working on an app like this now)
* HTML/JavaScript based user interfaces E.G. a tablet based paradigm for VR
* Integration with JavaScript for automated site testing like [PhantomJS](http://phantomjs.org/) - not there at all yet but that's one of the main plans for the future
* Web page contents analysis - E.G:
  * Capture images the top 500 web pages and count the top 5 colors in each.
  * Capture a Google Maps traffic map of your commute for 24 hours and see how miserable the commute is :)
* Web page capture to an image or a PDF file
* Lots more I haven't thought of yet

There are some examples included in this repository and there is a description and a screenshot for each [here](./examples/README.md). 

## What is CEF?

From the [CEF Wiki](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework) page: The Chromium Embedded Framework (CEF) is an open source framework for embedding a web browser engine based on the Chromium core. 

An essential site to visit when developing a CEF based application is the [CEF forum](http://magpcss.org/ceforum/) started and run by (I believe) the original CEF author Marshall Greenblatt who is very generous with his time and expertise.

Building CEF locally is quite a task so Adobe kindly host builds for all supported platforms [here](http://cefbuilds.com).

## Which version of CEF does Dullahan use?

Dullahan been developed against the [CEF 3.2704](https://cefbuilds.com) build of CEF but with some small tweaks to take account of CEF interface changes etc. it should work with slightly older or more recent versions too.

## What are the origins of this project?

Historically, the first incarnation was [LLCEFLib](https://bitbucket.org/lindenlab/3p-llceflib) - a [Linden Lab](http://lindenlab.com) internal library to render web content content inside [Second Life](http://secondlife.com). LLCEFLib had to support the outdated 32bit macOS version as well as many Second Life specific features so things became very disjointed and messy. This effort is an attempt to unburden the API from those requirements and provide a more generic version and hopefully useful in other applications.

## Is Windows 32/64bit the only supported platform?

For the moment yes. The 32bit version for macOS is no longer supported by CEF. One day, there will be a 64bit macOS version will be added too. No plans for a Linux version since I have no easy way to develop for Linux but if you'd like to help, please let me know.

## Which version of the C++ runtime library does it use?

For historical reasons, both it and the version of CEF in this repository are built with the C++ runtime library set to `Multi-threaded DLL` /  `Multi-threaded Debug DLL`. 

## How do I build Dullahan?

* Download or clone the Dullahan source code
* Build or download the version of CEF you want to use (See below)
* Ensure Microsoft Visual Studio 2013 (update 4) is installed (it might work with other versions but that is untested)
* Make sure [CMake](https://cmake.org/) version 3.6.3 or later is installed on your system
* Note: a rudimentary batch file that runs these steps automatically can be found in `tools\build_win.bat` - otherwise, follow the manual steps below
* Create a new directory off of the project root `mkdir build` and switch to it `cd build`
* Use CMake to generate project files `cmake -G "Visual Studio 12 2013" -DCEF_DIR="c:\work\cef_builds\cef_2704.1434.win32" ..` where `CEF_DIR` points to the absolute path of a version of CEF that you built or downloaded
* Note: For 64 bit builds, invoke CMake like this `cmake -G "Visual Studio 12 2013 Win64" -DCEF_DIR="c:\work\cef_builds\cef_2704.1434.win64" ..`
* Either open the resulting `dullahan.sln` in Visual Studio as normal or build everything on the command line using `msbuild dullahan.sln /p:Configuration=Release`
* Switch to the output directory `cd Release` and run the example `.\webcube`
* The Dullahan header you need can be found in `src` directory
* The Dullahan library file you need can be found in `build\Release | Debug` directory

## Which files do I need in my own project?

The main benefit of using Dullahan vs raw CEF is that you simply include the `dullahan.h` header file in your application and link against the `dullahan.lib`, `libcef.lib` and `libcef_dll_wrapper.lib`. Then, copy over the various CEF/Chromium runtime files alongside your application - see the post-build step for the examples in Visual Studio for a list - and you are ready to go.

## How do I use it?

The short answer is look at the code in the `examples` folder and `dullahan.h`. Generally speaking, you create an instance of the library, call `init()` and either (a) regularly call the `update()` function in your own message loop or the `run()` function if you don't have a message loop and want to use CEFs (e.g. a console application to save a web page as a bitmap). You can hook up a callback to be notified when the contents of the page changes and get access to the raw pixels. You can synthesize mouse and keyboard events and send them into the page. Finally, when you want to exit, make sure the `onRequestExit` callback is hooked up and call the `requestExit()` method. When the library and CEF have cleaned everything up, your callback will be triggered and you can call `shutdown()` before exiting normally.

## Grabbing CEF

* Make sure [CMake](https://cmake.org/) version 3.6.3 or later is installed on your system
* Grab a Windows 32 or 64 bit build of CEF from [http://cefbuilds.com](http://cefbuilds.com)
* Extract the 7Zip archive to a folder in an easy to find place - I use the desktop
* Edit the `tools/make_dullahan_cef_pkg.bat` batch file - specifically, the lines at the top of the file that set the bit width, the CEF folder you just unpacked and the destination folder of the Dullahan compatible package.
* Run the batch file
* If all goes well, you will end up with a CEF package in the `DST_DIR` folder
* Build Dullahan as per instructions above

If this step failed then you will have to inspect the batch file. Generally speaking, this is what it does:

* Run CMake to generate the solution and projects files for LibCEF DLL wrapper
* Change the `libcef_dll_wrapper -> Properties C++ -> Code Generation` settings to `Multi-threaded DLL` (Release configuration) and `Multi-threaded Debug DLL` (Debug configuration) 
* Build everything
* Copy files to the right place

Another option is to build the whole thing from scratch. This is necessary if you want to make changes to the Chromium code or perhaps turn on the proprietary media codec support which is not available in the [CEF Builds](http://cefbuilds.com) versions. For more information on how to do this, look online or inspect the `tools/build_cef.bat` batch file.

## What is left to do before the first real release?

*   Add support for 64bit macOS
*   Implement proper Dullahan <--> JavaScript interoperability
*   Improve the documentation
*   Try to find a way to grab CEF builds automagically vs that crazy build steps in the docs
*   Make sure all the features are used in the examples to help understand usage
*   Unit tests everywhere
*   Add support for different pages on each side of the Web Cube example
*   Lots and lots more... please help if you can.

## Why the name?

[Seemed appropriate](https://www.google.com/search?q=dullahan)