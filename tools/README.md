# Tools that are useful for working with CEF and Dullahan

#### build_cef.bat

Batch file that grabs the latest version of automate-git.py and uses it to build CEF from source. You might want to do this if you want to enable the embedded support of media including MPEG-4 files.

#### dullahan_test_urls.html

Simple HTML page that contains links to various pages that test Dullahan functionality.

#### make_dullahan_cef_pkg.bat

Batch file that uses a CEF build made using build_cef.bat and extracts files (DLLs, LIBS, Resource files etc.) that Dullahan needs.

#### make_src_hash.bat

Batch file used to generate a hash of the cpp/h source files that is used as part of the Dullahan version number. A testbed before transplanting code into MSVC Pre-Build event.

#### astyle.bat

I find it useful to use [Artistic Style](http://astyle.sourceforge.net/) to reformat the code occasionally and maintain control of tabs/space and line endings. This batch file assumes astyle is installed on your system and runs it with selected command line options.

