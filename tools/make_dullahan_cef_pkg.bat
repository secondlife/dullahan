@pushd .
set VS_CMD=Visual Studio 12 2013

rem uncomment this line or change one above for VS2015 builds
rem set VS_CMD=Visual Studio 14 2015

rem Edit these variables to point to the uncompressed CEF source directory and 
rem the required destination directory for both 32 and 64 bit versions
rem Note: I get the default "src" CEF packages from the Spotify CEF builds site 
rem here http://opensource.spotify.com/cefbuilds/index.html
set SRC_DIR_32="%USERPROFILE%\Desktop\cef_binary_3.3239.1716.g735b746_windows32"
set DST_DIR_32="%USERPROFILE%\Desktop\cef_3239.1716.g735b746_windows32"
set SRC_DIR_64="%USERPROFILE%\Desktop\cef_binary_3.3239.1716.g735b746_windows64"
set DST_DIR_64="%USERPROFILE%\Desktop\cef_3239.1716.g735b746_windows64"

@if "%1"=="32" goto BitWidth32
@if "%1"=="64" goto BitWidth64

:NoBitWidth
@echo.
@echo You must specify a bit width of 32 or 64
@goto End

:BitWidth32
set SRC_DIR=%SRC_DIR_32%
set DST_DIR=%DST_DIR_32%
set BUILD_DIR="build"
set CMAKE_CMD=%VS_CMD%
set PLATFORM_CMD="/property:Platform=x86"
goto skip_1

:BitWidth64
set SRC_DIR=%SRC_DIR_64%
set DST_DIR=%DST_DIR_64%
set BUILD_DIR="build64"
set CMAKE_CMD=%VS_CMD% Win64
set PLATFORM_CMD="/property:Platform=x64"
goto skip_1

:skip_1
if not exist %SRC_DIR% goto end
mkdir %DST_DIR%
if not exist %DST_DIR% goto end

mkdir %DST_DIR%"\bin\debug"
mkdir %DST_DIR%"\bin\release"
mkdir %DST_DIR%"\include"
mkdir %DST_DIR%"\lib\debug"
mkdir %DST_DIR%"\lib\release"
mkdir %DST_DIR%"\resources"

pushd .

cd /d %SRC_DIR%

if exist %BUILD_DIR% del /s /q %BUILD_DIR%
if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%

mkdir %BUILD_DIR%
cd /d %BUILD_DIR% 
cmake -G "%CMAKE_CMD%" ..
cd libcef_dll_wrapper

rem swap /MT for /MD because that's what we use (cringe - why isn't this an option in CMake)
powershell -Command "(get-content libcef_dll_wrapper.vcxproj) | ForEach-Object { $_ -replace '>MultiThreadedDebug<', '>MultiThreadedDebugDLL<' } | set-content libcef_dll_wrapper.vcxproj"
powershell -Command "(get-content libcef_dll_wrapper.vcxproj) | ForEach-Object { $_ -replace '>MultiThreaded<', '>MultiThreadedDLL<' } | set-content libcef_dll_wrapper.vcxproj"

msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Debug" %PLATFORM_CMD%
msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Release" %PLATFORM_CMD%

copy "Debug\libcef_dll_wrapper.lib" %DST_DIR%"\lib\debug"
copy "Release\libcef_dll_wrapper.lib" %DST_DIR%"\lib\release"

popd

rem ******** bin folder ********
copy %SRC_DIR%"\Release\d3dcompiler_43.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\d3dcompiler_47.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\chrome_elf.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\libcef.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\libEGL.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\libGLESv2.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\natives_blob.bin" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\snapshot_blob.bin" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\v8_context_snapshot.bin" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\widevinecdmadapter.dll" %DST_DIR%"\bin\release"

copy %SRC_DIR%"\Release\d3dcompiler_43.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\d3dcompiler_47.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\chrome_elf.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\libcef.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\libEGL.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\libGLESv2.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\natives_blob.bin" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\snapshot_blob.bin" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\v8_context_snapshot.bin" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\widevinecdmadapter.dll" %DST_DIR%"\bin\debug"

rem ******** include folder ********
xcopy %SRC_DIR%"\include\*" %DST_DIR%"\include\" /S

rem ******** lib folder ********
copy %SRC_DIR%"\Debug\libcef.lib" %DST_DIR%"\lib\debug"
copy %SRC_DIR%"\Release\libcef.lib" %DST_DIR%"\lib\release"

rem ******** resources folder ********
xcopy %SRC_DIR%"\resources\*" %DST_DIR%"\resources\" /S

:End

popd
