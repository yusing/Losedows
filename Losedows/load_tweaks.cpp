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

#define STRLEN(s) (sizeof(s)-1)
#define INC ++p;
#define CUR (*p)
#include "load_tweaks.h"


#include <cassert>
#include <deque>
#include <fstream>
#include <thread>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include "file.h"
#include "log.h"
#include "string_utils.h"
#include "tweaks.h"

std::unordered_map<std::string, std::vector<Tweak>*> tweaks_read;
std::vector<std::string>                             custom_tweaks;
static auto                                          LESS_EQ      = [](DWORD x, DWORD y) { return x <= y; };
static auto                                          LESS_THAN    = [](DWORD x, DWORD y) { return x < y; };
static auto                                          GREATER_EQ   = [](DWORD x, DWORD y) { return x >= y; };
static auto                                          GREATER_THAN = [](DWORD x, DWORD y) { return x > y; };
static auto                                          EQUAL_TO     = [](DWORD x, DWORD y) { return x == y; };

extern OSVERSIONINFOEX* get_os_info();



void clear_tweaks()
{
    for (auto& _ : tweaks_read) {
        // free memory of old content
        for (Tweak& entry : *_.second) {
            entry.dispose();
        }
        delete _.second;
    }
    tweaks_read.clear();
}

void add_custom_tweak(const std::string& file)
{
    custom_tweaks.push_back(file);   
}

void load_all_tweaks()
{
    clear_tweaks();
    const auto current_pack = current_path() + "\\current.tkpack";
    const auto custom_pack = current_path() + "\\custom.tkpack";
    if (file_exists(current_pack)){
        load_tweak_pack(current_pack);
    }
    else{
        log("Warning: current.tkpack not found");
    }
    if (file_exists(custom_pack)) {
        load_tweak_pack(custom_pack);
    }
    for (auto& file : custom_tweaks){
        load_tweak_from_file(file);
    }
    if (tweaks_read.size() == 0){
        log("No tweak loaded");
        return;
    }
    for (auto& _ : tweaks_read){
        init_tweak_list(*_.second);
    }
}

void load_tweak_from_file(const std::string& file)
{
    if (file_extension(file) == ".tkpack"){
        return load_tweak_pack(file);
    }
    std::fstream fs;
    if (!open_file(file, fs, IOS_IN)){
        log("File I/O Error: unable to open tweak file \"%s\" for read", file.c_str());
        return;
    }
    const std::string content((std::istreambuf_iterator<char>(fs)),
                              (std::istreambuf_iterator<char>()));
    const auto file_name = filename(file);
    if (is_invalid(file_name.c_str(), content.c_str())){ // do not handle invalid tweak
        log("Error found in \"%s\"", file_name.c_str());
        return;
    }
    log("file \"%s\" OK.", file_name.c_str());
    load_tweak(content.c_str());
}

// tweak pack structure: TweakPackVersion + tweak_file[0] + '\0' + tweak_file[1] + '\0' + ... + tweak_file[n] + '\0'
void load_tweak_pack(const std::string& file)
{
    std::fstream fs;
    if (!open_file(file, fs, IOS_IN)){
        log("File I/O Error: unable to open tweak pack \"%s\" for read",
            file.c_str());
        return;
    }
    const std::string content((std::istreambuf_iterator<char>(fs)),
                              (std::istreambuf_iterator<char>()));
    const char* p = content.c_str(), *p_end = p+content.size();
    TweakPackVersion version;
    memcpy(&version, p, sizeof TweakPackVersion);
    log("Tweak pack version %d.%d", version.major, version.minor);
    p += sizeof TweakPackVersion;
    do {
        p = load_tweak(p) + 1; // skip '\0'
    } while (p < p_end);
}

