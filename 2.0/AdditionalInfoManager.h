/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2020 STMicroelectronics
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

#include <android/hardware/sensors/2.0/ISensors.h>
#include <vector>

#include <STMSensorsList.h>
#include "SensorPlacement.h"

using ::android::hardware::sensors::V1_0::Event;
using stm::core::STMSensorsList;

struct AdditionalInfoManager {
public:
    AdditionalInfoManager(const STMSensorsList &list);
    ~AdditionalInfoManager(void) = default;

    const std::vector<Event> getPayload(int sensorHandle, int64_t timestamp) const;

    bool isSupported(int sensorHandle) const;

private:
    /**
     * Is additional info available for this sensor handle?
     */
    std::vector<bool> supported;

    /**
     * Sensors placement data
     */
    std::vector<SensorPlacement> placements;

    Event makeSensorPlacementFrame(int sensorHandle, int serial, int64_t timestamp) const;

    static Event getBeginFrame(int sensorHandle, int64_t timestamp);

    static Event getEndFrame(int sensorHandle, int serial, int64_t timestamp);
};
