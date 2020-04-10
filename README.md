# Dullahan

## What is it?

A headless browser SDK that uses the [Chromium Embedded Framework](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework) (CEF). It is designed to make it easier to write applications that render modern web content directly to a memory buffer, inject synthesized mouse and keyboard events as well as interact with features like JavaScript or cookies.

## What could I use it for?

It might be useful in a number of situations - for example:

* Anywhere you want to render web content in a graphics application - e.g. help pages in an OpenGL based game
* VR based video chat and shared environments
* HTML/JavaScript based user interfaces E.G. a tablet based paradigm for VR
* Integration with JavaScript for automated site testing
* Web page contents analysis - E.G:
    * Capture images the most popular 500 web pages and count the top 5 colors in each.
    * Capture a Google Maps traffic map of where you live for 24 hours and see how *really* miserable your commute is :)
* Web page capture to an image or a PDF file
* Lots more I haven't thought of yet

There are some examples included in this repository and there is a description and a screen shot for each [here](./examples/README.md). 

## What is CEF?

From the [CEF Wiki](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework) page: The Chromium Embedded Framework (CEF) is an open source framework for embedding a web browser engine based on the Chromium core. 

An essential site to visit when developing a CEF based application is the [CEF forum](http://magpcss.org/ceforum/) started and run by (I believe) the original CEF author Marshall Greenblatt who is very generous with his time and expertise.

## Which platforms are supported?

* Windows 32/64 bit (primary development platform)
* macOS 64 bit (works, is supported but needs a little work - any macOS devs out there want to help?)
* Linux 64 bit (not available yet but hopefully one day - any Linux devs want to help?)

## How do I build CEF?

In order to build Dullahan, you need to build CEF. You can acquire or build a compatible version of CEF in a number of ways:

* Use the CEF package from the Spotify Automated CEF Builds page
    * Look at `build_dullahan_mac.sh` and `build_dullahan_mac.bat`in the `tools` directory
* Use a Linden Lab autobuild package for CEF
    * Look at the `extract_3p_cef_wrapper_mac.sh` and `extract_3p_cef_wrapper_mac.bat` in the `tools` directory
* Build CEF from source entirely. This also means building Chromium from source too which takes a long time and needs to be done on a higher end spec computer
    * Look at the `build_cef_src_mac.sh` and `build_cef_src_win.bat` in the `tools` directory
    * You will have to do this if you want to enable the `media_codec` option in builds that enables support for sites like Twitch, YouTube Live and Zoom

* Lots more information about building CEF on all platforms can be found on the `cef / MasterBuildQuickStart` page [here](https://bitbucket.org/chromiumembedded/cef/wiki/MasterBuildQuickStart#markdown-header-mac-os-x-setup)

## How do I build Dullahan?

Once you have a build of CEF in hand using one of the steps above, you can proceed with a build of Dullahan:

* Use `build_dullahan_mac.sh` and `build_dullahan_mac.bat` to build Dullahan and the examples
* Pass in the location of the CEF build you made earlier as the first parameter
* For example: `tools/build_dullahan_mac.sh ./cef_builds/cef80.1.0.6`

## What are the system requirements for building CEF and Dullahan?

* The requirements for building Chromium and CEF change frequently
* The requirements for building Dullahan mirror those of CEF and Chromium
* The most up to date version will always be on the `cef / BranchesAndBuilding` page [here](https://bitbucket.org/chromiumembedded/cef/wiki/BranchesAndBuilding.md#markdown-header-background).

## What are those odd files in the root directory

You might notice some rather unusual files in the root directory of the Dullahan repository - for example: `build-cmd.sh`, `BuildParams`, `version.cpp` They are used to build the Linden Lab [autobuild](http://wiki.secondlife.com/wiki/Autobuild) version of the Dullahan package that is then used in our product which is called [Second Life](https://secondlife.com). The [autobuild](http://wiki.secondlife.com/wiki/Autobuild) system is what we use to build all of our internal software and requires that those files be there in the root directory. You can safely ingore them if you are using Dullahan for something outside Second Life.

## Which version of the C++ runtime library does it use on Windows?

For historical reasons, both Dullahan and the version of CEF it consumes are built with the C++ runtime library set to `Multi-threaded DLL` / `Multi-threaded Debug DLL`

## Is there a binary version I can download? ##

Not right now. I used to store a pre-built version on the Windows 64 bit version in my DropBox but that became untenable. Once I figure out a sensible place to store a large binary package, I'll bundle up the latest SDK and examples for all the platforms.

## How do I use it?

The short answer is look at the code in the `examples` folder and `dullahan.h`. Generally speaking, you create an instance of the library, call `init()` and regularly call the `update()` function in your own message loop. You can hook up a callback to be notified when the contents of the page changes and get access to the raw pixels. You can synthesize mouse and keyboard events and send them into the page. Finally, when you want to exit, make sure the `onRequestExit` callback is hooked up and call the `requestExit()` method. When the library and CEF have cleaned everything up, your callback will be triggered and you can call `shutdown()` before exiting normally.

## Are there examples?

Why yes there is - take a look in the `examples` subdirectory of this repository. There are some screen shots and short descriptions in the [README](https://bitbucket.org/lindenlab/dullahan/src/default/examples/README.md) file too 

## Alternatives

* The most exciting alternative in this space is from Google themselves - [headless Chromium](https://chromium.googlesource.com/chromium/src/+/lkgr/headless/README.md) and [Puppeteer](https://developers.google.com/web/tools/puppeteer) both appear to offer much of what Dullahan does albeit better because they have more than one person working on it :) It's very new and still developing but definitely worth a look and will likely supersede Dullahan one day. 

## Why the name?

[Seemed appropriate](https://en.wikipedia.org/wiki/Dullahan)
