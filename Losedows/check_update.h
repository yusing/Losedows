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
#include "log.h"
#include <curl/curl.h>


inline size_t curl_write_to_buf(void* content, size_t size, size_t nmemb, void* buf)
{
    auto new_size = size * nmemb;
    memcpy(buf, static_cast<const char*>(content), new_size);
    return new_size;
}

inline void check_update()
{
    CURL* curl;
    curl = curl_easy_init();
    char buf[4];
    curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/yusing/Losedows/main/tkpack_ver.txt");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_to_buf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    int result = curl_easy_perform(curl);
    if(result == CURLE_OK){
        log("curl success: result\n%s", buf);
    }
    else{
        log("curl error %d", result);
    }
    curl_easy_cleanup(curl);
}
