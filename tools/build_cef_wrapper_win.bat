@rem This batch file helps you build a CEF wrapper that is compatible 
@rem with Dullahan from a CEF source package that you:
@rem
@rem  1/ Downloaded from the Spotify open source site here:
@rem     http://opensource.spotify.com/cefbuilds/index.html
@rem  2/ Built yourself from the Chromium/CEF sources - look in
@rem     %BUILD_DIR%\chromium_git\chromium\src\cef\binary_distrib
@rem  3/ Somewhere else entirely but is compatible with 1/ or 2/
@rem
@rem Usage: tools\build_cef_wrapper_win.bat <CEF source dir> <CEF out dir> [32|64]
@rem
@rem     <CEF source dir> is the name of the directory containing the CEF 
@rem                      package you downloaded or built 
@rem
@rem     <CEF out dir> is the name of the directory where you want to store
@rem                    the Dullahan compatible CEF wrapper package
@rem
@rem     [32|64] the script will try to guess whether to do 32 or 64 builds 
@rem             based on source directory name but this lets you override
@rem
@rem Example:
@rem     tools\build_cef_wrapper_win.bat C:\download\spotify_cef_76_1_3_windows64 .\cef_76.64
@rem     tools\build_cef_wrapper_win.bat C:\code\build\cef_latest C:\cef\builds\cef77.1 32

@rem Set the source and dest directories using the clever batch feature that
@rem converts a relative or absolute path into an absolute one
@set CEF_SRC_DIR=%~f1
@set CEF_DST_DIR=%~f2

@rem Check if the source dir exists and bail if it doesn't
@if not exist %CEF_SRC_DIR% (
    @echo.
    @echo *** ERROR: %CEF_SRC_DIR% does not exist - check your settings ****
    @goto end
)

@rem Check if the destination dir exists and bail if it does - we want to start clean
@if exist %CEF_DST_DIR% (
    @echo.
    @echo *** ERROR: %CEF_DST_DIR% exists - remove this directory before running this script ****
    @goto end
)

@rem Check if the destination dir cannot be created and bail if not 
@mkdir %CEF_DST_DIR%
@if not exist %CEF_DST_DIR% (
    @echo.
    @echo *** ERROR: %CEF_DST_DIR% cannot be created - check your settings ****
    @goto end
)

@rem The CMake generator string to use
@set VS_CMD=Visual Studio 12 2013

