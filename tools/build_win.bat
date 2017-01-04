pushd .

cd ..

IF %1.==. GOTO NoBitWidth

IF %1 == 64 GOTO BitWidth64

:BitWidth32
if exist .\build\* rmdir /s /q .\build
mkdir build
cd build
cmake -G "Visual Studio 12 2013" -DCEF_DIR="c:\work\cef_builds\cef_2704.1434.win32" ..
GOTO Build

:BitWidth64
if exist .\build64\* rmdir /s /q .\build64
mkdir build64
cd build64
cmake -G "Visual Studio 12 2013 Win64" -DCEF_DIR="c:\work\cef_builds\cef_2704.1434.win64" ..
GOTO Build

:NoBitWidth
  ECHO You must specify a bit width of 32 or 64
GOTO End

:Build
msbuild dullahan.sln /p:Configuration=Release
cd Release
.\webcube.exe

:End

popd
