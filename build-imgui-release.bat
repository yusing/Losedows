@echo off
devenv imgui_lib\imgui_lib.sln /upgrade
msbuild imgui_lib\imgui_lib.sln -p:Configuration=Release /p:Platform="x86"
msbuild imgui_lib\imgui_lib.sln -p:Configuration=Release /p:Platform="x64"