@rem Get the bit width paramter from command line and if it's empty, try to guess the 
@rem value based on the source directory name - Spotify CEF builds have windows32 or
@rem windows64 in the name for example
@set BIT_WIDTH=%3
@if [%BIT_WIDTH%]==[] (
    @rem Determine if we are building 32 or 64 bit based on the folder name of the source CEF bundle
    @echo %CEF_SRC_DIR%|find "windows64" >nul
    @if errorlevel 1 (
        set BIT_WIDTH=32
    ) else (
        set BIT_WIDTH=64
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
@echo Building CEF in .\%BUILD_DIR% for %BIT_WIDTH% bit using CEF from %CEF_SRC_DIR% and writing compatible CEF package to %CEF_DST_DIR%
@echo.

@mkdir "%CEF_DST_DIR%\bin\debug"
@mkdir "%CEF_DST_DIR%\bin\debug\swiftshader"
@mkdir "%CEF_DST_DIR%\bin\release"
@mkdir "%CEF_DST_DIR%\bin\release\swiftshader"
@mkdir "%CEF_DST_DIR%\include\cef\include"
@mkdir "%CEF_DST_DIR%\lib\debug"
@mkdir "%CEF_DST_DIR%\lib\release"
@mkdir "%CEF_DST_DIR%\resources"

@pushd .

@cd /d %CEF_SRC_DIR%

@if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%

@mkdir %BUILD_DIR%
@cd /d %BUILD_DIR%
@rem Note that we invoke the option to use the dynamic runtime library for the CEF wrapper 
@rem since that is what we use everywhere else - remove the -D flag to revert to /MT
@cmake -G %CMAKE_CMD% .. -DCEF_RUNTIME_LIBRARY_FLAG=/MD
@cd libcef_dll_wrapper

@msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Debug" %PLATFORM_CMD%
@msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Release" %PLATFORM_CMD%

copy "Debug\libcef_dll_wrapper.lib" "%CEF_DST_DIR%\lib\debug"
copy "Release\libcef_dll_wrapper.lib" "%CEF_DST_DIR%\lib\release"

@popd

@rem ******** bin folder ********
copy "%CEF_SRC_DIR%\Release\d3dcompiler_47.dll" "%CEF_DST_DIR%\bin\release"
copy "%CEF_SRC_DIR%\Release\chrome_elf.dll" "%CEF_DST_DIR%\bin\release"
copy "%CEF_SRC_DIR%\Release\libcef.dll" "%CEF_DST_DIR%\bin\release"
copy "%CEF_SRC_DIR%\Release\libEGL.dll" "%CEF_DST_DIR%\bin\release"
copy "%CEF_SRC_DIR%\Release\libGLESv2.dll" "%CEF_DST_DIR%\bin\release"
copy "%CEF_SRC_DIR%\Release\natives_blob.bin" "%CEF_DST_DIR%\bin\release"
copy "%CEF_SRC_DIR%\Release\snapshot_blob.bin" "%CEF_DST_DIR%\bin\release"
copy "%CEF_SRC_DIR%\Release\v8_context_snapshot.bin" "%CEF_DST_DIR%\bin\release"
copy "%CEF_SRC_DIR%\Release\swiftshader\libEGL.dll" "%CEF_DST_DIR%\bin\release\swiftshader"
copy "%CEF_SRC_DIR%\Release\swiftshader\libGLESv2.dll" "%CEF_DST_DIR%\bin\release\swiftshader"

copy "%CEF_SRC_DIR%\Debug\d3dcompiler_47.dll" "%CEF_DST_DIR%\bin\debug"
copy "%CEF_SRC_DIR%\Debug\chrome_elf.dll" "%CEF_DST_DIR%\bin\debug"
copy "%CEF_SRC_DIR%\Debug\libcef.dll" "%CEF_DST_DIR%\bin\debug"
copy "%CEF_SRC_DIR%\Debug\libEGL.dll" "%CEF_DST_DIR%\bin\debug"
copy "%CEF_SRC_DIR%\Debug\libGLESv2.dll" "%CEF_DST_DIR%\bin\debug"
copy "%CEF_SRC_DIR%\Debug\natives_blob.bin" "%CEF_DST_DIR%\bin\debug"
copy "%CEF_SRC_DIR%\Debug\snapshot_blob.bin" "%CEF_DST_DIR%\bin\debug"
copy "%CEF_SRC_DIR%\Debug\v8_context_snapshot.bin" "%CEF_DST_DIR%\bin\debug"
copy "%CEF_SRC_DIR%\Debug\swiftshader\libEGL.dll" "%CEF_DST_DIR%\bin\debug\swiftshader"
copy "%CEF_SRC_DIR%\Debug\swiftshader\libGLESv2.dll" "%CEF_DST_DIR%\bin\debug\swiftshader"

@rem ******** include folder ********
@xcopy "%CEF_SRC_DIR%\include\*" "%CEF_DST_DIR%\include\cef\include\" /s

@rem ******** lib folder ********
copy "%CEF_SRC_DIR%\Debug\libcef.lib" "%CEF_DST_DIR%\lib\debug"
copy "%CEF_SRC_DIR%\Release\libcef.lib" "%CEF_DST_DIR%\lib\release"

@rem ******** resources folder ********
xcopy "%CEF_SRC_DIR%\resources\*" "%CEF_DST_DIR%\resources\" /s

:End
