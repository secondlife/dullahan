# Tools that are useful for working with CEF and Dullahan

    build_cef_wrapper_mac.sh
    build_cef_wrapper_win.bat

A macOS shell script and Windows batch file that builds the CEF DLL wrapper using a third-party version of CEF from the [Spotify CEF Automated Builds](http://opensource.spotify.com/cefbuilds/index.html) site for example. See the notes at the top of each file for usage, settings etc.

    extract_3p_cef_wrapper_mac.sh
    extract_3p_cef_wrapper_win.bat

A macOS shell script and Windows batch file that extracts a prebuilt CEF DLL wrapper from a Linden Lab autobuild 3P CEF package. Unless you are working on the Second Life Viewer, it is very unlikely you need to care about these files. See the notes at the top of each file for usage, settings etc.

    build_dullahan_mac.sh
    build_dullahan_win.bat

A macOS shell script and Windows batch file that builds the Dullahan library as well as example applications that use it. Pass in a directory containing the CEF wrapper you either built or extracted using the scripts and batch files listed above.

    astyle.sh
    astyle.bat

I find it useful to use [Artistic Style](http://astyle.sourceforge.net/) to reformat the code occasionally and maintain control of tabs/space and line endings. This shell script and batch file assume `astyle` is installed on your system and runs it with selected command line options.

    build_cef_src_win.bat

A Windows batch file that builds Chromium from source then CEF from source (excluding the CEF DLL wrapper - use the other scripts for that). Note: This takes a long time and consumes a lot of resources. On my modest development machine for example it takes 120GB of disk space and at least 15+ hours to run. The main reason you might want to build from source like this is to enable the CEF media codec support which is disabled in the Spotify CEF builds. 