template <class ContainerOfString>
void pack_tweaks(const TweakPackVersion& version, ContainerOfString tweak_files)
{
    if (tweak_files.empty()){
        return;
    }
    const auto         out_file    = current_path() + "\\custom.tkpack";
    std::fstream fs_pack_file;
    std::fstream fs_tweak_file;
    if (!open_file(out_file, fs_pack_file, IOS_OUT)){
        log("File I/O Error: unable to open file \"%s\" for write", out_file.c_str());
        return;
    }
    fs_pack_file.write(static_cast<const char*>(static_cast<const void*>(&version)), sizeof version);
    for (auto& file : tweak_files){
        if (open_file(file, fs_tweak_file, IOS_IN)) {
            fs_pack_file << fs_tweak_file.rdbuf();
            fs_pack_file.put('\0'); // use '\0' as separator
            log("packed \"%s\"", file.c_str());
        } else {
            log("File I/O Error: unable to open file \"%s\" for read", file.c_str());
        }
        fs_tweak_file.close();
    }
    fs_pack_file.close();
    log("tweaks packed to \"%s\"", out_file.c_str());
}

template<> void pack_tweaks(const TweakPackVersion& version, const std::vector<std::string>& tweak_files)
{
    pack_tweaks(version, tweak_files);
}
template<> void pack_tweaks(const TweakPackVersion& version, const std::deque<std::string>& tweak_files)
{
    pack_tweaks(version, tweak_files);
}

