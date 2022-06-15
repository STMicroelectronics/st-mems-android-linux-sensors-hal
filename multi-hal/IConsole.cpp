/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019-2022 STMicroelectronics
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

#define LOG_TAG "sensors-multihal@" HAL_MULTIHAL_VERSION_STR "-stm"

#include <log/log.h>
#include <iostream>
#include <string>

#include <IConsole.h>

namespace android {
namespace hardware {
namespace sensors {
namespace stm {
namespace multihal {

class Console : public ::stm::core::IConsole {
public:
    Console(void) = default;
    ~Console(void) = default;

    Console(const Console&) = delete;
    Console& operator= (const Console&) = delete;

    void info(const std::string& message) const override;

    void warning(const std::string& message) const override;

    void error(const std::string& message) const override;

    void debug(const std::string& message) const override;
};

void Console::info(const std::string& message) const
{
    std::string messageToPrint = message + "\n";

    ALOGI("%s", messageToPrint.c_str());
}

void Console::warning(const std::string& message) const
{
    std::string messageToPrint = message + "\n";

    ALOGW("%s", messageToPrint.c_str());
}

void Console::error(const std::string& message) const
{
    std::string messageToPrint = message + "\n";

    ALOGE("%s", messageToPrint.c_str());
}

void Console::debug(const std::string& message) const
{
    std::string messageToPrint = message + "\n";

    ALOGD("%s", messageToPrint.c_str());
}

}  // namespace multihal
}  // namespace stm
}  // namespace sensors
}  // namespace hardware
}  // namespace android

namespace stm {
namespace core {

using android::hardware::sensors::stm::multihal::Console;

IConsole& IConsole::getInstance(void)
{
    static Console instance;

    return instance;
}

}  // namespace core
}  // namespace stm
