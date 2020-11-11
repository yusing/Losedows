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

#include <cstdio>
#include <shlwapi.h>

#ifndef REG_SIMULATE_MODE
#include "error.h"
#include "reg.h"
#include "log.h"

bool reg_get_value(LPCSTR fullkey, LPCSTR value_name, DATA data, DWORD dwFlags, LPDWORD out_type, LPDWORD out_data_size)
{
    HKEY    hkey;
    DWORD   type;
    LPDWORD p_type = out_type ? out_type : &type;
    if (!reg_exist_value(fullkey, value_name, &hkey)){
        log("Registry Warning: value %s does not exist in key \"%s\"", value_name, fullkey);
        return false;
    }
    LSTATUS status = RegGetValueA(hkey, "", value_name, dwFlags, p_type, data, out_data_size);
    SetLastError(status);
    if (status == ERROR_SUCCESS){
        RegCloseKey(hkey);
        log("Registry: read value (type: %s) %s from \"%s\" (%lu bytes)", reg_type_name.at(*p_type), value_name,
            fullkey, *out_data_size);
        return true;
    }
    log("Registry Error: failed to read '%s' from key \"%s\", reason: \"%s\"", value_name, fullkey,
        last_error_string().c_str());
    return false;
}

bool reg_set_value(LPCSTR fullkey, LPCSTR value_name, CDATA value, DWORD value_size, DWORD type)
{
    HKEY    hkey;
    LSTATUS status;
    if (!reg_create_key(fullkey, &hkey)){
        log("Registry Error: failed to open key \"%s\", reason: \"%s\"", fullkey, value, last_error_string().c_str());
        return false;
    }
    status = RegSetValueExA(hkey, value_name, NULL, type, static_cast<const BYTE*>(value), value_size);
    SetLastError(status);
    RegCloseKey(hkey);
    if (ERROR_SUCCESS == status){
        char buf[STRBUF_SIZE]{'\0'};
        if (type == REG_SZ || type == REG_MULTI_SZ || type == REG_NONE){
            snprintf(buf, STRBUF_SIZE, "%s", static_cast<const char*>(value));
        }
        else if (type == REG_DWORD){
            snprintf(buf, STRBUF_SIZE, "%lx", *(DWORD*)value);
        }
        else{
            log("Registry Warning: unsupported registry type 0x%x", type);
        }
        log("Registry: set value (type: %s) %s to '%s' from \"%s\"", reg_type_name.at(type), value_name, buf, fullkey);
        return true;
    }
    log("Registry Error: failed to set value '%s' from \"%s\", reason: \"%s\"", value_name, fullkey,
        last_error_string().c_str());
    return false;
}

bool reg_create_key(LPCSTR fullkey, PHKEY out_hkey)
{
    HKEY    hkey = hkey_from_bytes(*(int*)fullkey), hkey_create;
    LSTATUS status;
    PHKEY   p_hkey = out_hkey ? out_hkey : &hkey_create;
    if (hkey == nullptr){
        log("Registry Error: unrecognized HKEY");
        return false;
    }
    if (reg_exist_key(fullkey, p_hkey)){
        if (out_hkey == nullptr){
            RegCloseKey(*p_hkey);
        }
        log("Registry Warning: attempt to create an existing sub key \"%s\"", fullkey);
        return true;
    }
    status = RegCreateKeyA(hkey, fullkey + 5, p_hkey);
    SetLastError(status);
    if (status == ERROR_SUCCESS){
        log("Registry: created sub key \"%s\"", fullkey);
    }
    else{
        log("Registry Error: failed to create sub key \"%s\", reason: \"%s\"", fullkey, last_error_string().c_str());
    }
    if (out_hkey == nullptr){
        // no key out, close the key
        RegCloseKey(*p_hkey);
    }
    return status == ERROR_SUCCESS;
}

bool reg_delete_key(LPCSTR fullkey)
{
    HKEY    hkey;
    LSTATUS status;
    if (!reg_exist_key(fullkey, &hkey)){
        // key does not exist
        log("Registry Warning: attempt to delete key \"%s\" which is not exist", fullkey);
        return true;
    }
    status = SHDeleteKeyA(hkey, "");
    SetLastError(status);
    if (status == ERROR_SUCCESS){
        // delete recursively
        log("Registry: deleted key \"%s\"", fullkey);
    }
    else{
        log("Registry Error: failed to delete key \"%s\", reason: \"%s\"", fullkey, last_error_string().c_str());
    }
    RegCloseKey(hkey);
    return status == ERROR_SUCCESS;
}

bool reg_delete_value(LPCSTR fullkey, LPCSTR value_name)
{
    HKEY    hkey;
    LSTATUS status;
    if (!reg_exist_value(fullkey, value_name, &hkey)){
        // value does not exist
        log("Registry Warning: attempt to delete value '%s' in \"%s\" which is not exist", value_name, fullkey);
        return true;
    }
    status = RegDeleteValueA(hkey, value_name);
    SetLastError(status);
    if (status == ERROR_SUCCESS){
        log("Registry: deleted value '%s' in \"%s\"", value_name, fullkey);
    }
    else{
        log("Registry Error: failed to delete value '%s' in \"%s\", reason: \"%s\"", value_name, fullkey,
            last_error_string().c_str());
    }
    RegCloseKey(hkey);
    return status == ERROR_SUCCESS;
}

bool reg_exist_key(LPCSTR fullkey, PHKEY out_hkey)
{
    HKEY    hkey   = hkey_from_bytes(*(int*)fullkey), hkey_open;
    PHKEY   p_hkey = out_hkey ? out_hkey : &hkey_open;
    LSTATUS status;
    if (hkey == nullptr){
        log("Registry Error: unrecognized HKEY");
        return false;
    }
    status = RegOpenKeyExA(hkey, fullkey + 5, 0, KEY_ALL_ACCESS, p_hkey);
    SetLastError(status);
    if (out_hkey == nullptr){
        RegCloseKey(*p_hkey);
    }
    return status == ERROR_SUCCESS;
}

bool reg_exist_value(LPCSTR fullkey, LPCSTR value_name, PHKEY out_hkey)
{
    HKEY    hkey;
    PHKEY   p_hkey = out_hkey ? out_hkey : &hkey;
    LSTATUS status;
    if (!reg_exist_key(fullkey, p_hkey)){
        // key does not exist
        return false;
    }
    status = RegGetValueA(*p_hkey, "", value_name, RRF_RT_ANY, nullptr, nullptr, nullptr);
    SetLastError(status);
    if (out_hkey == nullptr){
        RegCloseKey(*p_hkey);
    }
    return status == ERROR_SUCCESS;
}

HKEY hkey_from_bytes(int rtl_bytes)
{
    // NOTE: Windows uses Little endian
    if (rtl_bytes == 0x55434b48){
        // UCKH
        return HKEY_CURRENT_USER;
    }
    if (rtl_bytes == 0x4d4c4b48){
        // MLKH
        return HKEY_LOCAL_MACHINE;
    }
    if (rtl_bytes == 0x52434b48){
        // RCKH
        return HKEY_CLASSES_ROOT;
    }
    return nullptr;
}
#endif
