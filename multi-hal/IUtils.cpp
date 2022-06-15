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

#include <utils/SystemClock.h>

#include <IUtils.h>

namespace android {
namespace hardware {
namespace sensors {
namespace stm {
namespace multihal {

class Utils : public ::stm::core::IUtils {
public:
    Utils(void) = default;
    ~Utils(void) = default;

    Utils(const Utils&) = delete;
    Utils& operator= (const Utils&) = delete;

    int64_t getTime(void) const override;
};

int64_t Utils::getTime(void) const
{
    return android::elapsedRealtimeNano();
}

}  // namespace multihal
}  // namespace stm
}  // namespace sensors
}  // namespace hardware
}  // namespace android

namespace stm {
namespace core {

using android::hardware::sensors::stm::multihal::Utils;

IUtils& IUtils::getInstance(void)
{
    static Utils instance;

    return instance;
}

}  // namespace core
}  // namespace stm
