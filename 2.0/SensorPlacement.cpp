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

#include <regex>
#include <cutils/properties.h>

#include "SensorPlacement.h"

static const std::string placementPropNameSuffix = "ro.stm.sensors.placement.";

SensorPlacement::SensorPlacement(void)
    : data({ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f })
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
    std::string propertyName(placementPropNameSuffix);
    char propertyValue[PROPERTY_VALUE_MAX];
    using stm::core::SensorType;

    switch (sensorType) {
    case SensorType::ACCELEROMETER:
    case SensorType::ACCELEROMETER_UNCALIBRATED:
        propertyName += "accel";
        break;
    case SensorType::MAGNETOMETER:
    case SensorType::MAGNETOMETER_UNCALIBRATED:
        propertyName += "magn";
        break;
    case SensorType::ORIENTATION:
        propertyName += "orientation";
        break;
    case SensorType::GYROSCOPE:
    case SensorType::GYROSCOPE_UNCALIBRATED:
        propertyName += "gyro";
        break;
    case SensorType::PRESSURE:
        propertyName += "press";
        break;
    case SensorType::GRAVITY:
        propertyName += "gravity";
        break;
    case SensorType::LINEAR_ACCELERATION:
        propertyName += "linear";
        break;
    case SensorType::ROTATION_VECTOR:
        propertyName += "rotation_vector";
        break;
    case SensorType::GAME_ROTATION_VECTOR:
        propertyName += "game_vector";
        break;
    case SensorType::GEOMAGNETIC_ROTATION_VECTOR:
        propertyName += "geomag_vector";
        break;
    default:
        break;
    }
    if (propertyName.size() >= PROPERTY_KEY_MAX) {
        return;
    }

    console.debug(std::string("property get: " + propertyName));

    property_get(propertyName.c_str(), propertyValue, "1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0");

    std::array<float, 12> placementMatrix;
    if (parsePropValue(propertyValue, placementMatrix)) {
        data = placementMatrix;
    }
}

/**
 * parsePropValue: parse string read from system property for sensor placement
 * @value: string value.
 * @placement: return 4x3 matrix storing placement info.
 *
 * Return value: true if placement string is correct and matrix can be used, false otherwise.
 */
bool SensorPlacement::parsePropValue(std::string value,
                                     std::array<float, 12> &placement) const
{
    const std::regex digitsRegex("[+-]?([0-9]*[.])?[0-9]+\\,*");
    int digitsFound = 0;

    auto digitsBegin = std::sregex_iterator(value.begin(),
                                            value.end(),
                                            digitsRegex);

    auto digitsEnd = std::sregex_iterator();

    for (std::sregex_iterator it = digitsBegin; it != digitsEnd; ++it) {
        std::smatch match = *it;
        std::size_t pos = match.str().find(',');
        placement[digitsFound++] = std::stof(match.str().substr(0, pos));
    }
    if (digitsFound == 12) {
        return true;
    }

    console.error("sensor placement matrix is not valid, please check format!");

    return false;
}
