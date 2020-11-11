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

#include "Windows.h"
#ifdef BACKEND_USE_SDL2
#   include "init_graphics_sdl2.h"
#   undef main
#else
#   include "init_graphics.h"
#endif
#include "log.h"
extern bool             init_os_info();
extern OSVERSIONINFOEX* get_os_info();

int main(void)
{
    if (!init_os_info()){
        MessageBoxA(GetActiveWindow(), "Program will now exit.", "Unable to retrieve Windows version", MB_ICONERROR);
        exit(1);
    }
    /* check if OS is Windows 10 */
    auto* os_info = get_os_info();

    if (os_info->dwMajorVersion != 10){
        MessageBoxA(GetActiveWindow(), "This program supports Windows 10 only.", "Unsupported Windows Version",
                    MB_ICONERROR);
        exit(1);
    }

    log("Windows version: %lu.%lu build %lu", os_info->dwMajorVersion, os_info->dwMinorVersion, os_info->dwBuildNumber);
    init_graphics();
    return 0;
}
