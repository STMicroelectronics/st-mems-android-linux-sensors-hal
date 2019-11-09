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
#include <string>

#include <STMSensorType.h>

class STMSensor {
public:
    STMSensor(const std::string &name,
              const std::string &vendor,
              uint32_t handle,
              int32_t version,
              STMSensorType type,
              float maxRange,
              float resolution,
              float power,
              float minRateHz,
              float maxRateHz,
              uint32_t fifoRsvdCount,
              uint32_t fifoMaxCount,
              bool wakeUp);

    STMSensor(const std::string &name,
              const std::string &vendor,
              uint32_t handle,
              int32_t version,
              STMSensorType type,
              float maxRange,
              float resolution,
              float power,
              uint32_t fifoRsvdCount,
              uint32_t fifoMaxCount,
              bool wakeUp);

    const std::string& getName(void) const;
    const std::string& getVendor(void)const;
    uint32_t getHandle(void) const;
    int32_t getVersion(void) const;
    STMSensorType getType(void) const;
    float getMaxRange(void) const;
    float getResolution(void) const;
    float getPower(void) const;
    float getMinRateHz(void) const;
    float getMaxRateHz(void) const;
    bool isOnChange(void) const;
    uint32_t getFifoRsvdCount(void) const;
    uint32_t getFifoMaxCount(void) const;
    bool isWakeUp(void) const;

private:
    /**
     * Sensor name
     */
    const std::string name;

    /**
     * Sensor vendor name
     */
    const std::string vendor;

    /**
     * Sensor handle number, used to identify the sensor (unique number)
     */
    const uint32_t handle;

    /**
     * Sensor version, if based on algo, updated in case algo get updated
     */
    const int32_t version;

    /**
     * Sensor type
     */
    const STMSensorType type;

    /**
     * Sensor measurement max range,
     * unit specified in STMSensorType
     */
    const float maxRange;

    /**
     * Sensor resolution,
     * unit specified in STMSensorType
     */
    const float resolution;

    /**
     * Sensor power consumption
     */
    const float power;

    /**
     * Sensor minimum rate in Hz
     */
     const float minRateHz;

    /**
     * Sensor maximum rate in Hz
     */
    const float maxRateHz;

    /**
     * Flags used to indicate sensor produce data NOT in continuous mode
     */
    const bool onChange;

    /**
     * Number of reserved samples that can be stored in fifo (private space in fifo)
     */
    const uint32_t fifoRsvdCount;

    /**
     * Maximum number of samples that can be stored in fifo (shared space in fifo)
     */
    const uint32_t fifoMaxCount;

    /**
     * True if sensor can wake up the system from suspend
     */
    const bool wakeUp;
};
