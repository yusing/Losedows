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

#define LOG_BUF_SIZE 128
#define LOG_MAX_LINES 1000
#include <stdarg.h>
#include <deque>
#include <string>
#include <chrono>
#include <imgui.h>
#include "log.h"

std::deque<std::string> log_lines;
extern int              scroll_requests;

int log(const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    char buf[LOG_BUF_SIZE];
    /* write current time to buf */
    time_t now = time(nullptr);
    tm     tm_struct;
    localtime_s(&tm_struct, &now);
    strftime(buf, LOG_BUF_SIZE, "%X ", &tm_struct);
    const auto len = strlen(buf);
    /* write formatted log text to buf */
    vsnprintf(buf + len, LOG_BUF_SIZE - len, fmt, vl);
    va_end(vl);
    if (log_lines.size() == LOG_MAX_LINES) {
        log_lines.pop_front();
    }
    log_lines.push_back(buf);

    // if (str_match_s(buf + len, "Parsing error")){
    //     throw nullptr;
    // }
    ++scroll_requests;
    return 0;
}

void log_clear()
{
    log_lines.clear();
}

void log_append(const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    char buf[LOG_BUF_SIZE];
    vsnprintf(buf, LOG_BUF_SIZE, fmt, vl);
    log_lines.back().append(buf);
    va_end(vl);
}

std::deque<std::string> get_logs()
{
    return log_lines;
}
