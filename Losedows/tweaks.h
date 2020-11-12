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
#include "reg.h"
#include <vector>

enum _TweakType {
    Invalid,
    CreateSubKey = 1 << 1,
    DeleteKey = 1 << 2,
    CreateValue = 1 << 3,
    DeleteValue = 1 << 4,
    Bool = 1 << 5,
    Dword = 1 << 6,
    String = 1 << 7
};

typedef unsigned int TweakType;
typedef char*        Title;
typedef char*        RegKey;
typedef char*        RegValue;
typedef DWORD        RegType;

struct Tweak {
    TweakType           type;
    Title               title;
    RegKey              reg_key;
    RegValue            reg_value;
    DATA                value_default;
    DATA                value_optimal;
    DATA                data;
    std::vector<Tweak*> sub_tweaks; // array of sub tweaks
    DWORD               dw_enabled; // for toggle type

    void   dispose() const;
    void   toggle();
    bool   can_toggle() const;
    bool   is_user_input() const;
    DWORD  data_size() const; // used for reg_set_value
    DWORD  data_size_max() const;
    DWORD  reg_type() const;
    DWORD& enabled();
    DWORD& dw_value() const;
    LPSTR  str_value() const;

    bool execute(LPSTR override_key = nullptr);
    bool restore_default();
};

void init_tweak_list(std::vector<Tweak>& tweaks);
static_assert(sizeof(DWORD) == sizeof(int));
