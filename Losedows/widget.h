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

#pragma once
#define LTreeNode(label) ImGui::TreeNodeEx((label), ImGuiTreeNodeFlags_DefaultOpen)
#include "tweaks.h"

/// <summary>
/// Tweak item widget
/// </summary>
/// <param name="tweak">tweak to render</param>
void LItem(Tweak& tweak);
/// <summary>
/// 
/// </summary>
/// <param name="label"></param>
/// <param name="tweak"></param>
/// <returns></returns>
bool LButton(const char* label, Tweak& tweak);
