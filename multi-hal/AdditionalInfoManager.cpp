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

#include "AdditionalInfoManager.h"

AdditionalInfoManager::AdditionalInfoManager(const STMSensorsList &list)
    : supported(list.getList().size() + 1, false),
      placements(list.getList().size() + 1)
{
    using stm::core::SensorType;

    if (HAL_ENABLE_SENSOR_ADDITIONAL_INFO != 1) return;

    for (const auto &sensor : list.getList()) {
        switch (sensor.getType()) {
        case SensorType::ACCELEROMETER:
        case SensorType::MAGNETOMETER:
        case SensorType::ORIENTATION:
        case SensorType::GYROSCOPE:
        case SensorType::PRESSURE:
        case SensorType::GRAVITY:
        case SensorType::LINEAR_ACCELERATION:
        case SensorType::ROTATION_VECTOR:
        case SensorType::MAGNETOMETER_UNCALIBRATED:
        case SensorType::GAME_ROTATION_VECTOR:
        case SensorType::GYROSCOPE_UNCALIBRATED:
        case SensorType::GEOMAGNETIC_ROTATION_VECTOR:
        case SensorType::ACCELEROMETER_UNCALIBRATED:
            supported[sensor.getHandle()] = true;
            placements[sensor.getHandle()].loadFromProp(sensor.getType());
            break;
        default:
            break;
        }
    }
}

/**
 * getPayload: return additional info events for a specific sensor
 * @sensorHandle: sensor handle.
 * @timestamp: current timestamp, used as starting point for events timestamp.
 *
 * Return value: vector of sensor events for additional info.
 */
const std::vector<Event> AdditionalInfoManager::getPayload(int sensorHandle, int64_t timestamp) const
{
    if (!isSupported(sensorHandle)) return {};

    std::vector<Event> payload;
    int serial = 1;

    payload.push_back(getBeginFrame(sensorHandle, timestamp++));
    payload.push_back(makeSensorPlacementFrame(sensorHandle, serial++, timestamp++));
    payload.push_back(getEndFrame(sensorHandle, serial, timestamp));

    return payload;
}

/**
 * isSupported: check if additional info is supported for a specific sensor
 * @sensorHandle: sensor handle.
 *
 * Return value: true if supported, false otherwise.
 */
bool AdditionalInfoManager::isSupported(int sensorHandle) const
{
    return supported[sensorHandle];
}

/**
 * makeSensorPlacementFrame: create an additional info placement frame
 * @sensorHandle: sensor handle.
 * @serial: additional info serial id.
 * @timestamp: timestamp to use for the sensor event.
 *
 * Return value: sensor event for sensor placement.
 */
Event AdditionalInfoManager::makeSensorPlacementFrame(int sensorHandle, int serial, int64_t timestamp) const
{
    Event sensorEvent;

    sensorEvent.sensorHandle = sensorHandle;
    sensorEvent.sensorType = ::android::hardware::sensors::V2_1::SensorType::ADDITIONAL_INFO;
    sensorEvent.timestamp = timestamp;
    sensorEvent.u.additional.serial = serial;
    sensorEvent.u.additional.type = ::android::hardware::sensors::V1_0::AdditionalInfoType::AINFO_SENSOR_PLACEMENT;
    sensorEvent.u.additional.u.data_float = placements[sensorHandle].getPayload().data();

    constexpr size_t OutDataLen = sizeof(sensorEvent.u.additional.u.data_float) / sizeof(float);

    for (size_t i = placements[sensorHandle].getPayload().size(); i < OutDataLen; ++i) {
        sensorEvent.u.additional.u.data_float[i] = 0.0f;
    }

    return sensorEvent;
}

/**
 * getBeginFrame: create the additional info begin frame
 * @sensorHandle: sensor handle.
 * @timestamp: timestamp to use for the sensor event.
 *
 * Return value: sensor event representing the begin frame of additional info.
 */
Event AdditionalInfoManager::getBeginFrame(int sensorHandle, int64_t timestamp)
{
    Event sensorEvent;

    sensorEvent.sensorHandle = sensorHandle;
    sensorEvent.sensorType = ::android::hardware::sensors::V2_1::SensorType::ADDITIONAL_INFO;
    sensorEvent.timestamp = timestamp;
    sensorEvent.u.additional.serial = 0;
    sensorEvent.u.additional.type = ::android::hardware::sensors::V1_0::AdditionalInfoType::AINFO_BEGIN;

    return sensorEvent;
}

/**
 * getEndFrame: create the additional info end frame
 * @sensorHandle: sensor handle.
 * @timestamp: timestamp to use for the sensor event.
 *
 * Return value: sensor event representing the end frame of additional info.
 */
Event AdditionalInfoManager::getEndFrame(int sensorHandle, int serial, int64_t timestamp)
{
    Event sensorEvent;

    sensorEvent.sensorHandle = sensorHandle;
    sensorEvent.sensorType = ::android::hardware::sensors::V2_1::SensorType::ADDITIONAL_INFO;
    sensorEvent.timestamp = timestamp;
    sensorEvent.u.additional.serial = serial;
    sensorEvent.u.additional.type = ::android::hardware::sensors::V1_0::AdditionalInfoType::AINFO_END;

    return sensorEvent;
}
