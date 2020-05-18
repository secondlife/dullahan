# Tools that are useful for working with CEF and Dullahan

    build.bat
    build.sh

A macOS shell script and Windows batch file that builds the CEF DLL wrapper followed by Dullahan. To use it, you need to provide an unwrapped CEF package you either built yourself or one you downloaded from the [Spotify CEF Automated Builds](http://opensource.spotify.com/cefbuilds/index.html) automated builds site. See the notes at the top of each file for usage, settings etc.

    astyle.sh
    astyle.bat

I find it useful to use [Artistic Style](http://astyle.sourceforge.net/) to reformat the code occasionally and maintain control of tabs/space and line endings. This shell script and batch file assume `astyle` is installed on your system and runs it with selected command line options.

    autobuild_version.cpp

This repository now includes the Linden Lab [autobuild](http://wiki.secondlife.com/wiki/Autobuild) versions of the build files and this is used to generate a version number for the package.