const char* load_tweak(const char* tweak_data)
{
    auto& p = tweak_data; // alias
    char  category[STRBUF_SIZE];
    char* p_buf;

    // get category name
    p_buf = category;
    while (CUR != '\n'){
        *p_buf++ = *p++;
    }
    *p_buf = '\0';
    INC; // skip \n

    auto* p_tweak = new std::vector<Tweak>();
    auto& tweak   = *p_tweak;
    tweaks_read.insert({category, p_tweak});

    while (CUR != '\0'){
        while (CUR == '\n'){
            INC;
        }
        if (CUR == '#'){
            char      title_buf[STRBUF_SIZE]{'\0'};
            char*     title = title_buf;
            char      reg_key_buf[STRBUF_SIZE]{'\0'};
            char*     reg_key = reg_key_buf;
            char      reg_value[32];
            char      type_buf[16];
            char      value_opt[STRBUF_SIZE]{'\0'};
            char      value_default[STRBUF_SIZE]{'\0'};
            int       i = 0;
            Tweak*    info;
            TweakType tweak_type = 0;
            bool      hidden     = false;
            INC; // #
            /* get tweak title */
            p_buf = title_buf;
            while (CUR != '\n' && CUR != ';'){
                *p_buf++ = *p++;
            }
            *p_buf = '\0';
            /* check tweak required Windows version */
            if (*p++ == ';'){
                // skip ; or \n
                char   requirement[10];
                bool (*compare)(DWORD, DWORD);
                DWORD  version;
                p_buf = requirement;
                /* get build number */
                while (CUR != '\n'){
                    *p_buf++ = *p++;
                }
                *p_buf = '\0';
                INC // \n
                p_buf = requirement;
                if (p_buf[1] == '='){
                    if (*p_buf == '<'){
                        compare = LESS_EQ;
                    }
                    else{
                        // >
                        compare = GREATER_EQ;
                    }
                    p_buf += 2;
                }
                else{
                    if (*p_buf == '<'){
                        compare = LESS_THAN;
                    }
                    else if (*p_buf == '>'){
                        compare = GREATER_THAN;
                    }
                    else{
                        // =
                        compare = EQUAL_TO;
                    }
                    p_buf++;
                }
                OSVERSIONINFOEX* osInfo = get_os_info();
                version                 = strtoul(p_buf, nullptr, 10);
                if (!compare(osInfo->dwBuildNumber, version)){
                    hidden = true;
                    log("The tweak \"%s\" is hidden (OS build:%lu, required:%s)", title_buf, osInfo->dwBuildNumber,
                        requirement);
                }
                p_buf = nullptr; // reset it because it points to local array
            }
            i = 0;
            if (CUR == '-'){
                do{
                    INC;
                    p_buf = reg_key_buf;
                    while (CUR != '\n'){
                        // the whole line is the registry key
                        *p_buf++ = *p++;
                    }
                    *p_buf = '\0';
                    INC; // \n
                    if (!hidden){
                        reg_key = new char[STRBUF_SIZE];
                        m_strcpy(reg_key, reg_key_buf);
                        if (i == 0){
                            title = new char[STRBUF_SIZE];
                            m_strcpy(title, title_buf);
                            tweak.emplace_back(Tweak{DeleteKey, title, reg_key});
                        }
                        else{
                            tweak.back().sub_tweaks.emplace_back(
                                new Tweak{DeleteKey, nullptr, reg_key});
                        }
                    }
                    ++i;
                    while (CUR == '\n'){
                        // end of tweak
                        INC;
                    }
                }
                while (CUR == '-');
            }
            if (CUR == '#'){
                continue;
            }
            p_buf = reg_key = new char[STRBUF_SIZE];
            while (CUR != '\n'){
                *p_buf++ = *p++;
            }
            *p_buf = '\0';
            INC;
            do{
                *value_opt     = 0;
                *value_default = 0;
                p_buf          = type_buf;
                while (CUR != ' ' && CUR != '\n'){
                    *p_buf++ = *p++;
                }
                *p_buf = '\0';
                INC;
                // NOTE: the tweak file is already verified
                if (*type_buf == 'S'){
                    tweak_type = String;
                }
                else if (*type_buf == 'B'){
                    tweak_type = Bool;
                }
                else if (*type_buf == 'D' && type_buf[1] == 'w'){
                    tweak_type = Dword;
                }
                else if (*type_buf == 'D' && type_buf[1] == 'e'){
                    tweak_type = DeleteValue;
                }
                else if (str_match(type_buf, "CreateValue")){
                    tweak_type = CreateValue;
                }
                else{
                    tweak_type = CreateSubKey;
                }

                p_buf = reg_value;
                // These types does not require default value and optimal value
                if (tweak_type == Bool || tweak_type == CreateSubKey){
                    while (CUR != '\n' && CUR != '\0'){
                        *p_buf++ = *p++;
                    }
                    *p_buf = '\0';
                }
                else{
                    while (CUR != '=' && CUR != ' '){
                        *p_buf++ = *p++;
                    }
                    *p_buf = '\0';
                    if (*p++ == ' '){
                        p += STRLEN("Default=");
                        p_buf = value_default;
                        if (*p == '"'){
                            tweak_type |= String;
                            p++;
                            while (CUR != '\0' && CUR != '"'){
                                if (CUR == '\\' && *(p + 1) == '"'){
                                    *p_buf++ = '"';
                                    p += 2;
                                }
                                else{
                                    *p_buf++ = *p++;
                                }
                            }
                            *p_buf = '\0';
                            INC; // "
                        }
                        else{
                            tweak_type |= Dword;
                            while (CUR != '\0' && (CUR != ' ' && CUR != '\n')){
                                *p_buf++ = *p++;
                            }
                            *p_buf = '\0';
                        }
                    }
                    else{
                        // get optimal value
                        auto arg = 0;
                        p_buf    = value_opt;
                        if (CUR == '"'){
                            // optimal value is string
                            tweak_type |= String;
                            do{
                                INC; // leading "
                                if (arg == 1){
                                    p_buf = value_default;
                                }
                                while (CUR != '\0' && CUR != '"'){
                                    if (CUR == '\\' && *(p + 1) == '"'){
                                        // escaped "
                                        *p_buf++ = '"';
                                        p += 2;
                                    }
                                    else{
                                        *p_buf++ = *p++;
                                    }
                                }
                                *p_buf = '\0';
                                INC; // trailing "
                                if (arg == 0 && *p == ' '){
                                    p += STRLEN(" Default=");
                                }
                                arg++;
                            }
                            while (CUR == '"');
                        }
                        else{
                            // optimal value is bool or dword
                            tweak_type |= Dword;
                            do{
                                if (arg == 1){
                                    p_buf = value_default;
                                }
                                while (CUR != '\0' && CUR != ' ' && CUR != '\n'){
                                    *p_buf++ = *p++;
                                }
                                *p_buf = '\0';
                                if (arg == 0 && *p == 'D'){
                                    p += STRLEN("Default=");
                                }
                                arg++;
                            }
                            while (CUR != '\0' && CUR != '\n');
                        }
                    }
                }
                if (!hidden){
                    if (i == 0){
                        title = new char[STRBUF_SIZE];
                        m_strcpy(title, title_buf);
                        info        = &tweak.emplace_back();
                        info->title = title;
                    }
                    else{
                        info = new Tweak();
                        tweak.back().sub_tweaks.emplace_back(info);
                    }
                    info->type = tweak_type;
                    // NOTE: main tweak and sub-tweaks share same pointer
                    info->reg_key = reg_key;

                    if (*reg_value){
                        info->reg_value = new char[STRBUF_SIZE];
                        m_strcpy(info->reg_value, reg_value);
                    }
                    if (*value_opt){
                        if ((tweak_type & String) != 0){
                            info->value_optimal = new char[STRBUF_SIZE];
                            m_strcpy(static_cast<char*>(info->value_optimal), value_opt);
                        }
                        else{
                            auto dw             = new DWORD[1];
                            info->value_optimal = dw;
                            if (*value_opt == 't'){
                                *dw = 1;
                            }
                            else if (*value_opt == 'f'){
                                *dw = 0;
                            }
                            else{
                                *dw = strtoul(value_opt, nullptr, 16);
                            }
                        }
                    }
                    if (*value_default){
                        if ((tweak_type & String) != 0){
                            info->value_default = new char[STRBUF_SIZE];
                            m_strcpy(static_cast<char*>(info->value_default), value_default);
                        }
                        else{
                            auto dw             = new DWORD[1];
                            info->value_default = dw;
                            if (*value_default == 't'){
                                *dw = 1;
                            }
                            else if (*value_default == 'f'){
                                *dw = 0;
                            }
                            else{
                                *dw = strtoul(value_default, nullptr, 16);
                            }
                        }
                    }
                    if (i == 0){
                        if (*value_default != '\0'){
                            // no default value means no user input. Therefore no need to initialize info->data;
                            // if ((tweak_type & CreateValue) != 0 || (tweak_type & DeleteKey) != 0 || tweak_type == Bool){
                            //     throw std::runtime_error("unexpected default value found");
                            // }
                            if (tweak_type == String){
                                info->data = new char[STRBUF_SIZE]{'\0'};
                            }
                            else if (tweak_type == Dword){
                                info->data = new DWORD[1]{0};
                            }
                            else{
                                // DeleteValue

                            }
                        }
                    }
                }

                ++i;
                while (CUR == '\n'){
                    INC;
                }
            }
            while (CUR != '#' && CUR != '\0');
        }
    }
    return tweak_data;
}


