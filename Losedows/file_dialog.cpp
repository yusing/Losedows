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

#include <deque>
#include "windows_impl.h"
#include <imgui.h>
#include <thread>

#include "init_graphics.h"

std::deque<std::string> open_file_dialog_multi()
{
    constexpr size_t   FILE_BUF_SIZE = 5*MAX_PATH;
    std::deque<std::string> files;
    OPENFILENAMEA      ofn;
    char*              buf = new char[FILE_BUF_SIZE]{'\0'};
    ZeroMemory(&ofn, sizeof ofn);
    ofn.lStructSize    = sizeof ofn;
    ofn.hwndOwner      = GetActiveWindow();
    ofn.lpstrFile      = buf;
    ofn.nMaxFile       = FILE_BUF_SIZE;
    ofn.lpstrFilter    = "Tweak File\0*.TWEAK\0";
    ofn.nFilterIndex   = 0;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle  = 0;
    ofn.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    LPSTR p            = buf, p_file = buf;
    
    if (!GetOpenFileNameA(&ofn)){
        return {};
    }
    auto i = 0;
    while (true){
        if (*p == '\0'){
            if (i == 0){
                files.push_back(p_file);
                ++i;
            }
            else{
                files.push_back(files[0] + "\\" + p_file);
            }
            p_file = ++p;
            if (*p == '\0'){
                break;
            }
        }
        ++p;
    }
    if (files.size() > 1){
        files.pop_front();
    }
    return files;
}

std::string open_file_dialog()
{
    OPENFILENAMEA    ofn;
    char             buf[MAX_PATH]{'\0'};
    ZeroMemory(&ofn, sizeof ofn);
    ofn.lStructSize    = sizeof ofn;
    ofn.hwndOwner      = GetActiveWindow();
    ofn.lpstrFile      = buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter    = "Tweak Pack\0*.TKPACK\0";
    ofn.nFilterIndex   = 0;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle  = 0;
    ofn.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (!GetOpenFileNameA(&ofn)){
        return {};
    }
    return buf;
}
