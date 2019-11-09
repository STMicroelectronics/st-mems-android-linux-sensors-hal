/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019 STMicroelectronics
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

#include <iostream>
#include <log/log.h>

#include "Console.h"

void Console::info(const std::string &message) const
{
    std::string messageToPrint = message + "\n";

    ALOGI("%s", messageToPrint.c_str());
}

void Console::error(const std::string &message) const
{
    std::string messageToPrint = message + "\n";

    ALOGE("%s", messageToPrint.c_str());
}

void Console::debug(const std::string &message)const
{
    std::string messageToPrint = message + "\n";

    ALOGD("%s", messageToPrint.c_str());
}
