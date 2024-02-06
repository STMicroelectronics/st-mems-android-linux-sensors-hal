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
#include <IConsole.h>

#include "Convert.h"

namespace android {
namespace hardware {
namespace sensors {
namespace implementation {

using aidl::android::hardware::sensors::AdditionalInfo;
using aidl::android::hardware::sensors::DynamicSensorInfo;
using aidl::android::hardware::sensors::Event;
using aidl::android::hardware::sensors::ISensors;
using aidl::android::hardware::sensors::SensorInfo;
using aidl::android::hardware::sensors::SensorStatus;
using aidl::android::hardware::sensors::SensorType;

static bool convertFromSTMSensorType(const stm::core::SensorType &type,
                                     SensorType &sensorType,
                                     bool &isPartOfSensorList)
{
    isPartOfSensorList = true;

    switch (type) {
    case stm::core::SensorType::META_DATA:
        sensorType = SensorType::META_DATA;
        isPartOfSensorList = false;
        break;
    case stm::core::SensorType::ACCELEROMETER:
        sensorType = SensorType::ACCELEROMETER;
        break;
    case stm::core::SensorType::MAGNETOMETER:
        sensorType = SensorType::MAGNETIC_FIELD;
        break;
    case stm::core::SensorType::ORIENTATION:
        sensorType = SensorType::ORIENTATION;
        break;
    case stm::core::SensorType::GYROSCOPE:
        sensorType = SensorType::GYROSCOPE;
        break;
    case stm::core::SensorType::LIGHT:
        sensorType = SensorType::LIGHT;
        break;
    case stm::core::SensorType::PRESSURE:
        sensorType = SensorType::PRESSURE;
        break;
    case stm::core::SensorType::INTERNAL_TEMPERATURE:
        isPartOfSensorList = false;
        return false;
    case stm::core::SensorType::PROXIMITY:
        sensorType = SensorType::PROXIMITY;
        break;
    case stm::core::SensorType::GRAVITY:
        sensorType = SensorType::GRAVITY;
        break;
    case stm::core::SensorType::LINEAR_ACCELERATION:
        sensorType = SensorType::LINEAR_ACCELERATION;
        break;
    case stm::core::SensorType::ROTATION_VECTOR:
        sensorType = SensorType::ROTATION_VECTOR;
        break;
    case stm::core::SensorType::RELATIVE_HUMIDITY:
        sensorType = SensorType::RELATIVE_HUMIDITY;
        break;
    case stm::core::SensorType::AMBIENT_TEMPERATURE:
        sensorType = SensorType::AMBIENT_TEMPERATURE;
        break;
    case stm::core::SensorType::MAGNETOMETER_UNCALIBRATED:
        sensorType = SensorType::MAGNETIC_FIELD_UNCALIBRATED;
        break;
    case stm::core::SensorType::GAME_ROTATION_VECTOR:
        sensorType = SensorType::GAME_ROTATION_VECTOR;
        break;
    case stm::core::SensorType::GYROSCOPE_UNCALIBRATED:
        sensorType = SensorType::GYROSCOPE_UNCALIBRATED;
        break;
    case stm::core::SensorType::SIGNIFICANT_MOTION:
        sensorType = SensorType::SIGNIFICANT_MOTION;
        break;
    case stm::core::SensorType::STEP_DETECTOR:
        sensorType = SensorType::STEP_DETECTOR;
        break;
    case stm::core::SensorType::STEP_COUNTER:
        sensorType = SensorType::STEP_COUNTER;
        break;
    case stm::core::SensorType::GEOMAGNETIC_ROTATION_VECTOR:
        sensorType = SensorType::GEOMAGNETIC_ROTATION_VECTOR;
        break;
    case stm::core::SensorType::HEART_RATE:
        sensorType = SensorType::HEART_RATE;
        break;
    case stm::core::SensorType::TILT_DETECTOR:
        sensorType = SensorType::TILT_DETECTOR;
        break;
    case stm::core::SensorType::WAKE_GESTURE:
        sensorType = SensorType::WAKE_GESTURE;
        break;
    case stm::core::SensorType::GLANCE_GESTURE:
        sensorType = SensorType::GLANCE_GESTURE;
        break;
    case stm::core::SensorType::PICK_UP_GESTURE:
        sensorType = SensorType::PICK_UP_GESTURE;
        break;
    case stm::core::SensorType::WRIST_TILT_GESTURE:
        sensorType = SensorType::WRIST_TILT_GESTURE;
        break;
    case stm::core::SensorType::DEVICE_ORIENTATION:
        sensorType = SensorType::DEVICE_ORIENTATION;
        break;
    case stm::core::SensorType::POSE_6DOF:
        sensorType = SensorType::POSE_6DOF;
        break;
    case stm::core::SensorType::STATIONARY_DETECT:
        sensorType = SensorType::STATIONARY_DETECT;
        break;
    case stm::core::SensorType::MOTION_DETECT:
        sensorType = SensorType::MOTION_DETECT;
        break;
    case stm::core::SensorType::HEART_BEAT:
        sensorType = SensorType::HEART_BEAT;
        break;
    case stm::core::SensorType::DYNAMIC_SENSOR_META:
        sensorType = SensorType::DYNAMIC_SENSOR_META;
        isPartOfSensorList = false;
        break;
    case stm::core::SensorType::ADDITIONAL_INFO:
        sensorType = SensorType::ADDITIONAL_INFO;
        isPartOfSensorList = false;
        break;
    case stm::core::SensorType::LOW_LATENCY_OFFBODY_DETECT:
        sensorType = SensorType::LOW_LATENCY_OFFBODY_DETECT;
        break;
    case stm::core::SensorType::ACCELEROMETER_UNCALIBRATED:
        sensorType = SensorType::ACCELEROMETER_UNCALIBRATED;
        break;
    case stm::core::SensorType::ACCELEROMETER_LIMITED_AXES:
        sensorType = SensorType::ACCELEROMETER_LIMITED_AXES;
        break;
    case stm::core::SensorType::GYROSCOPE_LIMITED_AXES:
        sensorType = SensorType::GYROSCOPE_LIMITED_AXES;
        break;
    case stm::core::SensorType::ACCELEROMETER_LIMITED_AXES_UNCALIBRATED:
        sensorType = SensorType::ACCELEROMETER_LIMITED_AXES_UNCALIBRATED;
        break;
    case stm::core::SensorType::GYROSCOPE_LIMITED_AXES_UNCALIBRATED:
        sensorType = SensorType::GYROSCOPE_LIMITED_AXES_UNCALIBRATED;
        break;
    default:
        isPartOfSensorList = false;
        return false;
    }

    return true;
}

bool convertFromSTMSensorType(const stm::core::SensorType &type,
                              SensorType &sensorType)
{
    bool isPartOfSensorList;

    return convertFromSTMSensorType(type, sensorType, isPartOfSensorList);
}

static bool isOneShot(const SensorType &sensorType)
{
    switch (sensorType) {
    case SensorType::GLANCE_GESTURE:
    case SensorType::PICK_UP_GESTURE:
    case SensorType::SIGNIFICANT_MOTION:
    case SensorType::WAKE_GESTURE:
        return true;
    default:
        break;
    }

    return false;
}

static bool isSpecial(const SensorType &sensorType)
{
    switch (sensorType) {
    case SensorType::STEP_DETECTOR:
    case SensorType::TILT_DETECTOR:
        return true;
    default:
        break;
    }

    return false;
}

static int32_t convertFromHzToUs(float hz)
{
    if (hz < 1e-9)
       return 0;

    return 1e6 / hz;
}

/* convert from stm core STMSensor to AIDL SensorInfo */
bool convertFromSTMSensor(const stm::core::STMSensor &src,
                          SensorInfo *dst)
{
    SensorType sensorType;
    bool isPartOfSensorList;

    if (!convertFromSTMSensorType(src.getType(), sensorType, isPartOfSensorList) ||
        !isPartOfSensorList)
        return false;

    dst->sensorHandle = src.getHandle();
    dst->name = src.getName();
    dst->vendor = src.getVendor();
    dst->version = src.getVersion();
    dst->type = (SensorType)sensorType;
    dst->typeAsString = "";
    dst->maxRange = std::ceil(src.getMaxRange());
    dst->resolution = src.getResolution();
    dst->power = src.getPower();

    if (src.isOnChange()) {
        if (isOneShot(dst->type)) {
            dst->flags |= SensorInfo::SENSOR_FLAG_BITS_ONE_SHOT_MODE;
            dst->minDelayUs = -1;
            dst->maxDelayUs = 0;
        } else if (isSpecial(dst->type)) {
            dst->flags |= SensorInfo::SENSOR_FLAG_BITS_SPECIAL_REPORTING_MODE;
            dst->minDelayUs = 0;
            dst->maxDelayUs = 0;
        } else {
            dst->flags |= SensorInfo::SENSOR_FLAG_BITS_ON_CHANGE_MODE;
            dst->minDelayUs = 0;
            dst->maxDelayUs = convertFromHzToUs(src.getMinRateHz());
        }
    } else {
        dst->flags |= SensorInfo::SENSOR_FLAG_BITS_CONTINUOUS_MODE;
        dst->minDelayUs = convertFromHzToUs(src.getMaxRateHz());
        dst->maxDelayUs = convertFromHzToUs(src.getMinRateHz());
    }

    dst->fifoReservedEventCount = src.getFifoRsvdCount();
    dst->fifoMaxEventCount = src.getFifoMaxCount();
    dst->requiredPermission = "";

    if (src.isWakeUp())
        dst->flags |= SensorInfo::SENSOR_FLAG_BITS_WAKE_UP;

    if (HAL_ENABLE_DIRECT_REPORT_CHANNEL == 1) {
        if (!src.isOnChange()) {
            dst->flags |= SensorInfo::SENSOR_FLAG_BITS_DIRECT_CHANNEL_ASHMEM;
            // TODO add support for gralloc
            // dst->flags |= V1_0::SensorFlagBits::DIRECT_CHANNEL_GRALLOC;

            uint32_t directReportMaxRate = static_cast<int32_t>(ISensors::RateLevel::NORMAL);
            if (src.getMaxRateHz() > 180)
            {
                directReportMaxRate = static_cast<int32_t>(ISensors::RateLevel::FAST);
            }
            if (src.getMaxRateHz() > 780)
            {
                directReportMaxRate = static_cast<int32_t>(ISensors::RateLevel::VERY_FAST);
            }

            dst->flags |= (directReportMaxRate << static_cast<uint8_t>(SensorInfo::SENSOR_FLAG_SHIFT_DIRECT_REPORT));
        }
    }

    return true;
}

void convertFromSTMSensorData(const stm::core::ISTMSensorsCallbackData &sensorData,
                              Event &event)
{
    using stm::core::SensorType;

    switch (sensorData.getSensorType()) {
    case SensorType::ACCELEROMETER:
    case SensorType::MAGNETOMETER:
    case SensorType::ORIENTATION:
    case SensorType::GYROSCOPE:
    case SensorType::GRAVITY:
    case SensorType::LINEAR_ACCELERATION: {
        if (sensorData.getData().size() < 3) {
            return;
        }

        Event::EventPayload::Vec3 vec3;

        vec3.x = sensorData.getData().at(0);
        vec3.y = sensorData.getData().at(1);
        vec3.z = sensorData.getData().at(2);
        vec3.status = SensorStatus::ACCURACY_HIGH;

        event.payload.set<Event::EventPayload::Tag::vec3>(vec3);
        break;
    }
    case SensorType::GAME_ROTATION_VECTOR: {
        if (sensorData.getData().size() < 4)
            return;
 
        Event::EventPayload::Vec4 vec4;
        vec4.x = sensorData.getData().at(0);
        vec4.y = sensorData.getData().at(1);
        vec4.z = sensorData.getData().at(2);
        vec4.w = sensorData.getData().at(3);
        event.payload.set<Event::EventPayload::Tag::vec4>(vec4);
        break;
    }
    case SensorType::MAGNETOMETER_UNCALIBRATED:
    case SensorType::GYROSCOPE_UNCALIBRATED:
    case SensorType::ACCELEROMETER_UNCALIBRATED: {
        if (sensorData.getData().size() < 6)
            return;

        Event::EventPayload::Uncal uncal;
        uncal.x = sensorData.getData().at(0);
        uncal.y = sensorData.getData().at(1);
        uncal.z = sensorData.getData().at(2);
        uncal.xBias = sensorData.getData().at(3);
        uncal.yBias = sensorData.getData().at(4);
        uncal.zBias = sensorData.getData().at(5);
        event.payload.set<Event::EventPayload::Tag::uncal>(uncal);
        break;
    }
    case SensorType::STEP_COUNTER:
    case SensorType::STEP_DETECTOR: {
        if (sensorData.getData().size() < 1)
            return;

	event.payload.set<Event::EventPayload::Tag::stepCount>(sensorData.getData().at(0));
        break;
    }
    case SensorType::META_DATA: {
        using MetaDataEventType =
            ::aidl::android::hardware::sensors::Event::EventPayload::MetaData::MetaDataEventType;

        event.sensorHandle = (int32_t)sensorData.getSensorHandle();

        stm::core::IConsole& console = stm::core::IConsole::getInstance();
        console.error("Received META_DATA from handle " + std::to_string(event.sensorHandle));

        event.sensorType = ::aidl::android::hardware::sensors::SensorType::META_DATA;
        Event::EventPayload::MetaData meta = {
            .what = MetaDataEventType::META_DATA_FLUSH_COMPLETE,
        };
        event.payload.set<Event::EventPayload::meta>(meta);
        break;
    }
    case SensorType::ACCELEROMETER_LIMITED_AXES:
    case SensorType::GYROSCOPE_LIMITED_AXES: {
        Event::EventPayload::LimitedAxesImu limitedAxesImu;
        limitedAxesImu.x = sensorData.getData().at(0);
        limitedAxesImu.y = sensorData.getData().at(1);
        limitedAxesImu.z = sensorData.getData().at(2);
        /* in case of limited axes the event contains the supported state */
        limitedAxesImu.xSupported = sensorData.getData().at(4);
        limitedAxesImu.ySupported = sensorData.getData().at(5);
        limitedAxesImu.zSupported = sensorData.getData().at(6);
        event.payload.set<Event::EventPayload::Tag::limitedAxesImu>(limitedAxesImu);
        break;
        }
    case SensorType::ACCELEROMETER_LIMITED_AXES_UNCALIBRATED:
    case SensorType::GYROSCOPE_LIMITED_AXES_UNCALIBRATED: {
        Event::EventPayload::LimitedAxesImuUncal limitedAxesImuUncal;
        limitedAxesImuUncal.x = sensorData.getData().at(0);
        limitedAxesImuUncal.y = sensorData.getData().at(1);
        limitedAxesImuUncal.z = sensorData.getData().at(2);
        limitedAxesImuUncal.xBias = sensorData.getData().at(3);
        limitedAxesImuUncal.yBias = sensorData.getData().at(4);
        limitedAxesImuUncal.zBias = sensorData.getData().at(5);
        /* in case of limited axes the event contains the supported state */
        limitedAxesImuUncal.xSupported = sensorData.getData().at(6);
        limitedAxesImuUncal.ySupported = sensorData.getData().at(7);
        limitedAxesImuUncal.zSupported = sensorData.getData().at(8);
        event.payload.set<Event::EventPayload::Tag::limitedAxesImuUncal>(limitedAxesImuUncal);
        break;
        }
    default:
        break;
    }
}

} // namespace implementation
} // namespace sensors
} // namespace hardware
} // namespace android
