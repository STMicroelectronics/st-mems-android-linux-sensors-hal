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

#include <STMSensor.h>

STMSensor::STMSensor(const std::string &name,
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
                     bool wakeUp)
          : name(name),
            vendor(vendor),
            handle(handle),
            version(version),
            type(type),
            maxRange(maxRange),
            resolution(resolution),
            power(power),
            minRateHz(minRateHz),
            maxRateHz(maxRateHz),
            onChange(false),
            fifoRsvdCount(fifoRsvdCount),
            fifoMaxCount(fifoMaxCount),
            wakeUp(wakeUp)
{
}

STMSensor::STMSensor(const std::string &name,
                     const std::string &vendor,
                     uint32_t handle,
                     int32_t version,
                     STMSensorType type,
                     float maxRange,
                     float resolution,
                     float power,
                     uint32_t fifoRsvdCount,
                     uint32_t fifoMaxCount,
                     bool wakeUp)
          : name(name),
            vendor(vendor),
            handle(handle),
            version(version),
            type(type),
            maxRange(maxRange),
            resolution(resolution),
            power(power),
            minRateHz(0),
            maxRateHz(0),
            onChange(true),
            fifoRsvdCount(fifoRsvdCount),
            fifoMaxCount(fifoMaxCount),
            wakeUp(wakeUp)
{
}

const std::string& STMSensor::getName(void) const
{
    return name;
}

const std::string& STMSensor::getVendor(void) const
{
    return vendor;
}

uint32_t STMSensor::getHandle(void) const
{
    return handle;
}

int32_t STMSensor::getVersion(void) const
{
    return version;
}

STMSensorType STMSensor::getType(void) const
{
    return type;
}

float STMSensor::getMaxRange(void) const
{
    return maxRange;
}

float STMSensor::getResolution(void) const
{
    return resolution;
}

float STMSensor::getPower(void) const
{
    return power;
}

float STMSensor::getMinRateHz(void) const
{
    return minRateHz;
}

float STMSensor::getMaxRateHz(void) const
{
    return maxRateHz;
}

bool STMSensor::isOnChange(void) const
{
    return onChange;
}

uint32_t STMSensor::getFifoRsvdCount(void) const
{
    return fifoRsvdCount;
}

uint32_t STMSensor::getFifoMaxCount(void) const
{
    return fifoMaxCount;
}

bool STMSensor::isWakeUp(void) const
{
    return wakeUp;
}
