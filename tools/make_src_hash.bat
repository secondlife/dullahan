rem This batch files generates a C++ header containing the last few digits of 
rem the MD5 hash of the cpp/h source files and is used as part of the version number
rem This is not used currently other than as a place to test more easily.
rem Instead, this code is transplanted and modified slightly in the MSVC solution file:
rem   dullahan -> Properties -> Build Events -> Pre-Build Event

copy ..\host\*.cpp %TMP%\dullahan_host_c 
copy ..\src\*.cpp %TMP%\dullahan_c
copy ..\src\*.h   %TMP%\dullahan_h
copy %TMP%\dullahan_host_c + %TMP%\dullahan_c + %TMP%\dullahan_h %TMP%\dullahan_src
PowerShell -C $hash_out=[System.BitConverter]::ToString([System.Security.Cryptography.MD5]::Create().ComputeHash([System.IO.File]::ReadAllBytes('%TMP%\dullahan_src'))).ToLowerInvariant().Replace('-', '').SubString(0,6); Write-Host "`#define SRC_HASH_VAL `""${hash_out}"`"" > ..\src\dullahan_src_hash.h
