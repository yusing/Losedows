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

#include "widget.h"
#include <imgui.h>

static int btn_count = 0; // set btn_count to 0 after using LItem()!
void       LItem(Tweak& tweak)
{
    if (tweak.can_toggle()){
        // Delete type should not have reg_value
        if (ImGui::RadioButton(tweak.title, tweak.enabled())){
            tweak.execute();
        }
        return;
    }
    if (tweak.is_user_input()){
        // user modifiable
        if (tweak.type == String){
            ImGui::InputTextWithHint(tweak.title, "String value", static_cast<LPSTR>(tweak.data),
                                     STRBUF_SIZE);
        }
        else{
            // DWORD
            ImGui::InputScalar(tweak.title, ImGuiDataType_U32, tweak.data, nullptr, nullptr, "%lx",
                               ImGuiInputTextFlags_CharsHexadecimal);
        }
        ImGui::SameLine();
        if (LButton("Apply", tweak)){
            tweak.execute();
        }
    }
    else{
        // non-modifiable
        ImGui::TextUnformatted(tweak.title);
        ImGui::SameLine();
        if (LButton("Apply", tweak)){
            tweak.execute();
        }
    }
    if (tweak.value_default != nullptr){
        ImGui::SameLine();
        if (LButton("Restore", tweak)){
            tweak.restore_default();
        }
    }
    btn_count = 0;
}

bool LButton(const char* label, Tweak& tweak)
{
    ImGui::PushID(static_cast<int>(uintptr_t(&tweak) + btn_count));
    const auto clicked = ImGui::Button(label);
    ImGui::PopID();
    ++btn_count;
    return clicked;
}
