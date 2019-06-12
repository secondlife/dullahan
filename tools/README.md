# Tools that are useful for working with CEF and Dullahan

`build_dullahan.bat`

Simple batch file to build CEF then Dullahan as well as an example applications. Pass in 32 or 64 for 32/64bit versions.

`build_mac.sh`

Simple shell script to build 64 bit macOS version of Dullahan.

`make_dullahan_cef_pkg.sh`

A simple macOS shell script that uses a CEF build from the [Spotify](http://opensource.spotify.com/cefbuilds/index.html) CEF site and extracts files (headers, libs, frameworks etc.) that Dullahan needs.

`astyle.bat`

I find it useful to use [Artistic Style](http://astyle.sourceforge.net/) to reformat the code occasionally and maintain control of tabs/space and line endings. This batch file assumes astyle is installed on your system and runs it with selected command line options.

`generate_build_version_number.bat`

I wanted a way to version small changes over time and everything else I looked at didn't do the trick.
I am going to try using the "integer number of 10 minutes's since 2000-01-01" and see how that goes.
Each time I want to increase the build version number, I run this script and paste the output into
the DULLAHAN_VERSION_BUILD value in src/dullahan_version.h.in. We will see if this is a good idea...
