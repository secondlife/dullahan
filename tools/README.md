# Tools that are useful for working with CEF and Dullahan

`build_win.bat`

Simple batch file to build Dullahan as well as an example application. Pass in 32 or 64 for 32/64bit versions.

`build_mac.sh`

Simple shell script to build 64 bit macOS version of Dullahan.

`cef_download_build.bat`

Batch file that grabs the latest version of automate-git.py and uses it to build CEF from source. You might want to do this if you want to enable the embedded support of media including MPEG-4 files. Since the Chromium dependency on Visual Studio 2015 was introduced, this batch file no longer works but is left here in case parts of it are useful.

`dullahan_test_urls.html`

Simple HTML page that contains links to various sites that test Dullahan functionality.

`make_dullahan_cef_pkg.bat`

A simple Windows batch file that uses a CEF build from the [Spotify](http://opensource.spotify.com/cefbuilds/index.html) CEF site and extracts files (dlls, lib, Resource files etc.) that Dullahan needs.

`make_dullahan_cef_pkg.sh`

A simple macOS shell script that uses a CEF build from the [Spotify](http://opensource.spotify.com/cefbuilds/index.html) CEF site and extracts files (headers, libs, frameworks etc.) that Dullahan needs.

`make_src_hash.bat`

Batch file used to generate a hash of the cpp/h source files that is used as part of the Dullahan version number. A testbed before actually using in the client. A post-build step in the Windows version used to use this but no longer does.

`astyle.bat`

I find it useful to use [Artistic Style](http://astyle.sourceforge.net/) to reformat the code occasionally and maintain control of tabs/space and line endings. This batch file assumes astyle is installed on your system and runs it with selected command line options.