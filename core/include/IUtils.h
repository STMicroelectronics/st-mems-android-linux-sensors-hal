/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019-2020 STMicroelectronics
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdint>

namespace stm {
namespace core {

class IUtils {
protected:
    IUtils(void) = default;
    virtual ~IUtils(void) = default;

public:
    IUtils(const IUtils &) = delete;
    IUtils& operator= (const IUtils &) = delete;

    /**
     * getInstance: retrieve utils instance
     *
     * Return value: valid instance of the utils.
     */
    static IUtils& getInstance(void);

    /**
     * getTime: return the time since the system was booted, and include deep sleep
     *
     * Return value: nsec since the system was booted.
     */
    virtual int64_t getTime(void) const = 0;
};

} // namespace core
} // namespace stm
