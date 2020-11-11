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

#include "imgui.h"
#include "theme_editor.h"
#include "theme.h"
#include <filesystem>

static int current_item = 0;
namespace fs = std::filesystem;

constexpr const char* color_codes[] = {
    "ImGuiCol_Text",
    "ImGuiCol_TextDisabled",
    "ImGuiCol_WindowBg", // Background of normal windows
    "ImGuiCol_ChildBg",  // Background of child windows
    "ImGuiCol_PopupBg",  // Background of popups, menus, tooltips windows
    "ImGuiCol_Border",
    "ImGuiCol_BorderShadow",
    "ImGuiCol_FrameBg", // Background of checkbox, radio button, plot, slider, text input
    "ImGuiCol_FrameBgHovered",
    "ImGuiCol_FrameBgActive",
    "ImGuiCol_TitleBg",
    "ImGuiCol_TitleBgActive",
    "ImGuiCol_TitleBgCollapsed",
    "ImGuiCol_MenuBarBg",
    "ImGuiCol_ScrollbarBg",
    "ImGuiCol_ScrollbarGrab",
    "ImGuiCol_ScrollbarGrabHovered",
    "ImGuiCol_ScrollbarGrabActive",
    "ImGuiCol_CheckMark",
    "ImGuiCol_SliderGrab",
    "ImGuiCol_SliderGrabActive",
    "ImGuiCol_Button",
    "ImGuiCol_ButtonHovered",
    "ImGuiCol_ButtonActive",
    "ImGuiCol_Header", // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    "ImGuiCol_HeaderHovered",
    "ImGuiCol_HeaderActive",
    "ImGuiCol_Separator",
    "ImGuiCol_SeparatorHovered",
    "ImGuiCol_SeparatorActive", "ImGuiCol_ResizeGrip", "ImGuiCol_ResizeGripHovered", "ImGuiCol_ResizeGripActive",
    "ImGuiCol_Tab", "ImGuiCol_TabHovered", "ImGuiCol_TabActive", "ImGuiCol_TabUnfocused", "ImGuiCol_TabUnfocusedActive",
    "ImGuiCol_PlotLines", "ImGuiCol_PlotLinesHovered", "ImGuiCol_PlotHistogram", "ImGuiCol_PlotHistogramHovered",
    "ImGuiCol_TextSelectedBg", "ImGuiCol_DragDropTarget", "ImGuiCol_NavHighlight",
    // Gamepad/keyboard: current highlighted item
    "ImGuiCol_NavWindowingHighlight", // Highlight window when using CTRL+TAB
    "ImGuiCol_NavWindowingDimBg",     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
    "ImGuiCol_ModalWindowDimBg",      // Darken/colorize entire screen behind a modal window, when one is active
};
static_assert(sizeof color_codes / sizeof color_codes[0] == ImGuiCol_COUNT,
    "color codes need to be updated, check enum ImGuiCol_ in <imgui.h>");

void theme_editor()
{
    if (ImGui::Begin("Theme editor"), nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize){
        if (ImGui::Combo("Color Code", &current_item, color_codes, ImGuiCol_COUNT)){ }
        ImGui::ColorEdit4(color_codes[current_item], (float*)&ImGui::GetStyle().Colors[current_item]);
        if (ImGui::Button("Save theme")) {
            save_theme();
        }
        if (ImGui::Button("Reload theme from file")) {
            load_current_theme();
        }
        ImGui::End();
    }
}
