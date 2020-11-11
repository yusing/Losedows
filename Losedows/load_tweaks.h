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
#include <string>

enum class CheckType {
    Prohibit,
    Required,
    Optional
};

struct TweakPackVersion {
    int major;
    int minor;
};

void clear_tweaks();
void add_custom_tweak(const std::string& file);
void        load_all_tweaks();
const char* load_tweak(const char* tweak_data);
void        load_tweak_from_file(const std::string& file);

void        load_tweak_pack(const std::string& file);
bool        is_invalid(const std::string& file_name, const char* tweak_data);

/**
 * \brief pack tweak files them into tkpack
 * \tparam ContainerOfString a container of string (vector and deque only)
 * \param version pack version
 * \param tweak_files list of files to pack
 */
template <class ContainerOfString>
void pack_tweaks(const TweakPackVersion& version, ContainerOfString tweak_files);