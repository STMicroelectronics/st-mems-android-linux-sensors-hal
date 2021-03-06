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

#include <string>

namespace stm {
namespace core {

class IConsole {
protected:
    IConsole(void) = default;
    virtual ~IConsole(void) = default;

public:
    IConsole(const IConsole &) = delete;
    IConsole& operator= (const IConsole &) = delete;

    /**
     * getInstance: retrieve console instance
     *
     * Return value: valid instance of the console.
     */
    static IConsole& getInstance(void);

    /**
     * info: print info message
     * @message: message string to be printed
     */
    virtual void info(const std::string &message) const = 0;

    /**
     * warning: print warning message
     * @message: message string to be printed
     */
    virtual void warning(const std::string &message) const = 0;

    /**
     * error: print error message
     * @message: message string to be printed
     */
    virtual void error(const std::string &message) const = 0;

    /**
     * debug: print debug message
     * @message: message string to be printed
     */
    virtual void debug(const std::string &message) const = 0;
};

} // namespace core
} // namespace stm
