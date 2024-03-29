@rem This batch file builds the Dullahan SDK and example apps given a directory
@rem containing an unwrapped CEF package, a build directory to use and a bit 
@rem width [32/64]. You can either build the CEF package from source yourself 
@rem using instructions found here: https://bitbucket.org/lindenlab/cef - or 
@rem download a pre-built package via the Spotify open source Automated Builds 
@rem site here: http://opensource.spotify.com/cefbuilds/index.html
@rem 
@rem Usage: tools\build.bat <CEF dir> <build dir> <bit width>
@rem
@rem Example: tools\build.bat ".\cef\76.1.13+gf19c584" .\build 64
@rem          tools\build.bat "c:\cef_builds\sandbox\cef77.4" .\buildv77 32
@rem
@rem Note: The Windows batch file version differs from the macOS Shell script
@rem one in that you have to download the CEF package youself vs point to a
@rem URL - the commands to download a file are cumbersome on Windows and the 
@rem tar command to extract the files just doesn't work on the large CEF 
@rem packages. For now, it's easier to do that youself and point to the result. 

@rem Save current folder so we can return to it afterwards
@pushd .

@rem The CMake generator string to use
@set VS_CMD=Visual Studio 17 2022

@rem Rudimentary test to see if we are in the Dullahan root directory where
@rem we demand that this script is started from.
@if not exist "tools" (
    @echo.
    @echo *** ERROR: Run this script from Dullahan root directory ***
    @goto End
)

@rem Make use of a batch feature that expands %~f1 to the full path so that
@rem relative or absolute values work as a parameter for the CEF Build directory
@set SRC_CEF_DIR=%~f1

@rem Test if a CEF directory has been specified and bail if not 
@if [%SRC_CEF_DIR%]==[] (
    @echo.
    @echo *** ERROR: You must specify a compatible CEF build directory ***
    @goto End
)

@rem Test if the CEF directory exists and bail if not 
@if not exist %SRC_CEF_DIR% (
    @echo.
    @echo *** ERROR: CEF directory '%SRC_CEF_DIR%' does not exist ***
    @goto End
)

@rem Record the specified build directory - this is useful as it lets us 
@rem build multiple versions in different build directories 
@set BUILD_DIR=%~f2

@rem Test if a build directory has been specified and bail if not 
@if [%BUILD_DIR%]==[] (
    @echo.
    @echo *** ERROR: You must specify a build directory ***
    @goto End
)

@rem Test if the build directory exists and bail if it does - it is too
@rem difficult to deal with caching etc. so we always build a clean copy
@if exist %BUILD_DIR% (
    @echo.
    @echo *** ERROR: Build directory '%BUILD_DIR%' exists - delete first ***
    @goto End
)

@rem Record the bit width we have been asked to build [32 or 64]
@set BIT_WIDTH=%3

@rem Test if 32 or 64 is specified as a bit width and bail if not 
@if not "%BIT_WIDTH%"=="32" (
    if not "%BIT_WIDTH%"=="64" (
        @echo.
        @echo *** ERROR: You must specify a bit width of 32 or 64 ***
        @goto End
    )
)

@rem Set up build parameters for 32 bit builds
@if "%BIT_WIDTH%"=="32" (
    @set CMAKE_GENERATOR="%VS_CMD%"
    @set CMAKE_ARCH=-A Win32
    @set PLATFORM_CMD="/property:PlatformTarget=x86"
)

@rem Set up build parameters for 64 bit builds
@if "%BIT_WIDTH%"=="64" (
    @set CMAKE_GENERATOR="%VS_CMD%"
    @set CMAKE_ARCH=-A x64
    @set PLATFORM_CMD="/property:PlatformTarget=x64"
)

@rem Create the other directories we will use to build the CEF wrapper and
@rem then use it to build Dullahan and the examples. Note that mkdir here
@rem is able to build directories that do not exist as required.
@set DST_CEF_DIR=%BUILD_DIR%\cef
@mkdir %DST_CEF_DIR%
@set CEF_BUILD_DIR=%DST_CEF_DIR%\build
@mkdir %CEF_BUILD_DIR%
@set DULLAHAN_DIR=%cd%

@rem Display the parameters we specified and calculated
@echo Build parameters:
@echo   Dullahan project dir: %DULLAHAN_DIR%
@echo   CEF source dir: %SRC_CEF_DIR%
@echo   Bit width: %BIT_WIDTH%
@echo   Build dir: %BUILD_DIR%
@echo   Destination CEF dir: %DST_CEF_DIR%
@echo   CEF build dir: %CEF_BUILD_DIR%

@rem Copy over the CEF package files from the source directory provided
@rem so that we do not build in the original source location. By the way,
@rem who knew that robocopy existed in Windows! Specify all the /Nxx options
@rem to supress the spammy output of files and directories
@robocopy %SRC_CEF_DIR% %DST_CEF_DIR% /E /NFL /NDL /NJH /NJS

@rem Build the CEF wrapper for Debug and Release configurations. Note that while
@rem we don't often need the Debug build, the Dullahan CMakeLists.txt file refers
@rem to it and will fail without it so we must build
@cd %CEF_BUILD_DIR%
@cmake -G %CMAKE_GENERATOR% %CMAKE_ARCH% .. -DCEF_RUNTIME_LIBRARY_FLAG=/MD -DUSE_SANDBOX=Off
@if errorlevel 1 goto End
@cd libcef_dll_wrapper
@msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Debug" %PLATFORM_CMD%
@if errorlevel 1 goto End
@msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Release" %PLATFORM_CMD%
@if errorlevel 1 goto End

@rem We usually don't worry about the header file since it's autogenerated from
@rem src\dullahan.h.in but building Dullahan with different versions of CEF can
@rem lead to some annoying #define discrepancies so we delete it here and let 
@rem the code in the CMakeLists.txt recreeate it based on the contents of CEF 
@cd %BUILD_DIR%
if exist ..\src\dullahan_version.h del ..\src\dullahan_version.h 

@rem Build the Dullahan solution which includes the SDK as well as the examples 
@rem Note: we remove the Dullahan version header file since it is created each
@rem time to include the latest information by the CMake script 
@cmake -G %CMAKE_GENERATOR% %CMAKE_ARCH% ^
       -DCEF_WRAPPER_DIR=%DST_CEF_DIR% ^
       -DCEF_WRAPPER_BUILD_DIR=%CEF_BUILD_DIR% ^
       ..
@if errorlevel 1 goto End
@msbuild dullahan.sln /property:Configuration="Debug" %PLATFORM_CMD%
@if errorlevel 1 goto End
@msbuild dullahan.sln /property:Configuration="Release" %PLATFORM_CMD%
@if errorlevel 1 goto End

@echo Build succeeded: Visual Studio solution file is %BUILD_DIR%\dullahan.sln

:End

@rem Return to the folder we saved
@popd
