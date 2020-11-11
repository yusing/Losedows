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
/**
 * \brief string copy (unsafe)
 * \param dst destination
 * \param src source
 */
void m_strcpy(char* dst, const char* src);


/**
 * \brief check if a string match the other (unsafe)
 * \param str a string
 * \param to_match string to match
 * \return 
 */
bool str_match(const char* str, const char* to_match);

/**
 * \brief check if a string match the other (safe)
 * \param str a string
 * \param to_match string to match
 * \return
 */
bool str_match_s(const char* str, const char* to_match);
