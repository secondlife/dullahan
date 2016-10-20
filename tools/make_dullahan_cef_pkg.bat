set SRC_DIR="%USERPROFILE%\Desktop\cef_binary_3.2704.1434.gec3e9ed_windows32"
set DST_DIR="%USERPROFILE%\Desktop\cef_2704.1434.win32"

mkdir %DST_DIR%

if not exist %SRC_DIR% goto end
if not exist %DST_DIR% goto end

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
cmake -G "Visual Studio 12" ..
cd libcef_dll_wrapper

rem swap /MT for /MD because that's what we use (cringe - why isn't this an option in CMake)
powershell -Command "(get-content libcef_dll_wrapper.vcxproj) | ForEach-Object { $_ -replace '>MultiThreadedDebug<', '>MultiThreadedDebugDLL<' } | set-content libcef_dll_wrapper.vcxproj"
powershell -Command "(get-content libcef_dll_wrapper.vcxproj) | ForEach-Object { $_ -replace '>MultiThreaded<', '>MultiThreadedDLL<' } | set-content libcef_dll_wrapper.vcxproj"

msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Debug"
msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Release"

copy "Debug\libcef_dll_wrapper.lib" %DST_DIR%"\lib\debug"
copy "Release\libcef_dll_wrapper.lib" %DST_DIR%"\lib\release"

popd

rem ******** bin folder ********
copy %SRC_DIR%"\Release\d3dcompiler_43.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\d3dcompiler_47.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\libcef.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\libEGL.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\libGLESv2.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\natives_blob.bin" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\snapshot_blob.bin" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\widevinecdmadapter.dll" %DST_DIR%"\bin\release"
copy %SRC_DIR%"\Release\wow_helper.exe" %DST_DIR%"\bin\release"

rem ******** bin include ********
xcopy %SRC_DIR%"\include\*" %DST_DIR%"\include\" /S

rem ******** lib folder ********
copy %SRC_DIR%"\Debug\libcef.lib" %DST_DIR%"\lib\debug"
copy %SRC_DIR%"\Release\libcef.lib" %DST_DIR%"\lib\release"

rem ******** resources folder ********
xcopy %SRC_DIR%"\resources\*" %DST_DIR%"\resources\" /S

:END
