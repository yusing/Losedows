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

#include "log.h"
#include "load_tweaks.h"
#include "file.h"
#include "check_update.h"
#include <urlmon.h>
#include <cstdlib>
#include <comdef.h>

void check_update()
{
    // check latest version
    IStream* stream;
    HRESULT  result = URLOpenBlockingStreamA(
        nullptr, "https://raw.githubusercontent.com/yusing/Losedows/master/tkpack_ver.txt", &stream, 0, nullptr);
    if (ERROR_SUCCESS != result){
        const _com_error err(result);
        log("Network Error: failed to check version, reason: \"%s\"", err.ErrorMessage());
        return;
    }
    char  buf[8]; // more than enough
    ULONG bytes_read;
    result = stream->Read(buf, sizeof buf, &bytes_read);
    if (ERROR_SUCCESS != result || bytes_read == 0){
        const _com_error err(result);
        log("Network Error: failed to check version, reason: \"%s\"", err.ErrorMessage());
        return;
    }
    buf[bytes_read] = '\0';
    stream->Release();
    const char*      minor_begin = std::find(buf, buf + bytes_read, '.') + 1;
    TweakPackVersion latest_version;
    latest_version.major = strtol(buf, nullptr, 10);
    latest_version.minor = strtol(minor_begin, nullptr, 10);
    log("Tweak: latest version is %d.%d, ", latest_version.major, latest_version.minor);

    // check current version
    auto         update_needed = false;
    std::fstream tkpack_fs;
    if (!open_file(current_path(), tkpack_fs, IOS_IN)){
        update_needed = true;
    }
    else{
        TweakPackVersion current_version;
        tkpack_fs.read(current_version, sizeof(TweakPackVersion));
        log_append("current version is %d.%d", current_version.major, current_version.minor);
        if (latest_version > current_version){
            update_needed = true;
        }
    }

    // begin update
    if (update_needed){
        result = URLDownloadToFileA(nullptr, "https://raw.githubusercontent.com/yusing/Losedows/master/current.tkpack",
                                    "current.tkpack", 0, nullptr);
        if (ERROR_SUCCESS != result){
            const _com_error err(result);
            log("Network Error: failed to download the latest tweak pack, message: \"%s\"", err.ErrorMessage());
        }
        else{
            log("Tweak: Tweak pack updated successfully");
        }
    }
    else{
        log("Tweak: current version is already the latest");
    }
}
