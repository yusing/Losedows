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

#include "file.h"
#include <filesystem>

std::string str_current_path;

bool open_file(const std::string& file_path, std::fstream& stream, std::ios_base::openmode mode)
{
    stream.open(file_path, mode);
    if (stream.fail() || stream.bad()){
        return false;
    }
    return true;
}

std::string current_path()
{
    if (str_current_path.empty()){
        str_current_path = std::filesystem::current_path().string();
    }
    return str_current_path;
}

std::string filename(const std::string& path)
{
    return std::filesystem::path(path).filename().string();
}


std::string file_extension(const std::string& path)
{
    return std::filesystem::path(path).extension().string();
}

bool file_exists(const std::string& file)
{
    return std::filesystem::exists(file);
}
