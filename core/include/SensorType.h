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

#include <cstdint>
#include <string>
#include <unordered_map>

namespace stm {
namespace core {

/**
 * Sensors core types
 */
enum class SensorType : uint16_t {
    META_DATA,                      /* dimensionless */
    ACCELEROMETER,                  /* m/s^2 */
    MAGNETOMETER,                   /* uT */
    ORIENTATION,                    /* degrees */
    GYROSCOPE,                      /* rad/s */
    LIGHT,                          /* lux */
    PRESSURE,                       /* millibar */
    INTERNAL_TEMPERATURE,           /* Celsius degrees */
    PROXIMITY,                      /* millimeters */
    GRAVITY,                        /* m/s^2 */
    LINEAR_ACCELERATION,            /* m/s^2 */
    ROTATION_VECTOR,                /* dimensionless */
    RELATIVE_HUMIDITY,              /* % */
    AMBIENT_TEMPERATURE,            /* Celsius degrees */
    MAGNETOMETER_UNCALIBRATED,      /* uT */
    GAME_ROTATION_VECTOR,           /* dimensionless */
    GYROSCOPE_UNCALIBRATED,         /* rad/s */
    SIGNIFICANT_MOTION,             /* dimensionless */
    STEP_DETECTOR,                  /* dimensionless */
    STEP_COUNTER,                   /* dimensionless */
    GEOMAGNETIC_ROTATION_VECTOR,    /* dimensionless */
    HEART_RATE,                     /* BPM */
    TILT_DETECTOR,                  /* dimensionless */
    WAKE_GESTURE,                   /* dimensionless */
    GLANCE_GESTURE,                 /* dimensionless */
    PICK_UP_GESTURE,                /* dimensionless */
    WRIST_TILT_GESTURE,             /* dimensionless */
    DEVICE_ORIENTATION,             /* dimensionless */
    POSE_6DOF,                      /* dimensionless */
    STATIONARY_DETECT,              /* dimensionless */
    MOTION_DETECT,                  /* dimensionless */
    HEART_BEAT,                     /* dimensionless */
    DYNAMIC_SENSOR_META,            /* dimensionless */
    ADDITIONAL_INFO,                /* dimensionless */
    LOW_LATENCY_OFFBODY_DETECT,     /* dimensionless */
    ACCELEROMETER_UNCALIBRATED,     /* m/s^2 */
    ODR_SWITCH_INFO,                /* dimensionless */
    ACCELEROMETER_LIMITED_AXES,     /* m/s^2 */
    GYROSCOPE_LIMITED_AXES,         /* rad/s */
    ACCELEROMETER_LIMITED_AXES_UNCALIBRATED,     /* m/s^2 */
    GYROSCOPE_LIMITED_AXES_UNCALIBRATED,         /* rad/s */
};

using SensorHandle = uint32_t;

const std::unordered_map<SensorType, std::string> sensorTypeToString = {
    { SensorType::META_DATA, "meta-data"},
    { SensorType::ACCELEROMETER, "accelerometer"},
    { SensorType::MAGNETOMETER, "magnetometer"},
    { SensorType::ORIENTATION, "orientation"},
    { SensorType::GYROSCOPE, "gyroscope"},
    { SensorType::LIGHT, "light"},
    { SensorType::PRESSURE, "pressure"},
    { SensorType::INTERNAL_TEMPERATURE, "internal temperature"},
    { SensorType::PROXIMITY, "proximity"},
    { SensorType::GRAVITY, "gravity"},
    { SensorType::LINEAR_ACCELERATION, "linear acceleration"},
    { SensorType::ROTATION_VECTOR, "rotation vector"},
    { SensorType::RELATIVE_HUMIDITY, "relative humidity"},
    { SensorType::AMBIENT_TEMPERATURE, "ambient temperature"},
    { SensorType::MAGNETOMETER_UNCALIBRATED, "magnetometer uncalibrated"},
    { SensorType::GAME_ROTATION_VECTOR, "game rotation vector"},
    { SensorType::GYROSCOPE_UNCALIBRATED, "gyroscope uncalibrated"},
    { SensorType::SIGNIFICANT_MOTION, "significant motion"},
    { SensorType::STEP_DETECTOR, "step detector"},
    { SensorType::STEP_COUNTER, "step counter"},
    { SensorType::GEOMAGNETIC_ROTATION_VECTOR, "geomagnetic rotation vector"},
    { SensorType::ACCELEROMETER_UNCALIBRATED, "accelerometer uncalibrated"},
    { SensorType::ACCELEROMETER_LIMITED_AXES, "accelerometer limited axes"},
    { SensorType::GYROSCOPE_LIMITED_AXES, "gyroscope limited axes"},
    { SensorType::ACCELEROMETER_LIMITED_AXES_UNCALIBRATED, "accelerometer limited axes uncalibrated"},
    { SensorType::GYROSCOPE_LIMITED_AXES_UNCALIBRATED, "gyroscope limited axes uncalibrated"},
};

} // namespace core
} // namespace stm
