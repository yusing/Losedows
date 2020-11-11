@echo off
mkdir \lib\x86
mkdir \lib\x64
git clean -fdX curl
IF NOT EXIST .\curl\projects\Windows\VC15\lib\libcurl.sln (cmd /c "cd curl\projects && generate.bat vc15")
devenv .\curl\projects\Windows\VC15\lib\libcurl.sln /upgrade
msbuild .\curl\projects\Windows\VC15\lib\libcurl.sln /p:Configuration="DLL Release" /p:Platform="Win32"
msbuild .\curl\projects\Windows\VC15\lib\libcurl.sln /p:Configuration="DLL Release" /p:Platform="x64"
copy /Y ".\curl\build\Win32\VC15\DLL Release\libcurl.dll" ".\lib\x86"
copy /Y ".\curl\build\Win32\VC15\DLL Release\libcurl.lib" ".\lib\x86"
copy /Y ".\curl\build\Win64\VC15\DLL Release\libcurl.dll" ".\lib\x64"
copy /Y ".\curl\build\Win64\VC15\DLL Release\libcurl.lib" ".\lib\x64"