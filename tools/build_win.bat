pushd .
cd ..
if %1.==. goto NoBitWidth
if %1 == 64 goto BitWidth64

:BitWidth32
if exist .\build\* rmdir /s /q .\build
mkdir build
cd build
cmake -G "Visual Studio 12 2013" ^
      -DCEF_INCLUDE_DIR="c:\work\cef_builds\cef_2704.1434.win32\include" ^
      -DCEF_LIB_DIR="c:\work\cef_builds\cef_2704.1434.win32\lib" ^
      -DCEF_BIN_DIR="c:\work\cef_builds\cef_2704.1434.win32\bin" ^
      -DCEF_RESOURCE_DIR="c:\work\cef_builds\cef_2704.1434.win32\resources" ^
      ..
goto Build

:BitWidth64
if exist .\build64\* rmdir /s /q .\build64
mkdir build64
cd build64
cmake -G "Visual Studio 12 2013 Win64" ^
      -DCEF_INCLUDE_DIR="c:\work\cef_builds\cef_2704.1434.win64\include" ^
      -DCEF_LIB_DIR="c:\work\cef_builds\cef_2704.1434.win64\lib" ^
      -DCEF_BIN_DIR="c:\work\cef_builds\cef_2704.1434.win64\bin" ^
      -DCEF_RESOURCE_DIR="c:\work\cef_builds\cef_2704.1434.win64\resources" ^
      ..
goto Build

:NoBitWidth
  ECHO You must specify a bit width of 32 or 64
goto End

:Build
msbuild dullahan.sln /p:Configuration=Release
cd Release
.\webcube.exe

:End

popd
