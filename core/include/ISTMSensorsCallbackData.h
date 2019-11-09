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

#pragma once

#include <cstdint>
#include <vector>
#include <STMSensorType.h>

class ISTMSensorsCallbackData {
public:
    virtual ~ISTMSensorsCallbackData(void) = default;

    /**
     * Return sensor data vector reference
     */
    const std::vector<float>& getData(void) const;

    /**
     * Return timestamp value associated with data
     */
    int64_t getTimestamp(void) const;

    /**
     * Return sensor type
     */
    STMSensorType getSensorType(void) const;

    /**
     * Return sensor handle
     */
    uint32_t getSensorHandle(void) const;

    /**
     * Return true if sensor is a wake up sensor
     */
    bool isWakeUpSensor(void) const;

protected:
    /**
     * Sensor data vector
     */
    std::vector<float> sensorsData;

    /**
     * Sensor data timestamp (ns)
     */
    int64_t timestamp;

    /**
     * Sensor type
     */
    STMSensorType sensorType;

    /**
     * Sensor handle
     */
    uint32_t sensorHandle;

    /**
     * Sensor data may have wake up the AP
     */
    bool wakeUpSensor;
};
