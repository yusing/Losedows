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

#include "ui.h"
#include "check_update.h"
#include "tweaks.h"
#include "widget.h"
#include "load_tweaks.h"
#include "theme.h"
#include "theme_editor.h"
#include "windows_impl.h"
#include "log.h"
#include <imgui.h>
#include <thread>
#include <deque>
extern std::unordered_map<std::string, std::vector<Tweak>*> tweaks_read;
extern std::vector<std::string>                             custom_tweaks;
extern ImFont*                                              console_font;
extern HWND                                                 hwnd;

int  scroll_requests   = 0;
bool show_logs         = false;
bool show_theme_editor = false;
bool show_advanced_options = false;
RECT client_rect{0, 0, 0, 0};

void init(ImGuiIO& io)
{
    // init
    apply_font(io);

    if (!load_current_theme()){
        apply_default_theme();
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    load_all_tweaks();
}


void draw()
{
    ImGui::SetNextWindowPos({0, 0});
    if (ImGui::Begin("Main Window", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_AlwaysAutoResize)){
        if (ImGui::Button("Reload tweaks")){
            load_all_tweaks();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart explorer")){
            restart_explorer();
        }
        ImGui::SameLine();
        if (ImGui::Button("Check for tweak update")){
            check_update();
        }
        ImGui::Checkbox("Show Advanced options", &show_advanced_options);
        if (show_advanced_options){
            ImGui::Checkbox("Show logs", &show_logs);
            ImGui::SameLine();
            ImGui::Checkbox("Show theme editor", &show_theme_editor);
            ImGui::SameLine();
            if (ImGui::Button("Test tweak file(s)")) {
                for (auto& file : open_file_dialog_multi()) {
                    custom_tweaks.push_back(file);
                    load_tweak_from_file(file);
                    add_custom_tweak(file);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Pack tweaks")) {
                pack_tweaks({ 0, 1 }, open_file_dialog_multi());
            }
            ImGui::SameLine();
            if (ImGui::Button("Load tweak pack")) {
                std::string file = open_file_dialog();
                if (!file.empty()) {
                    load_tweak_pack(file);
                    add_custom_tweak(file);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear tweaks")) {
                clear_tweaks();
            }
        }
        ImGui::Spacing();
        if (ImGui::BeginTabBar("##tabs")){
            for (auto& _ : tweaks_read){
                if (ImGui::BeginTabItem(_.first.c_str())){
                    for (auto& item : *_.second){
                        LItem(item);
                    }
                    ImGui::EndTabItem();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1)); // reduce gpu usage for high refresh rate monitor
                }
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End(); // Main Window
    if (show_logs){
        if (console_font != nullptr){
            ImGui::PushFont(console_font);
        }
        // get client window size
        GetWindowRect(hwnd, &client_rect);
        // disallow the log windows to grow more than 80% of the client width and 60% of client height
        ImGui::SetNextWindowSizeConstraints({0, 0}, {client_rect.right * .8f, client_rect.bottom * .6f});
        if (ImGui::Begin("Logs", nullptr,
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysHorizontalScrollbar)){
            for (auto& line : get_logs()){
                if (ImGui::Selectable(line.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)){
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)){
                        // copy log content on double click
                        ImGui::SetClipboardText(line.c_str());
                        ++scroll_requests; // the last log is not displayed yet, so needed to scroll on next iteration
                    }
                }
            }
            if (scroll_requests > 0){
                ImGui::SetScrollHereY(1.0f);
                --scroll_requests;
            }
            if (ImGui::Button("Clear")){
                log_clear();
            }
        }
        if (console_font != nullptr){
            ImGui::PopFont();
        }
        ImGui::End(); // Logs
    }
    else{
        scroll_requests = 0; // clear scroll request when log is closed
    }
    if (show_theme_editor){
        theme_editor();
    }
}
