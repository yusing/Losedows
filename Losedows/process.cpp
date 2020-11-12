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

#include "windows_impl.h"
#include "log.h"
#include <string>
#include <Windows.h>
#include <tlhelp32.h>

void start_process(LPCSTR command)
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof si);
    si.cb = sizeof si;
    ZeroMemory(&pi, sizeof pi);

    if (CreateProcessA(command, nullptr, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)){
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        log("Process: started \"%s\" with PID %lu", command, pi.dwProcessId);
    }
}

std::vector<DWORD> get_pid_by_name(LPCSTR name)
{
    std::vector<DWORD> pids;

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof entry;
    HANDLE snap  = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //all processes

    if (!Process32First(snap, &entry)){
        return pids;
    }

    do{
        if (strcmp(entry.szExeFile, name) == 0){
            pids.emplace_back(entry.th32ProcessID);
        }
    }
    while (Process32Next(snap, &entry));
    return pids;
}

bool is_process_running(LPCSTR name)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof entry;
    HANDLE snap  = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //all processes

    if (!Process32First(snap, &entry)){
        return false;
    }

    do{
        if (strcmp(entry.szExeFile, name) == 0){
            return true;
        }
    }
    while (Process32Next(snap, &entry));
    return false;
}

void kill_process(LPCSTR proc_name)
{
    for (auto& pid : get_pid_by_name(proc_name)){
        HANDLE handle = OpenProcess(PROCESS_TERMINATE, false, pid);
        if (TerminateProcess(handle, 0)){
            log("Process: terminated process %s with PID %lu successfully", proc_name, pid);
        }
        else{
            log("Error: failed to terminate process %s with PID %lu", proc_name, pid);
        }
        CloseHandle(handle);
    }
}

void restart_explorer()
{
    // explorer should be restarted automatically
    kill_process("explorer.exe");

    // if not, start it manually
    if (!is_process_running("explorer.exe")){
        start_process(R"(C:\Windows\explorer.exe)");
    }
}
