/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019 STMicroelectronics
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

/**
 * STM sensors core types
 */
enum class STMSensorType : int16_t {
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
    INTERNAL_LIB_TYPE,              /* INTERNAL USE ONLY */
};
