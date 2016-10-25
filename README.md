# Dullahan

## What is it?

A headless browser SDK that uses the [Chromium Embedded Framework](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework) (CEF). It is designed to make it easier to write applications that render modern web content directly to a memory buffer, inject synthesized mouse and keyboard events and interact with web based features like JavaScript or cookies.

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
  * Capture a Google Maps traffic map of your commute for 24 hours and see how bad it is :)
* Web page capture to an image or a PDF file
* Lots more I haven't thought of yet

There is a short description and a screenshot for each example [here](examples.README.md). 

## What is CEF?

From the [CEF Wiki](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework) page: The Chromium Embedded Framework (CEF) is an open source framework for embedding a web browser engine based on the Chromium core. 

An essential site to visit when developing a CEF based application is the [CEF forum](http://magpcss.org/ceforum/) started and run by (I believe) the original author Marshall Greenblatt who is very generous with his time and expertise.

Building CEF locally is quite a task so Adobe kindly host builds for all supported platforms [here](http://cefbuilds.com).

## Which version of CEF does Dullahan use?

Dullahan been developed against the [CEF 3.2704](https://cefbuilds.com) build of CEF but with some small tweaks to take account of CEF interface changes etc. it should work with slightly older or more recent versions too.

## What are the origins of this project?

Historically, the first incarnation was [LLCEFLib](https://bitbucket.org/lindenlab/3p-llceflib) - a [Linden Lab](http://lindenlab.com) internal library to render web content content inside [Second Life](http://secondlife.com). LLCEFLib had to support the outdated 32bit macOS version as well as many Second Life specific features so things became very disjointed and messy. This effort is an attempt to unburden the API from those requirements and provide a Windows only version that is much more generic and hopefully useful in other applications.

## Is Windows, 32bit the only support platform?

For the moment yes. The 32bit version for macOS is no longer supported by CEF. One day, there will be a 64bit Windows version as well and ideally, at that point, the 64bit macOs version will be added too. No plans for a Linux version but if you'd like to help, please let me know.

## Which version of the C++ runtime library does it use?

For historical reasons, both it and the version of CEF in this repository are built with the C++ runtime library set to `Multi-threaded DLL` /  `Multi-threaded Debug DLL`. 

## How do I build Dullahan?

* Download or clone the source code here
* Build or grab the version of CEF you want to use (See below)
* Ensure Microsoft Visual Studio 2013 (update 4) is installed (it might work with other versions but that is untested)
* Open `dullahan.sln`. 
* Select the Release or Debug configuration and build the solution in the normal fashion. 
* If all goes well, you can set one of the example applications as the startup project and try it.

## Binary distribution of example applications

If you would like to try the Dullahan example applications without having to build them, thee is a zipped binary distribution in the `bin` folder. It contains all the CEF/Chromium runtime files as well as the example executables. 

## Which files do I need in my own project?

The main benefit of using Dullahan vs raw CEF is that you simply include the `dullahan.h` header file in your application and link against the `dullahan.lib` library along with two small CEF ones. Then, copy over the various CEF/Chromium runtime files along side your application - see the post-build step for the examples in Visual Studio for a list - and you are ready to go.

## How do I use it?

The short answer is look at the code in the `examples` folder and `dullahan.h`. Generally speaking, you create an instance of the library, call `init()` and either (a) regularly call the `update()` function in your own message loop or the `run()` function if you don't have a message loop and want to use CEFs (e.g. a console application to save a web page as a bitmap). You can hook up a callback to be notified when the contents of the page changes and get access to the raw pixels. You can synthesize mouse and keyboard events and send them into the page. Finally, when you want to exit, make sure the `onRequestExit` callback is hooked up and call the `requestExit()` method. When the library and CEF have cleaned everything up, your callback will be triggered and you can call `shutdown()` before exiting normally.

## Grabbing CEF

*   Make sure [CMake](https://cmake.org/) is installed on your system
*   Grab a Windows 32-bit build of CEF from [http://cefbuilds.com](http://cefbuilds.com)
*   Extract the 7Zip archive to a folder in an easy to find place - I use the desktop
*   Edit the `tools/make_dullahan_cef_pkg.bat` batch file - specifically, the lines at the top of the file that set the CEF folder you just unpacked and the destination folder of the Dullahan compatible package.
* Run the batch file
* If all goes well, you will end up with a Dullahan CEF package in the `DST_DIR` folder you set
* Move that to the `cef` folder in this project, edit the `src/dullahan.props` file and update the `CEF_DIR` variable to point to the folder
* Build Dullahan

If this step failed then you will have to inspect the batch file. Generally speaking, this is what it does:

* Run CMake to generate the solution and projects files for LibCEF DLL wrapper
* Change the `libcef_dll_wrapper -> Properties C++ -> Code Generation` settings to `Multi-threaded DLL` (Release configuration) and `Multi-threaded Debug DLL` (Debug configuration) 
* Build everything
* Copy files to the right place

Another option is to build the whole thing from scratch. This is necessary if you want to make changes to the Chromium code or perhaps turn on the proprietary media codecs support which is not available in the [CEF Builds](http://cefbuilds.com) versions. For more information on how to do this, look online or inspect the `tools/build_cef.bat` batch file.

## What is left to do before the first real release?

*   Consider using CMake to generate the Dullahan project files
*   Add support for 64bit windows, macOS and Linux versions
*   Implement proper Dullahan <--> JavaScript interoperability
*   Improve the documentation
*   Try to find a way to grab CEF builds automagically vs that crazy build steps in the docs
*   Make sure all the features are used in the examples to help understand usage
*   Unit tests everywhere
*   Add support for different pages on each side of the Web Cube example
*   Lots and lots more... please help if you can.

## What is `dullahan_src_hash.hpp`

In an effort to come up with a way to uniquely identify a version, I experimented with taking an MD5 hash of all the source code and using part of that in the version number string. A Pre-Build in the Visual Studio Project file step does this and writes out `dullahan_src_hash.hpp`. The hash in this header file is then combined with the CEF version, the Chrome version it represents as well as a Dullahan version into a single string that uniquely identifies the build. I'm still not sure this is a good idea but I'll leave in there until someone explains to me how stupid it is.

## Why the name?

[Seemed appropriate](https://www.google.com/search?q=dullahan)