constexpr bool is_hex_digit(char c) // inlined
{
    return c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F' || c >= '0' && c <= '9';
}

/**
 * \brief check the type of value in tweak file return at ' ', '\0' or '\n'
 * \param n_th n-th tweak
 * \param p pointer to value
 * \param out_has_error pointer to has_error
 * \param out_type pointer to tweak_type
 */
void check_value(int n_th, const char*& p, bool* out_has_error, TweakType* out_type)
{
    auto& has_error = *out_has_error;
    if (*p == '\0' || *p == '\n'){
        has_error = true;
        log("Parsing error: expect a value @ tweak #%d", n_th);
        return;
    }
    if (str_match_s(p, "true") || str_match_s(p, "false")){
        p += *p == 't' ? STRLEN("true") : STRLEN("false");
        *out_type = Bool;
        return;
    }
    if (*p == '"'){
        *out_type = String;
        ++p; // skip leading "
        while (*p != '"' && *p != '\n' && *p != '\0'){
            if (*p == '\\' && p[1] == '"'){
                // escaped "
                p += 2;
            }
            else{
                ++p;
            }
        }
        if (*p != '"'){
            has_error = true;
            log("Parsing error: expect double quote at the end of string @ tweak #%d", n_th);
        }
        ++p; // trailing "
    }
    else{
        *out_type = Dword;
        while (*p != '\n' && *p != ' ' && *p != '\0'){
            if (!is_hex_digit(*p)){
                has_error = true;
                log(
                    "Parsing error: unexpected character '%c' in default value @ tweak #%d\nExpecting hex, boolean value or string value",
                    *p, n_th);
            }
            ++p;
        }
    }
}

