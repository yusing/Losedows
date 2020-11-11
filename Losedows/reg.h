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
#define STRBUF_SIZE 256
#define DATA void*
#define CDATA const void*
#include <Windows.h>
#include <unordered_map>

static std::unordered_map<DWORD, LPCSTR> reg_type_name{
    {REG_SZ, "REG_SZ"},
    {REG_DWORD, "REG_DWORD"}
};
#ifdef REG_SIMULATE_MODE
#   define reg_get_value(k,vname,d,flags,t,p_size) [&](LPCSTR __){ log("Debug: in %s: Read %lu bytes from \"%s\" %s", __, *(p_size), (k), (vname)); memset((d), 0, (size_t)*(p_size)); (void)(flags); (void)(t); return true; }(__FUNCTION__)
#   define reg_set_value(k,vname,v,size,type) [&](LPCSTR __){ log("Debug: in %s: Write %lu bytes to \"%s\" %s (%s) content:", __, (size), (k), (vname), reg_type_name.at(type)); if ((type) == REG_SZ) { log_append("\"%s\"",(LPCSTR)(v)); } else { log_append("%lx",*(DWORD*)(v)); } ; return true; }(__FUNCTION__)
#   define reg_create_key(k,o) [&](LPCSTR __){ log("Debug: in %s: Create key \"%s\"", __, (k)); return true; }(__FUNCTION__)
#   define reg_delete_key(k) [&](LPCSTR __){ log("Debug: in %s: Delete key \"%s\"", __, (k)); return true; }(__FUNCTION__)
#   define reg_delete_value(k,v) [&](LPCSTR __){ log("Debug: in %s: Delete value %s from \"%s\"", __, (v), (k)); return true; }(__FUNCTION__)
#   define reg_exist_key(k,o) [&](LPCSTR __){ log("Debug: in %s: Check if \"%s\" exists", __, (k)); return true; }(__FUNCTION__)
#   define reg_exist_value(k,v,o) [&](LPCSTR __){ log("Debug: in %s: Check if value %s exists in \"%s\"", __, (v), (k)); return true; }(__FUNCTION__)
#else
/// \param out__type can be NULL
/// HKEY hkey_from_bytes(int rtl_bytes);
bool reg_get_value(LPCSTR  fullkey, LPCSTR value_name, DATA data, DWORD dwFlags, LPDWORD out_type,
                   LPDWORD out_data_size);
bool reg_set_value(LPCSTR fullkey, LPCSTR value_name, CDATA value, DWORD value_size, DWORD type);
bool reg_create_key(LPCSTR fullkey, PHKEY out_hkey);
bool reg_delete_key(LPCSTR fullkey); // recursive delete key
bool reg_delete_value(LPCSTR fullkey, LPCSTR value_name);
bool reg_exist_key(LPCSTR fullkey, PHKEY out_hkey);
bool reg_exist_value(LPCSTR fullkey, LPCSTR value_name, PHKEY out_hkey);
#endif
