rem Builds CEF from source with VS 2015 and optionally enables the proprietary
rem codec support that enables playback of media such as MPEG4 inline
rem It takes a long time to build - 5+ hours on my aging MacBook Pro/BootCamp.

rem rudimentary timing
time /t > build_duration

rem user variables for the build - change these to configure builds
set BIT_WIDTH=64
set BRANCH=3202
set PROPRIETARY_CODEC=1
set BUILD_DEBUG=1

rem make sure this path isn't too long, windows can fail with "path too long" errors!
set DOWNLOAD_DIR=%1
if "%1"=="" set DOWNLOAD_DIR="%USERPROFILE%\Desktop\cef.%BRANCH%.win%BIT_WIDTH%^%/

rem Grab the latest version the Python build script using the Powershell equivalent of curl!
powershell -Command "(New-Object System.Net.WebClient).DownloadString('https://bitbucket.org/chromiumembedded/cef/raw/master/tools/automate/automate-git.py')" > automate-git.py

rem CEF build scripts require these
set CEF_USE_GN=1
set GN_ARGUMENTS=--ide=vs2015 --sln=cef --filters=//cef/*

rem clumsily set up command line variables
if "%PROPRIETARY_CODEC%"=="0" goto :NO_PROPRIETARY_CODEC
set GN_DEFINES=is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome
:NO_PROPRIETARY_CODEC

set BUILD_DEBUG_FLAGS=--no-debug-build
if "%BUILD_DEBUG%"=="0" goto :NO_DEBUG_BUILD
set BUILD_DEBUG_FLAGS=
:NO_DEBUG_BUILD

set BUILD_64BIT_FLAGS=
if "%BIT_WIDTH%"=="32" goto :32BIT_BUILD
set BUILD_64BIT_FLAGS=--x64-build
:32BIT_BUILD

rem build it!
python automate-git.py^
 --download-dir=%DOWNLOAD_DIR%^
 --branch=%BRANCH%^
 %BUILD_64BIT_FLAGS%^
 %BUILD_DEBUG_FLAGS%

rem rudimentary timing
time /t >> build_duration