/**
 * \brief must return '\n' or '\0'
 * \param n_th the n-th tweak
 * \param p pointer to default value
 * \param check check type
 * \param out_has_error pointer to has_error
 * \param out_type pointer to tweak_type
 * \return whether default value is found
 */
void has_valid_default_value(int n_th, const char*& p, CheckType check, bool* out_has_error, TweakType* out_type)
{
    auto& has_error = *out_has_error;
    if (*p == '\0' || *p == '\n'){
        if (check == CheckType::Required){
            log("Parsing error: expect default value (arg #3) @ tweak #%d", n_th);
            has_error = true;
        }
        return;
    }
    if (*p == ' '){
        ++p;
    }
    if (str_match_s(p, "Default=")){
        // default value found
        p += STRLEN("Default=");
        if (check == CheckType::Prohibit){
            log("Parsing error: unexpected default value (arg #3) @ tweak #%d", n_th);
            has_error = true;
        }
    }
    else{
        // returned
        if (check == CheckType::Required){
            log("Parsing error: expect default value (arg #3) @ tweak #%d", n_th);
            while (*p != '\n' && *p != '\0'){ ++p; } // skip default value name
            has_error = true;
            return;
        }
    }
    check_value(n_th, p, &has_error, out_type);
    if (*p != '\n' && *p != '\0'){
        log("Parsing error: expect new line after default value (arg #3), got '%c' @ tweak #%d", *p, n_th);
        has_error = true;
        do{
            ++p;
        }
        while (*p != '\n' && *p != '\0');
    }
    assert(*p == '\n' || *p == '\0');
}

bool has_valid_optimal_value(int n_th, const char*& p, CheckType check, bool* out_has_err, TweakType* out_type)
{
    auto& has_error = *out_has_err;
    while (*p != ' ' && *p != '\n' && *p != '\0' && *p != '='){ ++p; } // skip registry value (arg 1)
    if (*p == '\n' || *p == '\0'){
        if (check == CheckType::Required){
            log("Parsing error: expect optimal value (arg #2) @ tweak #%d", n_th);
            has_error = true;
        }
        return false;
    }
    if (*p == '='){
        // have optimal value
        ++p;
        if (check == CheckType::Prohibit){
            log("Parsing error: unexpected optimal value (arg #2) @ tweak #%d", n_th);
            has_error = true;
        }
        check_value(n_th, p, &has_error, out_type);
        return true;
    }
    // no optimal value
    ++p; // skip space
    if (check == CheckType::Required){
        log("Parsing error: expect optimal value (arg #2) @ tweak #%d", n_th);
    }
    return false;
}

