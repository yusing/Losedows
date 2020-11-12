@echo off
; devenv imgui_lib\imgui_lib.sln /upgrade
msbuild imgui_lib\imgui_lib.sln -p:Configuration=Release;Platform="x86"
msbuild imgui_lib\imgui_lib.sln -p:Configuration=Release;Platform="x64"
msbuild imgui_lib\imgui_lib.sln -p:Configuration=Debug;Platform="x86"
msbuild imgui_lib\imgui_lib.sln -p:Configuration=Debug;Platform="x64"