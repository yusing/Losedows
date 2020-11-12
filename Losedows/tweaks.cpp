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

#include "tweaks.h"
#include "log.h"
#include <cassert>
#include <stdexcept>
#include <string>
#include <Windows.h>

using namespace std::string_literals;

DWORD Tweak::data_size() const
{
    assert(type != DeleteKey);
    if ((type & String) != 0){
        return static_cast<DWORD>(strlen(str_value()));
    }
    return sizeof(DWORD);
}

DWORD Tweak::data_size_max() const
{
    if ((type & String) != 0){
        return STRBUF_SIZE;
    }
    return sizeof(DWORD);
}

DWORD Tweak::reg_type() const
{
    if ((type & String) != 0){
        return REG_SZ;
    }
    if ((type & Bool) != 0 || (type & Dword) != 0){
        return REG_DWORD;
    }
    throw nullptr;
}

DWORD& Tweak::enabled()
{
    if (data == nullptr){
        // sub-tweak or toggle type
        return dw_enabled;
    }
    return *static_cast<DWORD*>(data);
}

DWORD& Tweak::dw_value() const
{
    assert(type == Dword || type == Bool);
    if (data == nullptr){
        // sub-tweak or CreateValue
        return *static_cast<DWORD*>(value_optimal);
    }
    return *static_cast<DWORD*>(data);
}

LPSTR Tweak::str_value() const
{
    if ((type & String) != 0)
        return static_cast<LPSTR>(data ? data : value_optimal ? value_optimal : value_default);
    throw nullptr;
}

bool Tweak::can_toggle() const
{
    return type != String && type != Dword;
}

bool Tweak::is_user_input() const
{
    return
            (type == String || type == Dword) &&
            (value_optimal == nullptr);
}

void Tweak::dispose() const
{
    delete[] title;
    delete[] reg_key;
    delete[] reg_value;
    // NOTE: they are initialized with new char[] or new DWORD[] so it is defined behavior
    delete[] value_default;
    if (data != value_optimal)
        delete[] data;
    delete[] value_optimal;

    for (auto* st : sub_tweaks){
        // NOTE: st->reg_key inherits main tweaks
        delete[] st->value_default;
        delete[] st->value_optimal;
        delete st;
    }
}

void Tweak::toggle()
{
    if (can_toggle()){
        enabled() = !enabled();
    }
}

bool Tweak::execute(LPSTR override_key)
{
    char* full_key;
    bool  result;

    if (override_key != nullptr){
        full_key = override_key;
    }
    else{
        full_key = reg_key;
    }

    if (type == Bool){
        toggle();
        result = reg_set_value(full_key, reg_value, &enabled(), data_size(),
                               reg_type());
    }
    else if (type == Dword){
        result = reg_set_value(full_key, reg_value, &dw_value(), data_size(),
                               reg_type());
    }
    else if (type == String){
        result = reg_set_value(full_key, reg_value, str_value(), data_size(),
                               reg_type());
    }
    else if ((type & CreateValue) != 0){
        if (enabled()){
            result = reg_delete_value(full_key, reg_value);
        }
        else{
            result = reg_set_value(full_key, reg_value, value_optimal, data_size(),
                                   reg_type());
        }
        if (result == true){
            toggle();
        }
    }
    else if ((type & DeleteValue) != 0){
        if (enabled()){
            result = reg_set_value(full_key, reg_value, value_default, data_size(), reg_type());
        }
        else{
            result = reg_delete_value(full_key, reg_value);
        }
        if (result == true){
            toggle();
        }
    }
    else if (type == CreateSubKey){
        if (enabled()){
            // sub key exists
            const auto key = full_key + "\\"s + reg_value;
            result         = reg_delete_key(key.c_str());
            if (result == true){
                toggle();
            }
            return result; // stuff under deleted key will be also deleted, no need to process sub-tweaks
        }
        // append "\\" and sub key
        auto* dst = full_key + strlen(full_key);
        *dst++    = '\\';
        auto* src = reg_value;
        while (*src){
            *dst++ = *src++;
        }
        *dst = '\0';

        result = reg_create_key(full_key, nullptr);
        if (result == true){
            toggle();
        }
    }
    else if ((type & DeleteKey) != 0){
        if (enabled()){
            // key does not exist
            result = reg_create_key(full_key, nullptr);
        }
        else{
            result = reg_delete_key(full_key);
        }
        if (result == true){
            toggle();
        }
        full_key = nullptr; // do not override!
    }
    else{
        throw std::runtime_error("Unknown type");
    }

    if (result == true){
        /*
         * Note: main tweak overrides sub-tweak's enabled() status in order to ensure all tweaks behave the same when toggled
         */
        for (auto* st : sub_tweaks){
            st->enabled() = !this->enabled();
            result        = result && st->execute(full_key);
        }
    }

    return result;
}

