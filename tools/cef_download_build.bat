rem Builds CEF from source with VS 2013 and optionally enables the proprietary
rem codec support that enables playback of media such as MPEG4 inline
rem It takes a long time to build - 5+ hours on my aging MacBook Pro/BootCamp.

rem rudimentary timing
time /t > build_duration

rem user variables for the build - change these to configure builds
set BIT_WIDTH=32
set BRANCH=2704
set PROPRIETARY_CODEC=1
set BUILD_DEBUG=1
set BASE_OUTPUT_FOLDER="%USERPROFILE%\Desktop"

rem Grab the latest version the Python build script using the Powershell equivalent of curl!
powershell -Command "(New-Object System.Net.WebClient).DownloadString('https://bitbucket.org/chromiumembedded/cef/raw/master/tools/automate/automate-git.py')" > automate-git.py

rem CEF build scripts require these
set GYP_MSVS_VERSION=2013

rem clumsily set up command line variables
set PROPRIETARY_CODEC_FILENAME_TAG=
if "%PROPRIETARY_CODEC%"=="0" goto :NO_PROPRIETARY_CODEC
set GYP_DEFINES=proprietary_codecs=1 ffmpeg_branding=Chrome buildtype=Official
set PROPRIETARY_CODEC_FILENAME_TAG=.media
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
 --download-dir=%BASE_OUTPUT_FOLDER%\cef-%BRANCH%.win.%BIT_WIDTH%%PROPRIETARY_CODEC_FILENAME_TAG%^
 --branch=%BRANCH%^
 %BUILD_64BIT_FLAGS%^
 %BUILD_DEBUG_FLAGS%

rem rudimentary timing
time /t >> build_duration
