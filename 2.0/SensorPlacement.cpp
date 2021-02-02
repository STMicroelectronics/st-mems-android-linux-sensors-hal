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

#include "SensorPlacement.h"

SensorPlacement::SensorPlacement(void)
    : data({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 })
{

}

/**
 * getPayload: return the payload of the sensor placement (4x3 matrix)
 *
 * Return value: array of 12 elements representing the 4x3 matrix.
 */
   const std::array<float, 12> &SensorPlacement::getPayload(void) const
{
    return data;
}

/**
 * loadFromProp: load the sensor placement data from system properties
 * @sensorType: sensor type.
 */
void SensorPlacement::loadFromProp(stm::core::SensorType sensorType)
{
    using stm::core::SensorType;

    // TODO read from the actual system properties

    switch (sensorType) {
    case SensorType::ACCELEROMETER:
        break;
    case SensorType::MAGNETOMETER:
        break;
    case SensorType::ORIENTATION:
        break;
    case SensorType::GYROSCOPE:
        break;
    case SensorType::PRESSURE:
        break;
    case SensorType::GRAVITY:
        break;
    case SensorType::LINEAR_ACCELERATION:
        break;
    case SensorType::ROTATION_VECTOR:
        break;
    case SensorType::MAGNETOMETER_UNCALIBRATED:
        break;
    case SensorType::GAME_ROTATION_VECTOR:
        break;
    case SensorType::GYROSCOPE_UNCALIBRATED:
        break;
    case SensorType::GEOMAGNETIC_ROTATION_VECTOR:
        break;
    case SensorType::ACCELEROMETER_UNCALIBRATED:
        break;
    default:
        break;
    }
}
