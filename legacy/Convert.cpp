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

#include <cstdlib>
#include <cmath>

#include "halVersion.h"
#include "Convert.h"

static bool convertFromSTMSensorType(const stm::core::STMSensorType &type,
                                     int &sensorType,
                                     bool &isPartOfSensorList)
{
    isPartOfSensorList = true;

    switch (type) {
    case stm::core::STMSensorType::META_DATA:
        sensorType = SENSOR_TYPE_META_DATA;
        isPartOfSensorList = false;
        break;
    case stm::core::STMSensorType::ACCELEROMETER:
        sensorType = SENSOR_TYPE_ACCELEROMETER;
        break;
    case stm::core::STMSensorType::MAGNETOMETER:
        sensorType = SENSOR_TYPE_MAGNETIC_FIELD;
        break;
    case stm::core::STMSensorType::ORIENTATION:
        sensorType = SENSOR_TYPE_ORIENTATION;
        break;
    case stm::core::STMSensorType::GYROSCOPE:
        sensorType = SENSOR_TYPE_GYROSCOPE;
        break;
    case stm::core::STMSensorType::LIGHT:
        sensorType = SENSOR_TYPE_LIGHT;
        break;
    case stm::core::STMSensorType::PRESSURE:
        sensorType = SENSOR_TYPE_PRESSURE;
        break;
    case stm::core::STMSensorType::INTERNAL_TEMPERATURE:
        isPartOfSensorList = false;
        return false;
    case stm::core::STMSensorType::PROXIMITY:
        sensorType = SENSOR_TYPE_PROXIMITY;
        break;
    case stm::core::STMSensorType::GRAVITY:
        sensorType = SENSOR_TYPE_GRAVITY;
        break;
    case stm::core::STMSensorType::LINEAR_ACCELERATION:
        sensorType = SENSOR_TYPE_LINEAR_ACCELERATION;
        break;
    case stm::core::STMSensorType::ROTATION_VECTOR:
        sensorType = SENSOR_TYPE_ROTATION_VECTOR;
        break;
    case stm::core::STMSensorType::RELATIVE_HUMIDITY:
        sensorType = SENSOR_TYPE_RELATIVE_HUMIDITY;
        break;
    case stm::core::STMSensorType::AMBIENT_TEMPERATURE:
        sensorType = SENSOR_TYPE_AMBIENT_TEMPERATURE;
        break;
    case stm::core::STMSensorType::MAGNETOMETER_UNCALIBRATED:
        sensorType = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
        break;
    case stm::core::STMSensorType::GAME_ROTATION_VECTOR:
        sensorType = SENSOR_TYPE_GAME_ROTATION_VECTOR;
        break;
    case stm::core::STMSensorType::GYROSCOPE_UNCALIBRATED:
        sensorType = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;
        break;
    case stm::core::STMSensorType::SIGNIFICANT_MOTION:
        sensorType = SENSOR_TYPE_SIGNIFICANT_MOTION;
        break;
    case stm::core::STMSensorType::STEP_DETECTOR:
        sensorType = SENSOR_TYPE_STEP_DETECTOR;
        break;
    case stm::core::STMSensorType::STEP_COUNTER:
        sensorType = SENSOR_TYPE_STEP_COUNTER;
        break;
    case stm::core::STMSensorType::GEOMAGNETIC_ROTATION_VECTOR:
        sensorType = SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
        break;
    case stm::core::STMSensorType::HEART_RATE:
        sensorType = SENSOR_TYPE_HEART_RATE;
        break;
    case stm::core::STMSensorType::TILT_DETECTOR:
        sensorType = SENSOR_TYPE_TILT_DETECTOR;
        break;
    case stm::core::STMSensorType::WAKE_GESTURE:
        sensorType = SENSOR_TYPE_WAKE_GESTURE;
        break;
    case stm::core::STMSensorType::GLANCE_GESTURE:
        sensorType = SENSOR_TYPE_GLANCE_GESTURE;
        break;
    case stm::core::STMSensorType::PICK_UP_GESTURE:
        sensorType = SENSOR_TYPE_PICK_UP_GESTURE;
        break;
    case stm::core::STMSensorType::WRIST_TILT_GESTURE:
        sensorType = SENSOR_TYPE_WRIST_TILT_GESTURE;
        break;
    case stm::core::STMSensorType::DEVICE_ORIENTATION:
        sensorType = SENSOR_TYPE_DEVICE_ORIENTATION;
        break;
    case stm::core::STMSensorType::POSE_6DOF:
        sensorType = SENSOR_TYPE_POSE_6DOF;
        break;
    case stm::core::STMSensorType::STATIONARY_DETECT:
        sensorType = SENSOR_TYPE_STATIONARY_DETECT;
        break;
    case stm::core::STMSensorType::MOTION_DETECT:
        sensorType = SENSOR_TYPE_MOTION_DETECT;
        break;
    case stm::core::STMSensorType::HEART_BEAT:
        sensorType = SENSOR_TYPE_HEART_BEAT;
        break;
    case stm::core::STMSensorType::DYNAMIC_SENSOR_META:
        sensorType = SENSOR_TYPE_DYNAMIC_SENSOR_META;
        isPartOfSensorList = false;
        break;
    case stm::core::STMSensorType::ADDITIONAL_INFO:
        sensorType = SENSOR_TYPE_ADDITIONAL_INFO;
        isPartOfSensorList = false;
        break;
#if SENSORS_HAL_DEVICE_API_VERSION >= SENSORS_DEVICE_API_VERSION_1_4
    case stm::core::STMSensorType::LOW_LATENCY_OFFBODY_DETECT:
        sensorType = SENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT;
        break;
    case stm::core::STMSensorType::ACCELEROMETER_UNCALIBRATED:
        sensorType = SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED;
        break;
#endif /* SENSORS_HAL_DEVICE_API_VERSION */
    default:
        isPartOfSensorList = false;
        return false;
    }

    return true;
}

