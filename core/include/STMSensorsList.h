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
#include <vector>
#include <STMSensor.h>

namespace stm {
namespace core {

class STMSensorsList {
public:
    STMSensorsList(void) = default;

    bool addSensor(STMSensor &sensor);

    const std::vector<STMSensor>& getList(void) const;

    void clear();

private:
    /**
     * List of available sensors
     */
    std::vector<STMSensor> list;
};

} // namespace core
} // namespace stm
