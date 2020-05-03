# Tools that are useful for working with CEF and Dullahan

    build.bat
    build.sh

A macOS shell script and Windows batch file that builds the CEF DLL wrapper followed by Dullahan. To use it, you need to provide an unwrapped CEF package you either built yourself or one you downloaded from the [Spotify CEF Automated Builds](http://opensource.spotify.com/cefbuilds/index.html) automated builds site. See the notes at the top of each file for usage, settings etc.

    astyle.sh
    astyle.bat

I find it useful to use [Artistic Style](http://astyle.sourceforge.net/) to reformat the code occasionally and maintain control of tabs/space and line endings. This shell script and batch file assume `astyle` is installed on your system and runs it with selected command line options.

    autobuild_version.cpp

This repository now includes the Linden Lab [autobuild](http://wiki.secondlife.com/wiki/Autobuild) versions of the build files and this is used to generate a version number for the package.

    build_cef_src_win.bat

A Windows batch file that builds Chromium from source then CEF from source. Note: This takes a very long time and consumes a lot of resources. On my modest development machine for example it takes 120GB of disk space and at least 15+ hours to run. The main reason you might want to build from source like this is to enable the CEF media codec support which is disabled in the Spotify CEF builds.