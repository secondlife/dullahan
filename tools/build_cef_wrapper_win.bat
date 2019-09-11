
@set CEF_SRC_DIR="C:\Users\callum\Desktop\cef_binary_76.1.13+gf19c584+chromium-76.0.3809.132_windows32"
@set CEF_DST_DIR="C:\Users\callum\Desktop\cef_32"

@echo %CEF_SRC_DIR%|find "windows64" >nul
@if errorlevel 1 (
    set BIT_WIDTH=32
) else (
    set BIT_WIDTH=64
)

@if not exist %CEF_SRC_DIR% (
    @echo.
    @echo *** ERROR: %CEF_SRC_DIR% does not exist - check your settings ****
    @goto end
)

@if exist %CEF_DST_DIR% (
    @echo.
    @echo *** ERROR: %CEF_DST_DIR% exists - remove this directory before running this script ****
    @goto end
)

mkdir %CEF_DST_DIR%
@if not exist %CEF_DST_DIR% (
    @echo.
    @echo *** ERROR: %CEF_DST_DIR% cannot be created - check your settings ****
    @goto end
)

@set VS_CMD=Visual Studio 15 2017

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

@echo Building CEF in .\%BUILD_DIR% for %BIT_WIDTH% bit using CEF from %CEF_SRC_DIR%
pause

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

msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Debug" %PLATFORM_CMD%
msbuild libcef_dll_wrapper.vcxproj /property:Configuration="Release" %PLATFORM_CMD%

copy Debug\libcef_dll_wrapper.lib %CEF_DST_DIR%\lib\debug
copy Release\libcef_dll_wrapper.lib %CEF_DST_DIR%\lib\release

@popd

@rem ******** bin folder ********
copy %CEF_SRC_DIR%\Release\d3dcompiler_47.dll %CEF_DST_DIR%\bin\release
copy %CEF_SRC_DIR%\Release\chrome_elf.dll %CEF_DST_DIR%\bin\release
copy %CEF_SRC_DIR%\Release\libcef.dll %CEF_DST_DIR%\bin\release
copy %CEF_SRC_DIR%\Release\libEGL.dll %CEF_DST_DIR%\bin\release
copy %CEF_SRC_DIR%\Release\libGLESv2.dll %CEF_DST_DIR%\bin\release
copy %CEF_SRC_DIR%\Release\natives_blob.bin %CEF_DST_DIR%\bin\release
copy %CEF_SRC_DIR%\Release\snapshot_blob.bin %CEF_DST_DIR%\bin\release
copy %CEF_SRC_DIR%\Release\v8_context_snapshot.bin %CEF_DST_DIR%\bin\release
copy %CEF_SRC_DIR%\Release\swiftshader\libEGL.dll %CEF_DST_DIR%\bin\release\swiftshader
copy %CEF_SRC_DIR%\Release\swiftshader\libGLESv2.dll %CEF_DST_DIR%\bin\release\swiftshader

copy %CEF_SRC_DIR%\Debug\d3dcompiler_47.dll %CEF_DST_DIR%\bin\debug
copy %CEF_SRC_DIR%\Debug\chrome_elf.dll %CEF_DST_DIR%\bin\debug
copy %CEF_SRC_DIR%\Debug\libcef.dll %CEF_DST_DIR%\bin\debug
copy %CEF_SRC_DIR%\Debug\libEGL.dll %CEF_DST_DIR%\bin\debug
copy %CEF_SRC_DIR%\Debug\libGLESv2.dll %CEF_DST_DIR%\bin\debug
copy %CEF_SRC_DIR%\Debug\natives_blob.bin %CEF_DST_DIR%\bin\debug
copy %CEF_SRC_DIR%\Debug\snapshot_blob.bin %CEF_DST_DIR%\bin\debug
copy %CEF_SRC_DIR%\Debug\v8_context_snapshot.bin %CEF_DST_DIR%\bin\debug
copy %CEF_SRC_DIR%\Debug\swiftshader\libEGL.dll %CEF_DST_DIR%\bin\debug\swiftshader
copy %CEF_SRC_DIR%\Debug\swiftshader\libGLESv2.dll %CEF_DST_DIR%\bin\debug\swiftshader

@rem ******** include folder ********
@xcopy %CEF_SRC_DIR%\include\* %CEF_DST_DIR%\include\cef\include\ /s

@rem ******** lib folder ********
copy %CEF_SRC_DIR%\Debug\libcef.lib %CEF_DST_DIR%\lib\debug
copy %CEF_SRC_DIR%\Release\libcef.lib %CEF_DST_DIR%\lib\release

@rem ******** resources folder ********
xcopy %CEF_SRC_DIR%\resources\* %CEF_DST_DIR%\resources\ /s

:End
