@pushd .

@if "%1"=="32" goto BitWidth32
@if "%1"=="64" goto BitWidth64

:NoBitWidth
@echo.
@echo You must specify a bit width of 32 or 64
@goto End

:BitWidth32
set SRC_DIR="%USERPROFILE%\Desktop\cef_binary_3.2987.1601.gf035232_windows32"
set DST_DIR="%USERPROFILE%\Desktop\cef_2987.1601.win32"
set CMAKE_CMD="Visual Studio 12 2013"
set PLATFORM_CMD="/property:Platform=x86"
goto skip_1

:BitWidth64
set SRC_DIR="%USERPROFILE%\Desktop\cef_binary_3.2987.1601.gf035232_windows64"
set DST_DIR="%USERPROFILE%\Desktop\cef_2987.1601.win64"
set CMAKE_CMD="Visual Studio 12 2013 Win64"
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

cd %SRC_DIR%

if exist build del /s /q build
if exist build rmdir /s /q build

mkdir build
cd build 
cmake -G %CMAKE_CMD% ..
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
copy %SRC_DIR%"\Release\widevinecdmadapter.dll" %DST_DIR%"\bin\release"

copy %SRC_DIR%"\Release\d3dcompiler_43.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\d3dcompiler_47.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\chrome_elf.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\libcef.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\libEGL.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\libGLESv2.dll" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\natives_blob.bin" %DST_DIR%"\bin\debug"
copy %SRC_DIR%"\Release\snapshot_blob.bin" %DST_DIR%"\bin\debug"
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
