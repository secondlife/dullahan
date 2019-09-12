@rem Chromium/CEF build script
@rem
@rem This batch file worked up using instructions from this CEF page:
@rem https://bitbucket.org/chromiumembedded/cef/wiki/MasterBuildQuickStart.md
@rem
@rem It builds Chromium/CEF and creates a CEF package much like what can
@rem be downloaded from the Spotify CEF site. If everything works [and it takes
@rem a long time to build - 10-12 hours at least] then a CEF package will appear
@rem in the %ROOT_CODE_DIRECTORY%\chromium_git\chromium\src\cef\binary_distrib dir
@rem
@rem Parse command line: for example:
@rem    build_cef_src_win.bat c:\cef    build in C:\cef - defaults for other settings
@rem    build_cef_src_win.bat - - 0 3500    to turn off codecs and build branch 3500
@rem    build_cef_src_win.bat - 32 - 3807    to enable 32 bit build of branch 3807

@rem default values - change via command line parameters - see below
@set DEFAULT_ROOT_CODE_DIRECTORY=\code
@set DEFAULT_BIT_WIDTH=64
@set DEFAULT_PROPRIETARY_CODEC=1
@set DEFAULT_BRANCH=3809

@rem Pass in the name of the directory where the build happens as param 1 or '-' to use default
@set ROOT_CODE_DIRECTORY=%DEFAULT_ROOT_CODE_DIRECTORY%
@if not [%1]==[] ( if not [%1]==[-] (set ROOT_CODE_DIRECTORY=%~f1))

@rem Pass in bit width [32/64] of the build as param 2 or '-' to use default
@set BIT_WIDTH=%DEFAULT_BIT_WIDTH%
@if not [%2]==[] ( if not [%2]==[-] (set BIT_WIDTH=%2))

@rem Pass in 1/0 to enable/disable proprietary codecs as param 3 or '-' to use default
@set PROPRIETARY_CODEC=%DEFAULT_PROPRIETARY_CODEC%
@if not [%3]==[] ( if not [%3]==[-] (set PROPRIETARY_CODEC=%3))

@rem Pass in the branch number to build as param 4 or '-' to use default
@set BRANCH=%DEFAULT_BRANCH%
@if not [%4]==[] ( if not [%4]==[-] (set BRANCH=%4))

@mkdir %ROOT_CODE_DIRECTORY%\automate
@mkdir %ROOT_CODE_DIRECTORY%\chromium_git
@mkdir %ROOT_CODE_DIRECTORY%\depot_tools

@cd %ROOT_CODE_DIRECTORY% 

@rem Chromium/CEF build scripts need this directory in the path
@set PATH=%ROOT_CODE_DIRECTORY%\depot_tools;%PATH%

@rem initialize build_details file
@echo Build details: > %ROOT_CODE_DIRECTORY%\build_details

@rem record build settings
@echo ROOT_CODE_DIRECTORY: %ROOT_CODE_DIRECTORY% >> %ROOT_CODE_DIRECTORY%\build_details
@echo BIT_WIDTH: %BIT_WIDTH% >> %ROOT_CODE_DIRECTORY%\build_details
@echo PROPRIETARY_CODEC: %PROPRIETARY_CODEC% >> %ROOT_CODE_DIRECTORY%\build_details
@echo BRANCH: %BRANCH% >> %ROOT_CODE_DIRECTORY%\build_details
@echo. >> %ROOT_CODE_DIRECTORY%\build_details

@rem rudimentary timing
@echo Start build: >> %ROOT_CODE_DIRECTORY%\build_details
@time /t >> %ROOT_CODE_DIRECTORY%\build_details
@echo. >> %ROOT_CODE_DIRECTORY%\build_details

@rem grab a recent version of the depot tools using built in curl [.exe extension is important]
@powershell.exe -NoP -NonI -Command "curl.exe -O https://storage.googleapis.com/chrome-infra/depot_tools.zip"

@rem uncompress the zip file using a Powershell command
@powershell.exe -NoP -NonI -Command "Expand-Archive 'depot_tools.zip' '.\depot_tools\'"

@rem Rudimentary timing
@echo Downloaded and unzipped depot tools build: >> %ROOT_CODE_DIRECTORY%\build_details
@time /t >> %ROOT_CODE_DIRECTORY%\build_details
@echo. >> %ROOT_CODE_DIRECTORY%\build_details

@rem run the Google batch file to update it to latest version via git pull
@cd %ROOT_CODE_DIRECTORY%\depot_tools
@call update_depot_tools.bat

@rem Rudimentary timing
@echo Ran update_depot_tools.bat: >> %ROOT_CODE_DIRECTORY%\build_details
@time /t >> %ROOT_CODE_DIRECTORY%\build_details
@echo. >> %ROOT_CODE_DIRECTORY%\build_details

@rem grab latest version of the main python script
@cd %ROOT_CODE_DIRECTORY%\automate
@powershell.exe -NoP -NonI -Command "curl.exe -O https://bitbucket.org/chromiumembedded/cef/raw/master/tools/automate/automate-git.py"

@rem Starting point for automate-git.py step
@cd %ROOT_CODE_DIRECTORY%\chromium_git

@rem Settings taking from the Chromium/CEF Master Build Page.
@set GN_ARGUMENTS=--ide=vs2017 --sln=cef --filters=//cef/*

@rem Not everyone wants the official media codec support
@set GN_DEFINES=is_official_build=true
@if "%PROPRIETARY_CODEC%"=="1" (set GN_DEFINES=is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome)

@rem Allow building of both 32 and 64 bit versions
@set BUILD_64BIT_FLAGS=
@if "%BIT_WIDTH%"=="64" (set BUILD_64BIT_FLAGS=--x64-build)

@rem Rudimentary timing
@echo About to run automate-git.py: >> %ROOT_CODE_DIRECTORY%\build_details
@time /t >> %ROOT_CODE_DIRECTORY%\build_details
@echo. >> %ROOT_CODE_DIRECTORY%\build_details

@rem This is the maion build step and does everything we need
cd %ROOT_CODE_DIRECTORY%\chromium_git\chromium\src\cef
@python ..\automate\automate-git.py^
 --download-dir=%ROOT_CODE_DIRECTORY%\chromium_git^
 --depot-tools-dir=%ROOT_CODE_DIRECTORY%\depot_tools^
 --branch=%BRANCH%^
 --client-distrib^
 --force-clean^
 %BUILD_64BIT_FLAGS%

@rem Rudimentary timing
@echo Ran automate-git.py: >> %ROOT_CODE_DIRECTORY%\build_details
@time /t >> %ROOT_CODE_DIRECTORY%\build_details
@echo. >> %ROOT_CODE_DIRECTORY%\build_details

@rem Rudimentary timing
@cd %ROOT_CODE_DIRECTORY%
@echo End build: >> %ROOT_CODE_DIRECTORY%\build_details
@time /t >> %ROOT_CODE_DIRECTORY%\build_details
@echo. >> %ROOT_CODE_DIRECTORY%\build_details
@echo Build details:
@type %ROOT_CODE_DIRECTORY%\build_details

@echo.
@echo If all went well, zipped builds will be in %ROOT_CODE_DIRECTORY%\chromium_git\chromium\src\cef\binary_distrib
@echo.

:end
