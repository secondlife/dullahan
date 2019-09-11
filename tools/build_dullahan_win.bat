@rem This batch file helps you build Dullahan given a compatible build of CEF.
@rem See the README.md file in this directory for how to download or build 
@rem a compatible version.
@rem
@rem Usage: tools\build_dullahan_win.bat <cef_build_dir> <bit width>
@rem
@rem Example: tools\build_dullahan_win.bat ".\cef\76.1.13+gf19c584" 64
@rem          tools\build_dullahan_win.bat "c:\cef_builds\sandbox\cef77.4" 32
@rem
@rem Note: There is a .\cef directory present in this repo - that's a good
@rem       location to put CEF builds since it's ignored by Mercurial.
 
@rem The CMake generator string to use
@set VS_CMD=Visual Studio 15 2017

@pushd .

@rem Rudimentary test to see if we are in the Dullahan root directory where
@rem we demand that this script is started from.
@if not exist "tools" (
    @echo.
    @echo *** ERROR: Run this script from Dullahan root directory ***
    @goto End
)

@rem Make use of a batch feature that expands %~f1 to the full path so that
@rem relative or absolute values work as a parameter for the CEF Build directory
@set CEF_DIR=%~f1

@rem Record the bit width we have been asked to build [32 or 64]
@set BIT_WIDTH=%2

@rem Test if a CEF directory has been specified and bail if not 
@if [%CEF_DIR%]==[] (
    @echo.
    @echo *** ERROR: You must specify a compatible CEF build directory ***
    @goto End
)

@rem Test if the CEF directory exists and bail if not 
@if not exist %CEF_DIR% (
    @echo.
    @echo *** ERROR: CEF directory '%CEF_DIR%' does not exist ***
    @goto End
)

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
    @set BUILD_DIR=build
    @set CMAKE_CMD="%VS_CMD%"
    @set PLATFORM_CMD="/property:PlatformTarget=x86"
)

@rem Set up build parameters for 64 bit builds
@if "%BIT_WIDTH%"=="64" (
    @set BUILD_DIR=build64
    @set CMAKE_CMD="%VS_CMD% Win64"
    @set PLATFORM_CMD="/property:PlatformTarget=x64"
)

@rem Helpful, reassuring description of what we are doing....
@echo.
@echo Building Dullahan in .\%BUILD_DIR% for %BIT_WIDTH% bit using CEF from %CEF_DIR%
@echo.

@rem The header file dullahan_version.h is auto-generated via
@rem dullahan_version.h.in if the former does not exist. We
@rem usually want this so delete the transient version to start
@if exist .\src\dullahan_version.h del .\src\dullahan_version.h

@rem Start fresh with a clean build directory
@if exist .\%BUILD_DIR%\* rmdir /s /q .\%BUILD_DIR%
@mkdir %BUILD_DIR%
@cd /d %BUILD_DIR%

@rem Generate Dullahan project files, passing in paths to
@rem various CEF folders that are consumed by CMake
@cmake -G %CMAKE_CMD% ^
       -DCEF_INCLUDE_DIR="%CEF_DIR%\include\cef\include" ^
       -DCEF_LIB_DIR="%CEF_DIR%\lib" ^
       -DCEF_BIN_DIR="%CEF_DIR%\bin" ^
       -DCEF_RESOURCE_DIR="%CEF_DIR%\resources" ^
       ..
@if errorlevel 1 goto End

@rem Build the Debug configuration of Dullahan
@pause here 1
@msbuild dullahan.sln /property:Configuration="Debug" %PLATFORM_CMD%
@if errorlevel 1 goto End

@rem Build the Release configuration of Dullahan
@msbuild dullahan.sln /property:Configuration="Release" %PLATFORM_CMD%
@if errorlevel 1 goto End

@rem Looks like everything worked - open our Visual Studio solution
@start dullahan.sln

:End

@popd
