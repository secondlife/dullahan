@rem This batch file helps you build Dullahan given an uncompressed CEF package.
@rem
@rem Usage: tools\build_dullahan.bat <cef_build_dir> <bit width> <build_dir>
@rem
@rem Example: tools\build_dullahan.bat ".\cef\76.1.13+gf19c584" 64 build64
@rem          tools\build_dullahan.bat "c:\cef_builds\sandbox\cef77.4" 32 build_32
@rem
@rem Note: There is a .\cef_builds directory present in this repo - that's 
@rem       a good location to put working CEF builds and is ignored by Git.
 
rem The CMake generator string to use
set VS_CMD=Visual Studio 15 2017

pushd .

rem Rudimentary test to see if we are in the Dullahan root directory where
rem we demand that this script is started from.
if not exist "tools" (
    echo.
    echo *** ERROR: Run this script from Dullahan root directory ***
    goto End
)

rem Make use of a batch feature that expands %~f1 to the full path so that
rem relative or absolute values work as a parameter for the CEF Build directory
set CEF_DIR=%~f1

rem Record the bit width we have been asked to build [32 or 64]
set BIT_WIDTH=%2

rem Force user to specify the location of the build folder - we need this to
rem be specified so that the autobuild scripts can specify it too
set BUILD_DIR=%3

rem Test if a CEF directory has been specified and bail if not 
if [%CEF_DIR%]==[] (
    echo.
    echo *** ERROR: You must specify a compatible CEF build directory ***
    goto End
)

rem Test if the CEF directory exists and bail if not 
if not exist %CEF_DIR% (
    echo.
    echo *** ERROR: CEF directory '%CEF_DIR%' does not exist ***
    goto End
)

rem Test if 32 or 64 is specified as a bit width and bail if not 
if not "%BIT_WIDTH%"=="32" (
    if not "%BIT_WIDTH%"=="64" (
        echo.
        echo *** ERROR: You must specify a bit width of 32 or 64 ***
        goto End
    )
)

rem Test if the build dir has been specified and bail if not 
if [%BUILD_DIR%]==[] (
    echo.
    echo *** ERROR: You must specify a directory to build in ***
    goto End
)

rem Test if the build directory exists and bail if it does 
if exist %BUILD_DIR% (
    echo.
    echo *** ERROR: Build directory '%BUILD_DIR%' exists - use something else ***
    goto End
)

rem Set up build parameters for 32 bit builds
if "%BIT_WIDTH%"=="32" (
    set CMAKE_CMD="%VS_CMD%"
    set PLATFORM_CMD="/property:Platform=win32"
)

rem Set up build parameters for 64 bit builds
if "%BIT_WIDTH%"=="64" (
    set CMAKE_CMD="%VS_CMD% Win64"
    set PLATFORM_CMD="/property:Platform=x64"
)

rem Helpful, reassuring description of what we are doing....
echo.
echo Building Dullahan in .\%BUILD_DIR% for %BIT_WIDTH% bit using CEF from %CEF_DIR%
echo.

rem The header file dullahan_version.h is auto-generated via
rem dullahan_version.h.in if the former does not exist. We
rem usually want this so delete the transient version to start
if exist .\src\dullahan_version.h del .\src\dullahan_version.h

rem Start fresh with a clean build directory
if exist .\%BUILD_DIR%\* rmdir /s /q .\%BUILD_DIR%
mkdir %BUILD_DIR%
cd /d %BUILD_DIR%

rem build release and debug version of the libcef DLL wrapper
pushd .
cd %CEF_DIR%
if exist .\%BUILD_DIR%\* rmdir /s /q .\%BUILD_DIR%
mkdir %BUILD_DIR%
cd /d %BUILD_DIR%
cmake -G %CMAKE_CMD% ^
       -DCEF_RUNTIME_LIBRARY_FLAG=/MD ^
       ..
if errorlevel 1 goto End

msbuild cef.sln /target:libcef_dll_wrapper /property:Configuration="Debug" %PLATFORM_CMD%
if errorlevel 1 goto End

msbuild cef.sln /target:libcef_dll_wrapper /property:Configuration="Release" %PLATFORM_CMD%
if errorlevel 1 goto End
popd

rem Generate Dullahan project files, passing in paths to
rem various CEF folders that are consumed by CMake
cmake -G %CMAKE_CMD% ^
       -DBUILD_DIR="%BUILD_DIR%" ^
       -DCEF_INCLUDE_DIR="%CEF_DIR%\include" ^
       -DCEF_LIB_DIR="%CEF_DIR%" ^
       -DCEF_BIN_DIR="%CEF_DIR%" ^
       -DCEF_RESOURCE_DIR="%CEF_DIR%\Resources" ^
       ..
if errorlevel 1 goto End

rem Build the Debug configuration of Dullahan
msbuild dullahan.sln /property:Configuration="Debug" %PLATFORM_CMD%
if errorlevel 1 goto End

rem Build the Release configuration of Dullahan
msbuild dullahan.sln /property:Configuration="Release" %PLATFORM_CMD%
if errorlevel 1 goto End

:End

popd
