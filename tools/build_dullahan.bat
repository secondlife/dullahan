@rem This new batch file builds the CEF wrapper and then Dullahan once
@rem you set the variables below: CEF_VERSION, CHROMIUM_VERSION and CEF_SRC_DST_DIR
@rem Eventually, it will replace both make_dullahan_cef_pkg.bat and build_win.bat.
@rem There is still some work to do: (automated download of src, drop into local dir vs
@rem have to specify a cef_builds folder, etc.) but this is a good start.

@pushd .
@set VS_CMD=Visual Studio 12 2013

@rem uncomment this line or change one above for VS2015 builds
@rem set VS_CMD=Visual Studio 14 2015

@rem Edit these variables to match the version of CEF & Chromium you want to use after
@rem deciding on the version you want to use from the Spotify open source build page
@rem here http://opensource.spotify.com/cefbuilds/index.html
@set CEF_VERSION=75.0.7+g19229b6
@set CHROMIUM_VERSION=75.0.3770.80

@rem Edit this variable to point to the directory where you stored the download and 
@rem where you would like to store the resulting Dullahan compatible CEF build
@set CEF_SRC_DST_DIR=c:\work\cef_builds

@rem This base name is common to both 32 and 64 bit versions.
@rem Do not edit unless Spotify change their naming scheme
@set BASE_NAME=cef_binary_%CEF_VERSION%+chromium-%CHROMIUM_VERSION%_

@rem Edit these variables to point to the uncompressed CEF source directory and
@rem the required destination directory for both 32 and 64 bit versions
@rem @remembering to express the locations in terms of the CEF_VERSION above
@set SRC_DIR_32=%CEF_SRC_DST_DIR%\%BASE_NAME%windows32
@set DST_DIR_32=%SRC_DIR_32%_dullahan
@set SRC_DIR_64=%CEF_SRC_DST_DIR%\%BASE_NAME%windows64
@set DST_DIR_64=%SRC_DIR_64%_dullahan

@if "%1"=="32" goto BitWidth32
@if "%1"=="64" goto BitWidth64

:NoBitWidth
@echo.
@echo You must specify a bit width of 32 or 64
@goto End

:BitWidth32
@set SRC_DIR=%SRC_DIR_32%
@set DST_DIR=%DST_DIR_32%
@set BUILD_DIR="build"
@set CMAKE_CMD="%VS_CMD%"
@set PLATFORM_CMD="/property:Platform=x86"
@goto skip_1

:BitWidth64
@set SRC_DIR=%SRC_DIR_64%
@set DST_DIR=%DST_DIR_64%
@set BUILD_DIR="build64"
@set CMAKE_CMD="%VS_CMD% Win64"

@set PLATFORM_CMD="/property:Platform=x64"

@goto skip_1

:skip_1
@if not exist %SRC_DIR% (
    @echo.
    @echo *** ERROR: %SRC_DIR% does not exist - check your settings ****
    @goto end
)

@if exist %DST_DIR% (
    @echo.
    @echo *** ERROR: %DST_DIR% exists - remove this directory before running this script ****
    @goto end
)

mkdir %DST_DIR%
@if not exist %DST_DIR% (
    @echo.
    @echo *** ERROR: %DST_DIR% cannot be created - check your settings ****
    @goto end
)

@mkdir "%DST_DIR%\bin\debug"
@mkdir "%DST_DIR%\bin\debug\swiftshader"
@mkdir "%DST_DIR%\bin\release"
@mkdir "%DST_DIR%\bin\release\swiftshader"
@mkdir "%DST_DIR%\include"
@mkdir "%DST_DIR%\lib\debug"
@mkdir "%DST_DIR%\lib\release"
@mkdir "%DST_DIR%\resources"

@pushd .

@cd /d %SRC_DIR%

@if exist %BUILD_DIR% del /s /q %BUILD_DIR%
@if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%

@mkdir %BUILD_DIR%
@cd /d %BUILD_DIR%
@cmake -G %CMAKE_CMD% ..
@cd libcef_dll_wrapper

@rem swap /MT for /MD because that's what we use (cringe - why isn't this an option in CMake)
powershell -Command "(get-content libcef_dll_wrapper.vcxproj) | ForEach-Object { $_ -replace '>MultiThreadedDebug<', '>MultiThreadedDebugDLL<' } | set-content libcef_dll_wrapper.vcxproj"
powershell -Command "(get-content libcef_dll_wrapper.vcxproj) | ForEach-Object { $_ -replace '>MultiThreaded<', '>MultiThreadedDLL<' } | set-content libcef_dll_wrapper.vcxproj"

msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Debug" %PLATFORM_CMD%
msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Release" %PLATFORM_CMD%

@copy "Debug\libcef_dll_wrapper.lib" "%DST_DIR%\lib\debug"
@copy "Release\libcef_dll_wrapper.lib" "%DST_DIR%\lib\release"

@popd

@rem ******** bin folder ********
@copy "%SRC_DIR%\Release\d3dcompiler_47.dll" "%DST_DIR%\bin\release"
@copy "%SRC_DIR%\Release\chrome_elf.dll" "%DST_DIR%\bin\release"
@copy "%SRC_DIR%\Release\libcef.dll" "%DST_DIR%\bin\release"
@copy "%SRC_DIR%\Release\libEGL.dll" "%DST_DIR%\bin\release"
@copy "%SRC_DIR%\Release\libGLESv2.dll" "%DST_DIR%\bin\release"
@copy "%SRC_DIR%\Release\natives_blob.bin" "%DST_DIR%\bin\release"
@copy "%SRC_DIR%\Release\snapshot_blob.bin" "%DST_DIR%\bin\release"
@copy "%SRC_DIR%\Release\v8_context_snapshot.bin" "%DST_DIR%\bin\release"
@copy "%SRC_DIR%\Release\swiftshader\libEGL.dll" "%DST_DIR%\bin\release\swiftshader"
@copy "%SRC_DIR%\Release\swiftshader\libGLESv2.dll" "%DST_DIR%\bin\release\swiftshader"

@copy "%SRC_DIR%\Debug\d3dcompiler_47.dll" "%DST_DIR%\bin\debug"
@copy "%SRC_DIR%\Debug\chrome_elf.dll" "%DST_DIR%\bin\debug"
@copy "%SRC_DIR%\Debug\libcef.dll" "%DST_DIR%\bin\debug"
@copy "%SRC_DIR%\Debug\libEGL.dll" "%DST_DIR%\bin\debug"
@copy "%SRC_DIR%\Debug\libGLESv2.dll" "%DST_DIR%\bin\debug"
@copy "%SRC_DIR%\Debug\natives_blob.bin" "%DST_DIR%\bin\debug"
@copy "%SRC_DIR%\Debug\snapshot_blob.bin" "%DST_DIR%\bin\debug"
@copy "%SRC_DIR%\Debug\v8_context_snapshot.bin" "%DST_DIR%\bin\debug"
@copy "%SRC_DIR%\Debug\swiftshader\libEGL.dll" "%DST_DIR%\bin\debug\swiftshader"
@copy "%SRC_DIR%\Debug\swiftshader\libGLESv2.dll" "%DST_DIR%\bin\debug\swiftshader"

@rem ******** include folder ********
@xcopy "%SRC_DIR%\include\*" "%DST_DIR%\include\" /S

@rem ******** lib folder ********
@copy "%SRC_DIR%\Debug\libcef.lib" "%DST_DIR%\lib\debug"
@copy "%SRC_DIR%\Release\libcef.lib" "%DST_DIR%\lib\release"

@rem ******** resources folder ********
@xcopy "%SRC_DIR%\resources\*" "%DST_DIR%\resources\" /S

@popd

@if exist .\src\dullahan_version.h del .\src\dullahan_version.h
@if exist .\%BUILD_DIR%\* rmdir /s /q .\%BUILD_DIR%
@mkdir %BUILD_DIR%
@cd /d %BUILD_DIR%
@cmake -G %CMAKE_CMD% ^
       -DCEF_INCLUDE_DIR="%DST_DIR%\include" ^
       -DCEF_LIB_DIR="%DST_DIR%\lib" ^
       -DCEF_BIN_DIR="%DST_DIR%\bin" ^
       -DCEF_RESOURCE_DIR="%DST_DIR%\resources" ^
       ..
@if errorlevel 1 goto End

msbuild dullahan.sln /p:Configuration=Debug %PLATFORM_CMD%
@if errorlevel 1 goto End
msbuild dullahan.sln /p:Configuration=Release %PLATFORM_CMD%
@if errorlevel 1 goto End

@start dullahan.sln

:End

@popd
