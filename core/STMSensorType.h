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

#pragma once

#include <SensorType.h>
#include "SensorTypeInternal.h"

namespace stm {
namespace core {

class STMSensorType {
public:
    STMSensorType(void) = delete;
    STMSensorType(SensorType type);
    STMSensorType(SensorTypeInternal type);

    bool isInternal(void) const;

    operator SensorType() const { return sType; };
    operator SensorTypeInternal() const { return sTypeInt; };

    bool operator==(const STMSensorType &other) const;
    bool operator!=(const STMSensorType &other) const;

private:
    const bool isInternalType;

    union {
        SensorType sType;
        SensorTypeInternal sTypeInt;
    };
};

const STMSensorType AccelSensorType(SensorType::ACCELEROMETER);
const STMSensorType MagnSensorType(SensorType::MAGNETOMETER);
const STMSensorType OrientationSensorType(SensorType::ORIENTATION);
const STMSensorType GyroSensorType(SensorType::GYROSCOPE);
const STMSensorType LigthSensorType(SensorType::LIGHT);
const STMSensorType PressureSensorType(SensorType::PRESSURE);
const STMSensorType IntTemperatureSensorType(SensorType::INTERNAL_TEMPERATURE);
const STMSensorType ProximitySensorType(SensorType::PROXIMITY);
const STMSensorType GravitySensorType(SensorType::GRAVITY);
const STMSensorType LinearAccelSensorType(SensorType::LINEAR_ACCELERATION);
const STMSensorType RotationVecSensorType(SensorType::ROTATION_VECTOR);
const STMSensorType HumiditySensorType(SensorType::RELATIVE_HUMIDITY);
const STMSensorType AmbTemperatureSensorType(SensorType::AMBIENT_TEMPERATURE);
const STMSensorType MagnUncalibSensorType(SensorType::MAGNETOMETER_UNCALIBRATED);
const STMSensorType GameRotationVecSensorType(SensorType::GAME_ROTATION_VECTOR);
const STMSensorType GyroUncalibSensorType(SensorType::GYROSCOPE_UNCALIBRATED);
const STMSensorType SignMotionSensorType(SensorType::SIGNIFICANT_MOTION);
const STMSensorType StepDetectorSensorType(SensorType::STEP_DETECTOR);
const STMSensorType StepCounterSensorType(SensorType::STEP_COUNTER);
const STMSensorType GeoRotationVecSensorType(SensorType::GEOMAGNETIC_ROTATION_VECTOR);
const STMSensorType HeartRateSensorType(SensorType::HEART_RATE);
const STMSensorType TiltDetectorSensorType(SensorType::TILT_DETECTOR);
const STMSensorType WakeGestureSensorType(SensorType::WAKE_GESTURE);
const STMSensorType GlaceGestureSensorType(SensorType::GLANCE_GESTURE);
const STMSensorType PickupGestureSensorType(SensorType::PICK_UP_GESTURE);
const STMSensorType WristTiltGestureSensorType(SensorType::WRIST_TILT_GESTURE);
const STMSensorType DeviceOrientationSensorType(SensorType::DEVICE_ORIENTATION);
const STMSensorType Pose6DoFSensorType(SensorType::POSE_6DOF);
const STMSensorType StationaryDetectSensorType(SensorType::STATIONARY_DETECT);
const STMSensorType MotionDetectSensorType(SensorType::MOTION_DETECT);
const STMSensorType HeartBeatSensorType(SensorType::HEART_BEAT);
const STMSensorType DynamicMetaSensorType(SensorType::DYNAMIC_SENSOR_META);
const STMSensorType AdditionalInfoSensorType(SensorType::ADDITIONAL_INFO);
const STMSensorType LowLatencyOffbodyDetectSensorType(SensorType::LOW_LATENCY_OFFBODY_DETECT);
const STMSensorType AccelUncalibSensorType(SensorType::ACCELEROMETER_UNCALIBRATED);

const STMSensorType AccelGyroFusion6XSensorType(SensorTypeInternal::ACCEL_GYRO_FUSION_6X);
const STMSensorType AccelMagnGyroFusion9XSensorType(SensorTypeInternal::ACCEL_MAGN_GYRO_FUSION_9X);

} // namespace core
} // namespace stm