bool is_invalid(const std::string& file_name, const char* tweak_data)
{
    if (*tweak_data == '\0'){
        return true;
    }
    auto* p         = tweak_data;
    auto* fname     = file_name.c_str();
    auto  n_th      = 0;
    auto  has_error = false;
    log("validating file tweak file \"%s\"", fname);
    if (*p++ == '\0'){
        log("Parsing error: expected category name in the first line");
        has_error = true;
    }
    while (*p != '\n' && *p != '\0'){ ++p; } // skip first line (category name)
    while (*p == '\n'){ ++p; }               // skip empty lines
    while (*p != '\0'){
        TweakType type_parsed;
        if (*p++ != '#'){
            log("Parsing error: expected tweak title @ tweak #%d", n_th);
            while (*p != '\n' && *p != '\0'){ ++p; } // skip this line cause of error
            has_error = true;
        }
        else{
            ++n_th;
            while (*p != '\n' && *p != '\0' && *p != ';'){ ++p; } // skip tweak title
        }
        if (*p == ';'){
            // found target build number
            ++p;
            if (*p == '\0'){
                log("Parsing error: unexpected end of file @ tweak #%d", n_th);
                return false;
            }
            if (!(*p == '<' || *p == '=' || *p == '>')){
                has_error = true;
                log("Parsing error: unexpected operator '%c' (expect '<', '=' or '>') @ tweak #%d", *p, n_th);
            }
            else{
                if ((*p == '<' || *p == '>') && *(p + 1) == '='){
                    p += 2;
                }
                else{
                    ++p;
                }
            }

            while (*p != '\n' && *p != '\0'){
                if (!(*p >= '0' && *p <= '9')){
                    has_error = true;
                    log("Parsing error: unexpected char '%c' for build number @ tweak #%d", *p, n_th);
                }
                ++p;
            }
        }
        if (*p == '\0'){
            log("Parsing error: unexpected end of file @ tweak #%d", n_th);
            return false;
        }
        ++p; // skip \n
        if (*p == '-'){
            // DeleteKey type
            do{
                ++p; // skip -
                if (!(str_match_s(p, "HKLM") || str_match_s(p, "HKCU") || str_match_s(p, "HKCR"))){
                    goto INVALID_KEY;
                }
                while (*p != '\n' && *p != '\0'){ ++p; } // skip registry key
                if (*p == '\0'){
                    return has_error;
                }
                ++p; // skip \n
            }
            while (*p == '-');
            while (*p == '\n'){
                // skip empty lines
                ++p;
            }
            if (*p == '#'){
                // end of tweak
                continue;
            }
            if (*p == '\0'){
                return has_error;
            }
        }

        if (str_match_s(p, "HKLM") || str_match_s(p, "HKCU") || str_match_s(p, "HKCR")){
            // Only allow these 3 registry keys
            while (*p != '\n' && *p != '\0'){ ++p; } // skip registry key
            if (*p++ == '\0'){
                log("Parsing error: unexpected end of file @ tweak #%d", n_th);
                return true;
            }
            while (*p != '#' && *p != '\0'){
                // match until next tweak
                type_parsed = Invalid; // reset every loop
                if (*p == '\0'){
                    // end of file in title line
                    return has_error;
                }
                auto is_str   = str_match_s(p, "String");
                auto is_dword = str_match_s(p, "Dword");

                if (is_str || is_dword){
                    // Require either optimal value or default value, but not both
                    p += *p == 'S' ? STRLEN("String") : STRLEN("Dword"); // skip tweak type
                    if (*p++ == '\0'){
                        log("Parsing error: unexpected end of file @ tweak #%d", n_th);
                      return true;
                    }
                    if (has_valid_optimal_value(n_th, p, CheckType::Optional, &has_error, &type_parsed)){
                        // has optimal value
                        if (is_str && type_parsed != String || is_dword && type_parsed != Dword){
                            log("Parsing error: type mismatch (require %s) @ tweak #%d", is_str ? "String" : "Dword",
                                n_th);
                            has_error = true;
                        }
                        if (*p != '\n' && *p != '\0'){
                            TweakType _; // type unused because default value is prohibited
                            has_valid_default_value(n_th, p, CheckType::Prohibit, &has_error, &_);
                        }
                    }
                    else{
                        // does not have optimal value, so default value is required
                        if (*p == '\n'){
                            log("Parsing error: string type require 1 argument @ tweak #%d", n_th);
                            has_error = true;
                        }
                        else{
                            TweakType default_value_type = Invalid;
                            has_valid_default_value(n_th, p, CheckType::Required, &has_error, &default_value_type);
                            if (is_str && default_value_type != String || is_dword && default_value_type != Dword){
                                log("Parsing error: type mismatch (require %s) @ tweak #%d",
                                    is_str ? "String" : "Dword", n_th);
                                has_error = true;
                            }
                        }
                    }
                }
                else if (str_match_s(p, "CreateSubKey")){
                    p += STRLEN("CreateSubKey");
                    if (*p != ' ' || *p == '\0' || *p == '\n'){
                        log("Parsing error: expect key name for CreateSubKey @ tweak #%d", n_th);
                        return true;
                    }
                    ++p;                                     // skip space
                    while (*p != '\n' && *p != '\0'){ ++p; } // the whole line is the sub key, skip it
                }
                else if (str_match_s(p, "Bool") || str_match_s(p, "DeleteKey")){
                    p += *p == 'B' ? STRLEN("Bool") : STRLEN("DeleteKey"); // skip tweak type
                    if (*p != ' ' || *p == '\0' || *p == '\n'){
                        log("Parsing error: expect a value @ tweak #%d", n_th);
                        return true;
                    }
                    ++p;                                                  // skip space
                    while (*p != ' ' && *p != '\n' && *p != '\0'){ ++p; } // skip registry value (arg #1)
                    // Require only registry value (arg #1)
                    if (*p == '\0'){
                        // end of file
                        return has_error;
                    }
                    if (*p == ' '){
                        log("Parsing error: too many argument @ tweak #%d", n_th);
                        has_error = true;
                        while (*p != '\n' && *p != '\0'){ ++p; } // skip this line
                    }
                    // leave the new line as is
                }
                else if (str_match_s(p, "CreateValue")){
                    // Require only optimal value (arg #2)
                    p += STRLEN("CreateValue");
                    if (*p == '\0' || *p == '\n' || *p != ' '){
                        log("Parsing error: expect a value @ tweak #%d", n_th);
                      return true;
                    }
                    ++p; // skip space
                    has_valid_optimal_value(n_th, p, CheckType::Required, &has_error, &type_parsed);
                    if (*p != '\n'){
                        has_valid_default_value(n_th, p, CheckType::Prohibit, &has_error, &type_parsed);
                    }
                }
                else if (str_match_s(p, "DeleteValue")){
                    // Require only default value (arg #3)
                    p += STRLEN("DeleteValue");
                    if (*p == '\0' || *p == '\n' || *p != ' '){
                        log("Parsing error: expect a value @ tweak #%d", n_th);
                      return true;
                    }
                    ++p; // skip space
                    has_valid_optimal_value(n_th, p, CheckType::Prohibit, &has_error, &type_parsed);
                    if (*p == '\n'){
                        log("Parsing error: unexpected next line @ tweak #%d", n_th);
                        has_error = true;
                    }
                    else{
                        has_valid_default_value(n_th, p, CheckType::Required, &has_error, &type_parsed);
                    }
                }
                else{
                    // Unknown type
                    has_error = true;
                    log("Parsing error: unknown type found in tweak #%d", n_th);
                    while (*p != '\n' && *p != '\0'){ ++p; } // skip this line cause of Parsing error
                }
                while (*p == '\n'){
                    // skip empty lines
                    ++p;
                }
            }
        }
        else{
            // if not match HKLM, HKCU or HKCR key
        INVALID_KEY:
            char got[5];
            memcpy(got, p, 4);
            got[4] = '\0';
            log(
                "Parsing error: invalid registry key in tweak file \"%s\": (Unknown \"%s\"): Only accept HKLM, HKCU and HKCR",
                fname, got);
            has_error = true;
            while (*p != '\n' && *p != '\0'){ ++p; } // skip this line cause of Parsing error
        }
    }
    return has_error;
}
