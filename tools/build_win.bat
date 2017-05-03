@pushd .
@if exist build_win.bat cd ..

set CEF_32_DIR="c:\work\cef_builds\cef_3029.1611.g44e39a8_windows32"
set CEF_64_DIR="C:\work\cef_builds\cef_3029.1611.g44e39a8_windows64"

@if "%1"=="32" goto BitWidth32
@if "%1"=="64" goto BitWidth64

:NoBitWidth
@echo.
@echo You must specify a bit width of 32 or 64
@goto End

:BitWidth32
if exist .\build\* rmdir /s /q .\build
mkdir build
cd build
cmake -G "Visual Studio 12 2013" ^
      -DCEF_INCLUDE_DIR="%CEF_32_DIR%\include" ^
      -DCEF_LIB_DIR="%CEF_32_DIR%\lib" ^
      -DCEF_BIN_DIR="%CEF_32_DIR%\bin" ^
      -DCEF_RESOURCE_DIR="%CEF_32_DIR%\resources" ^
      ..
goto Build

:BitWidth64
if exist .\build64\* rmdir /s /q .\build64
mkdir build64
cd build64
cmake -G "Visual Studio 12 2013 Win64" ^
      -DCEF_INCLUDE_DIR="%CEF_64_DIR%\include" ^
      -DCEF_LIB_DIR="%CEF_64_DIR%\lib" ^
      -DCEF_BIN_DIR="%CEF_64_DIR%\bin" ^
      -DCEF_RESOURCE_DIR="%CEF_64_DIR%\resources" ^
      ..
goto Build

:Build
msbuild dullahan.sln /p:Configuration=Debug
msbuild dullahan.sln /p:Configuration=Release

cd Release
.\webcube.exe

:End

popd
