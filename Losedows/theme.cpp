// Copyright 2020 yusing
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
#define rgba(r,g,b,a) ImVec4(r/255.f, g/255.f, b/255.f, a/255)
#include "log.h"
#include "theme.h"
#include <imgui.h>
#include <filesystem>
#include <cstdio>
// Fonts
constexpr char TTF_SEGOE_UI[] = R"(C:\Windows\Fonts\segoeui.ttf)";
constexpr char TTF_CONSOLAS[] = R"(C:\Windows\Fonts\consola.ttf)";
inline ImFont* console_font;

namespace fs = std::filesystem;

void apply_font(ImGuiIO& io)
{
    if (fs::exists(TTF_SEGOE_UI)){
        io.Fonts->AddFontFromFileTTF(TTF_SEGOE_UI, 24.0f);
    }
    else{
        log("Warning: Font not found in \"%s\"", TTF_SEGOE_UI);
    }
    if (fs::exists(TTF_CONSOLAS)){
        console_font = io.Fonts->AddFontFromFileTTF(TTF_CONSOLAS, 18.0f);
    }
    else{
        log("Warning: Font not found in \"%s\"", TTF_CONSOLAS);
    }
}

void apply_default_theme()
{
    // TODO: support Unicode characters, e.g.
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.00f);
    style.Colors[ImGuiCol_PopupBg] = style.Colors[ImGuiCol_WindowBg];
    style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = rgba(255, 255, 255, 255);
    style.Colors[ImGuiCol_FrameBgHovered] = rgba(253, 85, 85, 16);
    style.Colors[ImGuiCol_FrameBgActive] = rgba(255, 181, 181, 255);
    style.Colors[ImGuiCol_TitleBg] = rgba(185, 185, 185, 135);
    style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_WindowBg];
    style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_WindowBg];
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
    style.Colors[ImGuiCol_CheckMark] = rgba(52, 73, 94, 255); // RadioButton
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
    style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_WindowBg];
    style.Colors[ImGuiCol_ButtonHovered] = rgba(217, 221, 221, 255);
    style.Colors[ImGuiCol_ButtonActive] = rgba(192, 185, 185, 255);
    style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f); //ImVec4(0.46f, 0.84f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
    style.Colors[ImGuiCol_Tab] = rgba(255, 235, 238, 255);
    style.Colors[ImGuiCol_TabHovered] = rgba(255, 184, 197, 255);
    style.Colors[ImGuiCol_TabActive] = rgba(255, 198, 198, 255);
    style.Alpha = 1.0f;
    style.FrameRounding = 4;
    style.IndentSpacing = 12.0f;
}

bool save_theme()
{
    FILE*      theme_file;
    const auto path  = fs::current_path().string() + "\\current.tm";
    auto       error = fopen_s(&theme_file, path.c_str(), "w");
    if (error != 0 || theme_file == nullptr){
        log("Error %d, failed to open theme file \"current.tm\" for write", error);
        return false;
    }
    auto& colors = ImGui::GetStyle().Colors;
    fwrite(colors, sizeof colors, 1, theme_file);
    if ((error = ferror(theme_file)) != 0){
        log("Error %d when writing to file \"current.tm\"", error);
    }
    else{
        log("Theme saved to \"current.tm\"");
    }
    fclose(theme_file);
    return error == 0;
}

bool load_theme(const char* file)
{
    FILE* theme_file;
    auto  error = fopen_s(&theme_file, file, "r");
    if (error != 0 || theme_file == nullptr){
        log("Error %d, failed to open theme file \"%s\" for read", error, file);
        return false;
    }
    auto& colors = ImGui::GetStyle().Colors;
    fread(colors, sizeof colors, 1, theme_file);
    if ((error = ferror(theme_file)) != 0){
        log("Error %d when writing to file \"current.tm\"", error);
    }
    else{
        log("Theme loaded from \"%s\"", file);
    }
    fclose(theme_file);
    return error == 0;
}

bool load_current_theme()
{
    const auto path = fs::current_path().string() + "\\current.tm";
    return load_theme(path.c_str());
}
