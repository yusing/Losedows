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

#include <Windows.h>
static OSVERSIONINFOEX os_info;
// Windows 10 Build number see: https://www.anoopcnair.com/windows-10-build-numbers-version-numbers/
// Windows 10 version 20H2 19042
// Windows 10 version 2004 19041
// Windows 10 version 1909 18363
// Windows 10 version 1903 18362
// Windows 10 version 1809 17763
// Windows 10 version 1803 17134
// Windows 10 version 1709 16299
// Windows 10 version 1703 15063
// Windows 10 version 1607 14393
// Windows 10 version 1511 10586
// Windows 10 version 1507 10240
bool init_os_info()
{
    NTSTATUS (WINAPI *RtlGetVersion)(LPOSVERSIONINFOEX);

    *(FARPROC*)&RtlGetVersion   = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");
    os_info.dwOSVersionInfoSize = sizeof(os_info);
    if (RtlGetVersion == nullptr){
        return false;
    }
    return RtlGetVersion(&os_info) == 0; // 0 == STATUS_SUCCESS
}

OSVERSIONINFOEX* get_os_info()
{
    return &os_info;
}
