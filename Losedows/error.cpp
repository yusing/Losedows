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

#include "error.h"

#include <Windows.h>

std::string last_error_string()
{
    auto err = GetLastError();
    if (err == ERROR_SUCCESS){
        return {};
    }
    LPSTR msg_buf = nullptr;
    auto  n_words = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err,
        GetUserDefaultUILanguage(), (LPSTR)&msg_buf, 0, nullptr);
    std::string msg(msg_buf, n_words);
    LocalFree(msg_buf);
    return msg;
}

// strerror_s see https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?redirectedfrom=MSDN&view=msvc-160
std::string errno_string(errno_t err)
{
    char msg_buf[94]{'\0'};
    strerror_s(msg_buf, err);
    return msg_buf;
}
