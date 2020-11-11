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
#include "reg.h"
#include "log.h"

bool reg_get_value(LPCSTR fullkey, LPCSTR value_name, DATA data, DWORD dwFlags, LPDWORD out_type, LPDWORD out_data_size)
{
    HKEY    hkey;
    LSTATUS status;
    DWORD   type;
    LPDWORD p_type = out_type ? out_type : &type;
    if (!reg_exist_value(fullkey, value_name, &hkey)){
        log("Warning: reg value %s does not exist in \"%s\"", value_name, full_key);
        return false;
    }
    status = RegGetValueA(hkey, "", value_name, dwFlags, p_type, data, out_data_size);
    RegCloseKey(hkey);
    log("reg read value %s from \"%s\" (%lu bytes %s)", value_name, fullkey, *out_data_size, reg_type_name.at(*p_type));
    return status == ERROR_SUCCESS;
}

bool reg_set_value(LPCSTR fullkey, LPCSTR value_name, CDATA value, DWORD value_size, DWORD type)
{
    HKEY    hkey;
    LSTATUS status;
    if (!reg_create_key(fullkey, &hkey)){
        log("Error: reg failed to open key \"%s\"", fullkey, value);
        return false;
    }
    status = RegSetValueExA(hkey, value_name, NULL, type, static_cast<const BYTE*>(value), value_size);
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
            log("Warning: reg unsupported reg type 0x%x.", type);
        }
        log("reg set value (%s) %s to '%s' from \"%s\"", reg_type_name.at(type), value_name, buf, fullkey);
        return true;
    }
    log("Error %d: reg failed to set value '%s' from \"%s\"", status, value_name, fullkey);
    return false;
}

bool reg_create_key(LPCSTR fullkey, PHKEY out_hkey)
{
    HKEY    hkey = hkey_from_bytes(*(int*)fullkey), hkey_create;
    LSTATUS status;
    PHKEY   p_hkey = out_hkey ? out_hkey : &hkey_create;
    if (hkey == nullptr){
        log("Error: reg unrecognized HKEY '0x%x'", *(int*)fullkey);
        return false;
    }
    if (reg_exist_key(fullkey, p_hkey)){
        if (out_hkey == nullptr){
            RegCloseKey(*p_hkey);
        }
        log("Warning: reg attempt to create an existing sub key \"%s\"", fullkey);
        return true;
    }
    if ((status = RegCreateKeyA(hkey, fullkey + 5, p_hkey)) == ERROR_SUCCESS){
        log("reg created sub key \"%s\"", fullkey);
    }
    else{
        log("Error %d: reg failed to create sub key \"%s\"", status, fullkey);
    }
    if (out_hkey == nullptr){
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
        log("Warning: reg attempt to delete key \"%s\" which is not exist", fullkey);
        return true;
    }
    if ((status = SHDeleteKeyA(hkey, "")) == ERROR_SUCCESS){
        // delete recursively
        log("reg deleted key \"%s\"", fullkey);
    }
    else{
        log("Error %d: reg failed to delete key \"%s\"", status, fullkey);
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
        log("Warning: reg attempt to delete value '%s' in \"%s\" which is not exist", value_name, fullkey);
        return true;
    }
    if ((status = RegDeleteValueA(hkey, value_name)) == ERROR_SUCCESS){
        log("reg deleted value '%s' in \"%s\"", value_name, fullkey);
    }
    else{
        log("Error %d: reg failed to delete value '%s' in \"%s\"", status, value_name, fullkey);
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
        log("Error: reg unrecognized HKEY '0x%x'", *(int*)fullkey);
        return false;
    }
    status = RegOpenKeyExA(hkey, fullkey + 5, 0, KEY_ALL_ACCESS, p_hkey);
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
