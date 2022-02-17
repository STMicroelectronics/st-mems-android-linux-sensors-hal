/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2022 STMicroelectronics
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

#include <cstddef>
#include <cstdint>
#include <deque>
#include <cerrno>

#include "STMTimesync.h"

static const int maxPointsMin = 5;

int STMTimesync::init(uint8_t maxPoints)
{
    if (maxPoints < maxPointsMin) {
        return -EINVAL;
    }

    return 0;
}

void STMTimesync::reset(void)
{

}

bool STMTimesync::add(int64_t timestamp1, int64_t timestamp2)
{
    if ((timestamp1 <= lastTimestamp1) || (timestamp2 <= lastTimestamp2)) {
        return false;
    }

    lastTimestamp1 = timestamp1;
    lastTimestamp2 = timestamp2;

    return true;
}

bool STMTimesync::estimate(int64_t timestamp2, int64_t &timestamp1)
{
    (void) timestamp2;
    (void) timestamp1;

    return false;
}

const std::string& STMTimesync::getLibVersion(void)
{
    static const std::string libVersion("stm-timesync-lib-mock");

    return libVersion;
}