bool Tweak::restore_default()
{
    assert(value_default != nullptr);
    assert(data != nullptr);
    bool result = reg_set_value(reg_key, reg_value, value_default,
                                data_size_max(), reg_type());
    memcpy(data, value_default, data_size_max());
    for (auto* st : sub_tweaks){
        result = result && st->restore_default();
    }
    return result;
}

constexpr DWORD DWFLAGS_DWORD  = RRF_RT_REG_DWORD;
constexpr DWORD DWFLAGS_STRING = (
    RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_RT_REG_NONE | RRF_NOEXPAND |
    RRF_ZEROONFAILURE);

void init_tweak_list(std::vector<Tweak>& tweaks)
{
    DWORD dwFlags;
    for (auto& _ : tweaks){
        assert(_.type != Invalid);
        assert(_.reg_key != nullptr);
        if (_.type == Bool){
            assert(_.reg_value != nullptr);
            assert(_.value_default == nullptr);
            assert(_.value_optimal == nullptr);
            assert(_.data == nullptr);
            dwFlags        = DWFLAGS_DWORD;
            DWORD max_size = _.data_size_max();
            reg_get_value(_.reg_key, _.reg_value, &_.enabled(), dwFlags, nullptr,
                          &max_size);
        }
        else if (_.type == Dword){
            assert(_.reg_value != nullptr);
            assert(_.data != nullptr || _.value_optimal != nullptr);
            dwFlags = DWFLAGS_DWORD;
        }
        else if (_.type == String){
            assert(_.reg_value != nullptr);
            assert(_.data != nullptr || _.value_optimal != nullptr);
            dwFlags = DWFLAGS_STRING;
        }
        else if (_.type == CreateSubKey){
            assert(_.reg_value != nullptr);
            assert(_.value_default == nullptr);
            assert(_.value_optimal == nullptr);
            assert(_.data == nullptr);
            const auto key = _.reg_key + "\\"s + _.reg_value;
            _.enabled()    = reg_exist_key(key.c_str(), nullptr);
        }
        else if (_.type & DeleteKey){
            assert(_.reg_value == nullptr);
            assert(_.value_default == nullptr);
            assert(_.value_optimal == nullptr);
            assert(_.data == nullptr);
            _.enabled() = !reg_exist_key(_.reg_key, nullptr);
        }
        else if ((_.type & CreateValue) != 0){
            assert(_.reg_value != nullptr);
            assert(_.value_default == nullptr);
            assert(_.value_optimal != nullptr);
            assert(_.data == nullptr);
            _.enabled() = reg_exist_value(_.reg_key, _.reg_value, nullptr);
        }
        else if ((_.type & DeleteValue) != 0){
            assert(_.reg_value != nullptr);
            assert(_.value_default != nullptr);
            assert(_.value_optimal == nullptr);
            assert(_.data == nullptr);
            _.enabled() = !reg_exist_value(_.reg_key, _.reg_value, nullptr);
        }
        else{
            throw nullptr;
        }
        if (_.is_user_input()){
            // if the value/key doesn't exist, use the default value
            DWORD max_size = _.data_size_max();
            if (!reg_get_value(_.reg_key, _.reg_value, _.data, dwFlags, nullptr,
                               &max_size)){
                if (_.value_default != nullptr){
                    memcpy(_.data, _.value_default, static_cast<size_t>(max_size));
                }
            }
        }
    }
}
