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
#include <string>
/**
 * \brief get string representation of GetLastError()
 * \return error message
 */
std::string last_error_string();


/**
 * \brief get string representation of errno_t
 * \param err error id
 * \return error message of err
 */
std::string errno_string(errno_t err);
