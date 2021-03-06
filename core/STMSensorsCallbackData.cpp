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

#include <STMSensorsCallbackData.h>

namespace stm {
namespace core {

STMSensorsCallbackData::STMSensorsCallbackData(uint32_t sensorHandle,
                                               SensorType sensorType,
                                               int64_t timestamp,
                                               const std::vector<float> &data)
{
    this->sensorHandle = sensorHandle;
    this->sensorType = sensorType;
    this->timestamp = timestamp;
    this->wakeUpSensor = false;
    this->sensorsData.assign(data.begin(), data.end());
}

} // namespace core
} // namespace stm