bool convertFromSTMSensorType(const stm::core::STMSensorType &type,
                              int &sensorType)
{
    bool isPartOfSensorList;

    return convertFromSTMSensorType(type, sensorType, isPartOfSensorList);
}

static bool isOneShot(int sensorType)
{
    switch (sensorType) {
    case SENSOR_TYPE_GLANCE_GESTURE:
    case SENSOR_TYPE_PICK_UP_GESTURE:
    case SENSOR_TYPE_SIGNIFICANT_MOTION:
    case SENSOR_TYPE_WAKE_GESTURE:
        return true;
    default:
        break;
    }

    return false;
}

static bool isSpecial(int sensorType)
{
    switch (sensorType) {
    case SENSOR_TYPE_STEP_DETECTOR:
    case SENSOR_TYPE_TILT_DETECTOR:
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

bool convertFromSTMSensor(const stm::core::STMSensor &src,
                          struct sensor_t *dst)
{
    int sensorType;
    bool isPartOfSensorList;

    if (!convertFromSTMSensorType(src.getType(), sensorType, isPartOfSensorList) ||
        !isPartOfSensorList) {
        return false;
    }

    dst->name = strdup(src.getName().c_str());
    if (dst->name == nullptr) {
        return false;
    }

    dst->vendor = strdup(src.getVendor().c_str());
    if (dst->vendor == nullptr) {
        free((void *)dst->name);
        return false;
    }

    dst->version = src.getVersion();
    dst->handle = src.getHandle();
    dst->type = sensorType;
    dst->maxRange = std::ceil(src.getMaxRange());
    dst->resolution = src.getResolution();
    dst->power = src.getPower();

    if (src.isOnChange()) {
        if (isOneShot(dst->type)) {
            dst->flags |= SENSOR_FLAG_ONE_SHOT_MODE;
            dst->minDelay = -1;
            dst->maxDelay = 0;
        } else if (isSpecial(dst->type)) {
            dst->flags |= SENSOR_FLAG_SPECIAL_REPORTING_MODE;
            dst->minDelay = 0;
            dst->maxDelay = 0;
        } else {
            dst->flags |= SENSOR_FLAG_ON_CHANGE_MODE;
            dst->minDelay = 0;
            dst->maxDelay = convertFromHzToUs(src.getMinRateHz());
        }
    } else {
        dst->flags |= SENSOR_FLAG_CONTINUOUS_MODE;
        dst->minDelay = convertFromHzToUs(src.getMaxRateHz());
        dst->maxDelay = convertFromHzToUs(src.getMinRateHz());
    }

    dst->fifoReservedEventCount = src.getFifoRsvdCount();
    dst->fifoMaxEventCount = src.getFifoMaxCount();
    dst->stringType = "";
    dst->requiredPermission = "";

    if (src.isWakeUp()) {
        dst->flags |= SENSOR_FLAG_WAKE_UP;
    }

    return true;
}

void convertFromSTMSensorData(const stm::core::ISTMSensorsCallbackData &sensorData,
                              struct sensors_event_t *event)
{
    using stm::core::STMSensorType;

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
        event->acceleration.x = sensorData.getData().at(0);
        event->acceleration.y = sensorData.getData().at(1);
        event->acceleration.z = sensorData.getData().at(2);
        break;
    case STMSensorType::GAME_ROTATION_VECTOR:
        if (sensorData.getData().size() < 4) {
            return;
        }
        event->data[0] = sensorData.getData().at(0);
        event->data[1] = sensorData.getData().at(1);
        event->data[2] = sensorData.getData().at(2);
        event->data[3] = sensorData.getData().at(3);
        break;
    case STMSensorType::MAGNETOMETER_UNCALIBRATED:
    case STMSensorType::GYROSCOPE_UNCALIBRATED:
    case STMSensorType::ACCELEROMETER_UNCALIBRATED:
        if (sensorData.getData().size() < 6) {
            return;
        }
        event->uncalibrated_gyro.x_uncalib = sensorData.getData().at(0);
        event->uncalibrated_gyro.y_uncalib = sensorData.getData().at(1);
        event->uncalibrated_gyro.z_uncalib = sensorData.getData().at(2);
        event->uncalibrated_gyro.x_bias = sensorData.getData().at(3);
        event->uncalibrated_gyro.y_bias = sensorData.getData().at(4);
        event->uncalibrated_gyro.z_bias = sensorData.getData().at(5);
        break;
    case STMSensorType::META_DATA:
        event->sensor = 0;
        event->timestamp = 0;
        event->version = META_DATA_VERSION;
        event->meta_data.sensor = sensorData.getSensorHandle();
        event->meta_data.what = META_DATA_FLUSH_COMPLETE;
        break;
    default:
        break;
    }
}
