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
#include <vector>
#include <deque>
#include <string>
#include <Windows.h>

/* process.cpp */
void               start_process(LPCSTR command);
void               kill_process(LPCSTR proc_name);
bool               is_process_running(LPCSTR name);
void               restart_explorer();
std::vector<DWORD> get_pid_by_name(LPCSTR name);

/* file dialog.cpp */
/// \brief Open file dialog to choose multiple files
/// \return list of selected files
std::deque<std::string> open_file_dialog_multi();
std::string             open_file_dialog();
