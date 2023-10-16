/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2022 STMicroelectronics
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

#include <android-base/properties.h>

#include "AndroidPropertiesLoader.h"
#include <IConsole.h>

int AndroidPropertiesLoader::readInt(PropertyId property) const
{
    std::string propName;

    switch (property) {
    case PropertyId::MAX_ODR:
        propName = "persist.vendor.stm.sensors.max-odr";
        break;
    default:
        return 0;
    }

    return android::base::GetIntProperty(propName, 0);
}

std::string AndroidPropertiesLoader::readString(SensorPropertyId property,
                                                SensorType sensorType) const
{
    static const std::string emptyString = "";
    std::string propName;

    switch (property) {
    case SensorPropertyId::ROTATION_MATRIX_1:
        propName = "persist.vendor.stm.sensors.rot-matrix-1.";
        break;
    case SensorPropertyId::ROTATION_MATRIX_2:
        propName = "persist.vendor.stm.sensors.rot-matrix-2.";
        break;
    case SensorPropertyId::SENSOR_PLACEMENT_1:
        propName = "persist.vendor.stm.sensors.placement-1.";
        break;
    case SensorPropertyId::SENSOR_PLACEMENT_2:
        propName = "persist.vendor.stm.sensors.placement-2.";
        break;
     default:
        return emptyString;
    }

    switch (sensorType) {
    case SensorType::ACCELEROMETER:
        propName += "accel";
        break;
    case SensorType::MAGNETOMETER:
        propName += "magn";
        break;
    case SensorType::GYROSCOPE:
        propName += "gyro";
        break;
    default:
        return emptyString;
    }

    return android::base::GetProperty(propName, emptyString);
}

int AndroidPropertiesLoader::readInt(SensorPropertyId property,
                                     SensorType sensorType) const
{
    std::string propName;

    switch (property) {
    case SensorPropertyId::MAX_RANGE:
        propName = "persist.vendor.stm.sensors.max-range.";
        break;
    default:
        return 0;
    }

    switch (sensorType) {
    case SensorType::ACCELEROMETER:
        propName += "accel";
        break;
    case SensorType::MAGNETOMETER:
        propName += "magn";
        break;
    case SensorType::GYROSCOPE:
        propName += "gyro";
        break;
    default:
        return 0;
    }

    return android::base::GetIntProperty(propName, 0);
}
