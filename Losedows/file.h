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
#define IOS_IN_BINARY (std::ios::in | std::ios::binary)
#define IOS_IN (std::ios::in)
#define IOS_OUT_BINARY (std::ios::out | std::ios::binary)
#define IOS_OUT (std::ios::out)
#include <fstream>
bool open_file(const std::string& file_path, std::fstream& stream, std::ios_base::openmode mode);
std::string current_path();
std::string filename(const std::string& path);
std::string file_extension(const std::string& path);
bool file_exists(const std::string& file);
