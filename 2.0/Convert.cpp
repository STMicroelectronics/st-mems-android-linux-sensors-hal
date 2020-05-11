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

#include <cmath>

#include "Convert.h"

namespace android {
namespace hardware {
namespace sensors {

static bool convertFromSTMSensorType(const STMSensorType &type,
                                     V1_0::SensorType &sensorType,
                                     bool &isPartOfSensorList)
{
    isPartOfSensorList = true;

    switch (type) {
    case STMSensorType::META_DATA:
        sensorType = V1_0::SensorType::META_DATA;
        isPartOfSensorList = false;
        break;
    case STMSensorType::ACCELEROMETER:
        sensorType = V1_0::SensorType::ACCELEROMETER;
        break;
    case STMSensorType::MAGNETOMETER:
        sensorType = V1_0::SensorType::MAGNETIC_FIELD;
        break;
    case STMSensorType::ORIENTATION:
        sensorType = V1_0::SensorType::ORIENTATION;
        break;
    case STMSensorType::GYROSCOPE:
        sensorType = V1_0::SensorType::GYROSCOPE;
        break;
    case STMSensorType::LIGHT:
        sensorType = V1_0::SensorType::LIGHT;
        break;
    case STMSensorType::PRESSURE:
        sensorType = V1_0::SensorType::PRESSURE;
        break;
    case STMSensorType::INTERNAL_TEMPERATURE:
        isPartOfSensorList = false;
        return false;
    case STMSensorType::PROXIMITY:
        sensorType = V1_0::SensorType::PROXIMITY;
        break;
    case STMSensorType::GRAVITY:
        sensorType = V1_0::SensorType::GRAVITY;
        break;
    case STMSensorType::LINEAR_ACCELERATION:
        sensorType = V1_0::SensorType::LINEAR_ACCELERATION;
        break;
    case STMSensorType::ROTATION_VECTOR:
        sensorType = V1_0::SensorType::ROTATION_VECTOR;
        break;
    case STMSensorType::RELATIVE_HUMIDITY:
        sensorType = V1_0::SensorType::RELATIVE_HUMIDITY;
        break;
    case STMSensorType::AMBIENT_TEMPERATURE:
        sensorType = V1_0::SensorType::AMBIENT_TEMPERATURE;
        break;
    case STMSensorType::MAGNETOMETER_UNCALIBRATED:
        sensorType = V1_0::SensorType::MAGNETIC_FIELD_UNCALIBRATED;
        break;
    case STMSensorType::GAME_ROTATION_VECTOR:
        sensorType = V1_0::SensorType::GAME_ROTATION_VECTOR;
        break;
    case STMSensorType::GYROSCOPE_UNCALIBRATED:
        sensorType = V1_0::SensorType::GYROSCOPE_UNCALIBRATED;
        break;
    case STMSensorType::SIGNIFICANT_MOTION:
        sensorType = V1_0::SensorType::SIGNIFICANT_MOTION;
        break;
    case STMSensorType::STEP_DETECTOR:
        sensorType = V1_0::SensorType::STEP_DETECTOR;
        break;
    case STMSensorType::STEP_COUNTER:
        sensorType = V1_0::SensorType::STEP_COUNTER;
        break;
    case STMSensorType::GEOMAGNETIC_ROTATION_VECTOR:
        sensorType = V1_0::SensorType::GEOMAGNETIC_ROTATION_VECTOR;
        break;
    case STMSensorType::HEART_RATE:
        sensorType = V1_0::SensorType::HEART_RATE;
        break;
    case STMSensorType::TILT_DETECTOR:
        sensorType = V1_0::SensorType::TILT_DETECTOR;
        break;
    case STMSensorType::WAKE_GESTURE:
        sensorType = V1_0::SensorType::WAKE_GESTURE;
        break;
    case STMSensorType::GLANCE_GESTURE:
        sensorType = V1_0::SensorType::GLANCE_GESTURE;
        break;
    case STMSensorType::PICK_UP_GESTURE:
        sensorType = V1_0::SensorType::PICK_UP_GESTURE;
        break;
    case STMSensorType::WRIST_TILT_GESTURE:
        sensorType = V1_0::SensorType::WRIST_TILT_GESTURE;
        break;
    case STMSensorType::DEVICE_ORIENTATION:
        sensorType = V1_0::SensorType::DEVICE_ORIENTATION;
        break;
    case STMSensorType::POSE_6DOF:
        sensorType = V1_0::SensorType::POSE_6DOF;
        break;
    case STMSensorType::STATIONARY_DETECT:
        sensorType = V1_0::SensorType::STATIONARY_DETECT;
        break;
    case STMSensorType::MOTION_DETECT:
        sensorType = V1_0::SensorType::MOTION_DETECT;
        break;
    case STMSensorType::HEART_BEAT:
        sensorType = V1_0::SensorType::HEART_BEAT;
        break;
    case STMSensorType::DYNAMIC_SENSOR_META:
        sensorType = V1_0::SensorType::DYNAMIC_SENSOR_META;
        isPartOfSensorList = false;
        break;
    case STMSensorType::ADDITIONAL_INFO:
        sensorType = V1_0::SensorType::ADDITIONAL_INFO;
        isPartOfSensorList = false;
        break;
    case STMSensorType::LOW_LATENCY_OFFBODY_DETECT:
        sensorType = V1_0::SensorType::LOW_LATENCY_OFFBODY_DETECT;
        break;
    case STMSensorType::ACCELEROMETER_UNCALIBRATED:
        sensorType = V1_0::SensorType::ACCELEROMETER_UNCALIBRATED;
        break;
    default:
        isPartOfSensorList = false;
        return false;
    }

    return true;
}

bool convertFromSTMSensorType(const STMSensorType &type,
                              V1_0::SensorType &sensorType)
{
    bool isPartOfSensorList;

    return convertFromSTMSensorType(type, sensorType, isPartOfSensorList);
}

static bool isOneShot(V1_0::SensorType sensorType)
{
    switch (sensorType) {
    case V1_0::SensorType::GLANCE_GESTURE:
    case V1_0::SensorType::PICK_UP_GESTURE:
    case V1_0::SensorType::SIGNIFICANT_MOTION:
    case V1_0::SensorType::WAKE_GESTURE:
        return true;
    default:
        break;
    }

    return false;
}

static bool isSpecial(V1_0::SensorType sensorType)
{
    switch (sensorType) {
    case V1_0::SensorType::STEP_DETECTOR:
    case V1_0::SensorType::TILT_DETECTOR:
        return true;
    default:
        break;
    }

    return false;
}

static int64_t convertFromHzToUs(float hz)
{
    return 1e6 / hz;
}

bool convertFromSTMSensor(const STMSensor &src, V1_0::SensorInfo *dst)
{
    V1_0::SensorType sensorType;
    bool isPartOfSensorList;

    if (!convertFromSTMSensorType(src.getType(), sensorType, isPartOfSensorList) ||
        !isPartOfSensorList) {
        return false;
    }

    dst->sensorHandle = src.getHandle();
    dst->name = src.getName();
    dst->vendor = src.getVendor();
    dst->version = src.getVersion();
    dst->type = sensorType;
    dst->typeAsString = "";
    dst->maxRange = std::ceil(src.getMaxRange());
    dst->resolution = src.getResolution();
    dst->power = src.getPower();

    if (src.isOnChange()) {
        if (isOneShot(dst->type)) {
            dst->flags |= V1_0::SensorFlagBits::ONE_SHOT_MODE;
            dst->minDelay = -1;
            dst->maxDelay = 0;
        } else if (isSpecial(dst->type)) {
            dst->flags |= V1_0::SensorFlagBits::SPECIAL_REPORTING_MODE;
            dst->minDelay = 0;
            dst->maxDelay = 0;
        } else {
            dst->flags |= V1_0::SensorFlagBits::ON_CHANGE_MODE;
            dst->minDelay = 0;
            dst->maxDelay = convertFromHzToUs(src.getMinRateHz());
        }
    } else {
        dst->flags |= V1_0::SensorFlagBits::CONTINUOUS_MODE;
        dst->minDelay = convertFromHzToUs(src.getMaxRateHz());
        dst->maxDelay = convertFromHzToUs(src.getMinRateHz());
    }

    dst->fifoReservedEventCount = src.getFifoRsvdCount();
    dst->fifoMaxEventCount = src.getFifoMaxCount();
    dst->requiredPermission = "";

    if (src.isWakeUp()) {
        dst->flags |= V1_0::SensorFlagBits::WAKE_UP;
    }

    return true;
}

void convertFromSTMSensorData(const ISTMSensorsCallbackData &sensorData, V1_0::Event &event)
{
    switch (sensorData.getSensorType()) {
    case STMSensorType::ACCELEROMETER:
    case STMSensorType::MAGNETOMETER:
    case STMSensorType::ORIENTATION:
    case STMSensorType::GYROSCOPE:
    case STMSensorType::GRAVITY:
    case STMSensorType::LINEAR_ACCELERATION:
        if (sensorData.getData().size() < 3) {
            return;
        }
        event.u.vec3.x = sensorData.getData().at(0);
        event.u.vec3.y = sensorData.getData().at(1);
        event.u.vec3.z = sensorData.getData().at(2);
        break;
    case STMSensorType::GAME_ROTATION_VECTOR:
        if (sensorData.getData().size() < 4) {
            return;
        }
        event.u.vec4.x = sensorData.getData().at(0);
        event.u.vec4.y = sensorData.getData().at(1);
        event.u.vec4.z = sensorData.getData().at(2);
        event.u.vec4.w = sensorData.getData().at(3);
        break;
    case STMSensorType::MAGNETOMETER_UNCALIBRATED:
    case STMSensorType::GYROSCOPE_UNCALIBRATED:
    case STMSensorType::ACCELEROMETER_UNCALIBRATED:
        if (sensorData.getData().size() < 6) {
            return;
        }
        event.u.uncal.x = sensorData.getData().at(0);
        event.u.uncal.y = sensorData.getData().at(1);
        event.u.uncal.z = sensorData.getData().at(2);
        event.u.uncal.x_bias = sensorData.getData().at(3);
        event.u.uncal.y_bias = sensorData.getData().at(4);
        event.u.uncal.z_bias = sensorData.getData().at(5);
        break;
    case STMSensorType::META_DATA:
        event.u.meta.what = V1_0::MetaDataEventType::META_DATA_FLUSH_COMPLETE;
        break;
    default:
        break;
    }
}

}  // namespace sensors
}  // namespace hardware
}  // namespace android
