@pushd .
@if exist build_win.bat cd ..
set VS_CMD=Visual Studio 12 2013

rem uncomment this line or change one above for VS2015 builds
rem set VS_CMD=Visual Studio 14 2015

rem We are using the CEF branch that we built from the Spotify CEF site
rem using the make_dullahan_cef_pkg.bat script in this folder.
set CEF_32_DIR="c:\work\cef_builds\cef_3325.1750.gaabe4c4_windows32"
set CEF_64_DIR="C:\work\cef_builds\cef_3325.1750.gaabe4c4_windows64"

@if "%1"=="32" goto BitWidth32
@if "%1"=="64" goto BitWidth64

:NoBitWidth
@echo.
@echo You must specify a bit width of 32 or 64
@goto ErrorOut

:BitWidth32
set BUILD_DIR=build
set CMAKE_CMD="%VS_CMD%"
set CEF_DIR=%CEF_32_DIR%
set PLATFORM_CMD="/property:Platform=Win32"
goto Build

:BitWidth64
set BUILD_DIR=build64
set CMAKE_CMD="%VS_CMD% Win64"
set CEF_DIR=%CEF_64_DIR%
set PLATFORM_CMD="/property:Platform=x64"
goto Build

:Build
if exist .\src\dullahan_version.h del .\src\dullahan_version.h
if exist .\%BUILD_DIR%\* rmdir /s /q .\%BUILD_DIR%
mkdir %BUILD_DIR%
cd /d %BUILD_DIR%
cmake -G %CMAKE_CMD% ^
      -DCEF_INCLUDE_DIR="%CEF_DIR%\include" ^
      -DCEF_LIB_DIR="%CEF_DIR%\lib" ^
      -DCEF_BIN_DIR="%CEF_DIR%\bin" ^
      -DCEF_RESOURCE_DIR="%CEF_DIR%\resources" ^
      ..
if errorlevel 1 goto ErrorOut

msbuild dullahan.sln /p:Configuration=Debug %PLATFORM_CMD%
if errorlevel 1 goto ErrorOut
msbuild dullahan.sln /p:Configuration=Release %PLATFORM_CMD%
if errorlevel 1 goto ErrorOut

goto Finished

:ErrorOut
@echo.
@echo [101;93m An error occured - look in console output for the reason [0m
goto End

:Finished
start dullahan.sln

:End
popd
