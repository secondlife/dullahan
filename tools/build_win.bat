@pushd .
@if exist build_win.bat cd ..

rem We are using the CEF 3202 branch (like Chrome 62) version that we built
rem from the Spotify CEF site using make_dullahan_cef_pkg.bat script in this folder.
set CEF_32_DIR="c:\work\cef_builds\cef_3202.1686.gd665578_windows32"
set CEF_64_DIR="C:\work\cef_builds\cef_3202.1686.gd665578_windows64"

@if "%1"=="32" goto BitWidth32
@if "%1"=="64" goto BitWidth64

:NoBitWidth
@echo.
@echo You must specify a bit width of 32 or 64
@goto End

:BitWidth32
if exist .\src\dullahan_version.h rm .\src\dullahan_version.h
if exist .\build\* rmdir /s /q .\build
mkdir build
cd build
cmake -G "Visual Studio 12 2013" ^
      -DCEF_INCLUDE_DIR="%CEF_32_DIR%\include" ^
      -DCEF_LIB_DIR="%CEF_32_DIR%\lib" ^
      -DCEF_BIN_DIR="%CEF_32_DIR%\bin" ^
      -DCEF_RESOURCE_DIR="%CEF_32_DIR%\resources" ^
      ..
if errorlevel 1 goto ErrorOut

msbuild dullahan.sln /p:Configuration=Debug /p:Platform="Win32"
if errorlevel 1 goto ErrorOut
msbuild dullahan.sln /p:Configuration=Release /p:Platform="Win32"
if errorlevel 1 goto ErrorOut

goto Finished

:BitWidth64
if exist .\src\dullahan_version.h rm .\src\dullahan_version.h
if exist .\build64\* rmdir /s /q .\build64
mkdir build64
cd build64
cmake -G "Visual Studio 12 2013 Win64" ^
      -DCEF_INCLUDE_DIR="%CEF_64_DIR%\include" ^
      -DCEF_LIB_DIR="%CEF_64_DIR%\lib" ^
      -DCEF_BIN_DIR="%CEF_64_DIR%\bin" ^
      -DCEF_RESOURCE_DIR="%CEF_64_DIR%\resources" ^
      ..
if errorlevel 1 goto ErrorOut

msbuild dullahan.sln /p:Configuration=Debug
if errorlevel 1 goto ErrorOut
msbuild dullahan.sln /p:Configuration=Release
if errorlevel 1 goto ErrorOut

goto Finished

:ErrorOut
@echo.
@echo [101;93m An error occured - look in console output for the reason [0m
goto End

:Finished
cd Release
.\webcube.exe

:End
echo ^<ESC^>[94m [94mBlue[0m
popd
