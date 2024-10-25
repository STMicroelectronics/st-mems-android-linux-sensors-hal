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
#include "Utils.h"

namespace android {
namespace hardware {
namespace sensors {
namespace stm {
namespace multihal {

static bool convertFromSTMSensorType(const ::stm::core::SensorType& type,
                                     V2_1::SensorType& sensorType,
                                     bool& isPartOfSensorList)
{
    isPartOfSensorList = true;

    switch (type) {
    case ::stm::core::SensorType::META_DATA:
        sensorType = V2_1::SensorType::META_DATA;
        isPartOfSensorList = false;
        break;
    case ::stm::core::SensorType::ACCELEROMETER:
        sensorType = V2_1::SensorType::ACCELEROMETER;
        break;
    case ::stm::core::SensorType::MAGNETOMETER:
        sensorType = V2_1::SensorType::MAGNETIC_FIELD;
        break;
    case ::stm::core::SensorType::ORIENTATION:
        sensorType = V2_1::SensorType::ORIENTATION;
        break;
    case ::stm::core::SensorType::GYROSCOPE:
        sensorType = V2_1::SensorType::GYROSCOPE;
        break;
    case ::stm::core::SensorType::LIGHT:
        sensorType = V2_1::SensorType::LIGHT;
        break;
    case ::stm::core::SensorType::PRESSURE:
        sensorType = V2_1::SensorType::PRESSURE;
        break;
    case ::stm::core::SensorType::INTERNAL_TEMPERATURE:
        isPartOfSensorList = false;
        return false;
    case ::stm::core::SensorType::PROXIMITY:
        sensorType = V2_1::SensorType::PROXIMITY;
        break;
    case ::stm::core::SensorType::GRAVITY:
        sensorType = V2_1::SensorType::GRAVITY;
        break;
    case ::stm::core::SensorType::LINEAR_ACCELERATION:
        sensorType = V2_1::SensorType::LINEAR_ACCELERATION;
        break;
    case ::stm::core::SensorType::ROTATION_VECTOR:
        sensorType = V2_1::SensorType::ROTATION_VECTOR;
        break;
    case ::stm::core::SensorType::RELATIVE_HUMIDITY:
        sensorType = V2_1::SensorType::RELATIVE_HUMIDITY;
        break;
    case ::stm::core::SensorType::AMBIENT_TEMPERATURE:
        sensorType = V2_1::SensorType::AMBIENT_TEMPERATURE;
        break;
    case ::stm::core::SensorType::MAGNETOMETER_UNCALIBRATED:
        sensorType = V2_1::SensorType::MAGNETIC_FIELD_UNCALIBRATED;
        break;
    case ::stm::core::SensorType::GAME_ROTATION_VECTOR:
        sensorType = V2_1::SensorType::GAME_ROTATION_VECTOR;
        break;
    case ::stm::core::SensorType::GYROSCOPE_UNCALIBRATED:
        sensorType = V2_1::SensorType::GYROSCOPE_UNCALIBRATED;
        break;
    case ::stm::core::SensorType::SIGNIFICANT_MOTION:
        sensorType = V2_1::SensorType::SIGNIFICANT_MOTION;
        break;
    case ::stm::core::SensorType::STEP_DETECTOR:
        sensorType = V2_1::SensorType::STEP_DETECTOR;
        break;
    case ::stm::core::SensorType::STEP_COUNTER:
        sensorType = V2_1::SensorType::STEP_COUNTER;
        break;
    case ::stm::core::SensorType::GEOMAGNETIC_ROTATION_VECTOR:
        sensorType = V2_1::SensorType::GEOMAGNETIC_ROTATION_VECTOR;
        break;
    case ::stm::core::SensorType::HEART_RATE:
        sensorType = V2_1::SensorType::HEART_RATE;
        break;
    case ::stm::core::SensorType::TILT_DETECTOR:
        sensorType = V2_1::SensorType::TILT_DETECTOR;
        break;
    case ::stm::core::SensorType::WAKE_GESTURE:
        sensorType = V2_1::SensorType::WAKE_GESTURE;
        break;
    case ::stm::core::SensorType::GLANCE_GESTURE:
        sensorType = V2_1::SensorType::GLANCE_GESTURE;
        break;
    case ::stm::core::SensorType::PICK_UP_GESTURE:
        sensorType = V2_1::SensorType::PICK_UP_GESTURE;
        break;
    case ::stm::core::SensorType::WRIST_TILT_GESTURE:
        sensorType = V2_1::SensorType::WRIST_TILT_GESTURE;
        break;
    case ::stm::core::SensorType::DEVICE_ORIENTATION:
        sensorType = V2_1::SensorType::DEVICE_ORIENTATION;
        break;
    case ::stm::core::SensorType::POSE_6DOF:
        sensorType = V2_1::SensorType::POSE_6DOF;
        break;
    case ::stm::core::SensorType::STATIONARY_DETECT:
        sensorType = V2_1::SensorType::STATIONARY_DETECT;
        break;
    case ::stm::core::SensorType::MOTION_DETECT:
        sensorType = V2_1::SensorType::MOTION_DETECT;
        break;
    case ::stm::core::SensorType::HEART_BEAT:
        sensorType = V2_1::SensorType::HEART_BEAT;
        break;
    case ::stm::core::SensorType::DYNAMIC_SENSOR_META:
        sensorType = V2_1::SensorType::DYNAMIC_SENSOR_META;
        isPartOfSensorList = false;
        break;
    case ::stm::core::SensorType::ADDITIONAL_INFO:
        sensorType = V2_1::SensorType::ADDITIONAL_INFO;
        isPartOfSensorList = false;
        break;
    case ::stm::core::SensorType::LOW_LATENCY_OFFBODY_DETECT:
        sensorType = V2_1::SensorType::LOW_LATENCY_OFFBODY_DETECT;
        break;
    case ::stm::core::SensorType::ACCELEROMETER_UNCALIBRATED:
        sensorType = V2_1::SensorType::ACCELEROMETER_UNCALIBRATED;
        break;
    default:
        isPartOfSensorList = false;
        return false;
    }

    return true;
}

bool convertFromSTMSensorType(const ::stm::core::SensorType& type,
                              V2_1::SensorType& sensorType)
{
    bool isPartOfSensorList;

    return convertFromSTMSensorType(type, sensorType, isPartOfSensorList);
}

static bool isOneShot(const V2_1::SensorType& sensorType)
{
    switch (sensorType) {
    case V2_1::SensorType::GLANCE_GESTURE:
    case V2_1::SensorType::PICK_UP_GESTURE:
    case V2_1::SensorType::SIGNIFICANT_MOTION:
    case V2_1::SensorType::WAKE_GESTURE:
        return true;
    default:
        break;
    }

    return false;
}

static bool isSpecial(const V2_1::SensorType& sensorType)
{
    switch (sensorType) {
    case V2_1::SensorType::STEP_DETECTOR:
    case V2_1::SensorType::TILT_DETECTOR:
        return true;
    default:
        break;
    }

    return false;
}

static int32_t convertFromHzToUs(float hz)
{
    if (hz < 1e-9) {
        return 0;
    }

    return 1e6 / hz;
}

bool convertFromSTMSensor(const ::stm::core::STMSensor& src,
                          V2_1::SensorInfo *dst)
{
    using ::stm::core::SensorType;
    V2_1::SensorType sensorType;
    bool isPartOfSensorList;
    int modId;

    if (!convertFromSTMSensorType(src.getType(), sensorType, isPartOfSensorList) ||
        !isPartOfSensorList) {
        return false;
    }

    dst->sensorHandle = src.getHandle();

    modId = src.getModuleId();
    if (modId == 0)
        return false;
    else if (modId == 1)
        dst->name = src.getName();
    else
        dst->name = src.getName() + " " + std::to_string(modId - 1);

    dst->vendor = src.getVendor();
    dst->version = src.getVersion();
    dst->type = sensorType;
    dst->typeAsString = "";
    if (src.getType() == SensorType::AMBIENT_TEMPERATURE) {
        dst->resolution = src.getResolution() / 1000;
        dst->maxRange = std::ceil(src.getMaxRange()) / 1000;
    } else {
        dst->resolution = src.getResolution();
        dst->maxRange = std::ceil(src.getMaxRange());
    }
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

    if (HAL_ENABLE_DIRECT_REPORT_CHANNEL == 1) {
        if (!src.isOnChange()) {
            dst->flags |= V1_0::SensorFlagBits::DIRECT_CHANNEL_ASHMEM;

            uint32_t directReportMaxRate = static_cast<int32_t>(V1_0::RateLevel::NORMAL);
            if (src.getMaxRateHz() > 180) {
                directReportMaxRate = static_cast<int32_t>(V1_0::RateLevel::FAST);
            }
            if (src.getMaxRateHz() > 780) {
                directReportMaxRate = static_cast<int32_t>(V1_0::RateLevel::VERY_FAST);
            }

            dst->flags |= (directReportMaxRate << static_cast<uint8_t>(V1_0::SensorFlagShift::DIRECT_REPORT));
        }
    }

    return true;
}

void convertFromSTMSensorData(const ::stm::core::ISTMSensorsCallbackData& sensorData,
                              V2_1::Event& event)
{
    using ::stm::core::SensorType;

    switch (sensorData.getSensorType()) {
    case SensorType::MAGNETOMETER:
        if (sensorData.getData().size() < 3) {
            return;
        }
        event.u.vec3.x = Conversion::G_to_uTesla(sensorData.getData().at(0));
        event.u.vec3.y = Conversion::G_to_uTesla(sensorData.getData().at(1));
        event.u.vec3.z = Conversion::G_to_uTesla(sensorData.getData().at(2));
        break;
    case SensorType::ACCELEROMETER:
    case SensorType::ORIENTATION:
    case SensorType::GYROSCOPE:
    case SensorType::GRAVITY:
    case SensorType::LINEAR_ACCELERATION:
        if (sensorData.getData().size() < 3) {
            return;
        }
        event.u.vec3.x = sensorData.getData().at(0);
        event.u.vec3.y = sensorData.getData().at(1);
        event.u.vec3.z = sensorData.getData().at(2);
        break;
    case SensorType::GAME_ROTATION_VECTOR:
        if (sensorData.getData().size() < 4) {
            return;
        }
        event.u.vec4.x = sensorData.getData().at(0);
        event.u.vec4.y = sensorData.getData().at(1);
        event.u.vec4.z = sensorData.getData().at(2);
        event.u.vec4.w = sensorData.getData().at(3);
        break;
    case SensorType::ROTATION_VECTOR:
        if (sensorData.getData().size() < 4) {
            return;
        }
        event.u.data[0] = sensorData.getData().at(0);
        event.u.data[1] = sensorData.getData().at(1);
        event.u.data[2] = sensorData.getData().at(2);
        event.u.data[3] = sensorData.getData().at(3);
        /* values[4]: estimated heading Accuracy (in radians) (-1 if unavailable) */
        event.u.data[4] = -1;
        break;
    case SensorType::MAGNETOMETER_UNCALIBRATED:
        if (sensorData.getData().size() < 6) {
            return;
        }
        event.u.uncal.x = Conversion::G_to_uTesla(sensorData.getData().at(0));
        event.u.uncal.y = Conversion::G_to_uTesla(sensorData.getData().at(1));
        event.u.uncal.z = Conversion::G_to_uTesla(sensorData.getData().at(2));
        event.u.uncal.x_bias = Conversion::G_to_uTesla(sensorData.getData().at(3));
        event.u.uncal.y_bias = Conversion::G_to_uTesla(sensorData.getData().at(4));
        event.u.uncal.z_bias = Conversion::G_to_uTesla(sensorData.getData().at(5));
        break;
    case SensorType::GYROSCOPE_UNCALIBRATED:
    case SensorType::ACCELEROMETER_UNCALIBRATED:
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
    case SensorType::META_DATA:
        event.u.meta.what = V1_0::MetaDataEventType::META_DATA_FLUSH_COMPLETE;
        break;
    case SensorType::AMBIENT_TEMPERATURE:
        if (sensorData.getData().size() < 1) {
            return;
        }
        event.u.scalar = sensorData.getData().at(0) / 1000.0f;
        break;
    case SensorType::PRESSURE:
    case SensorType::RELATIVE_HUMIDITY:
        if (sensorData.getData().size() < 1) {
            return;
        }
        event.u.scalar = sensorData.getData().at(0);
        break;
    default:
        break;
    }
}

}  // namespace multihal
}  // namespace stm
}  // namespace sensors
}  // namespace hardware
}  // namespace android
