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

#include <aidl/android/hardware/sensors/ISensors.h>
#include <ISTMSensorsCallbackData.h>
#include <STMSensor.h>

namespace android {
namespace hardware {
namespace sensors {
namespace implementation {

using aidl::android::hardware::sensors::Event;
using aidl::android::hardware::sensors::SensorInfo;
using aidl::android::hardware::sensors::SensorType;

bool convertFromSTMSensorType(const stm::core::SensorType &type,
                             SensorType &sensorType);

bool convertFromSTMSensor(const stm::core::STMSensor &src,
                          SensorInfo *dst);

void convertFromSTMSensorData(const stm::core::ISTMSensorsCallbackData &sensorData,
                              Event &event);

}  // namespace implementation
}  // namespace sensors
}  // namespace hardware
}  // namespace android

