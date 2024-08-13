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

#include <array>
#include <vector>
#include <cstdint>
#include <string>

#include <STMSensorType.h>

enum {
    SENSOR_STATUS_NO_CONTACT = -1 /* (-1) */,
    SENSOR_STATUS_UNRELIABLE = 0,
    SENSOR_STATUS_ACCURACY_LOW = 1,
    SENSOR_STATUS_ACCURACY_MEDIUM = 2,
    SENSOR_STATUS_ACCURACY_HIGH = 3,
};

struct sensor_t {
    sensor_t(const stm::core::STMSensorType &type2) : type(type2) {};
    sensor_t(const struct sensor_t &data) = default;

    stm::core::STMSensorType type;
    float resolution;
    float maxRange;
    float power;
    float fifoRsvdCount;
    float fifoMaxEventCount;
    float minRateHz;
    float maxRateHz;
    int32_t handle;
    const char *name;
    const char *vendor;
    int moduleId;
};

struct Triaxial {
    float x;
    float y;
    float z;
};

struct TriaxialUn {
    float x_un;
    float y_un;
    float z_un;
};

struct TriaxialBias {
    float x_bias;
    float y_bias;
    float z_bias;
};

struct TriaxialEuler {
    float azimuth;
    float pitch;
    float roll;
};

struct u64 {
    uint64_t        data[8];

    /* step-counter */
    uint64_t        step_counter;
};

struct sensors_event_t {
    /* must be sizeof(struct sensors_event_t) */
    int32_t version;

    /* sensor identifier */
    int32_t sensor;

    /* sensor type */
    stm::core::SensorType type;

    /* reserved */
    int32_t reserved0;

    /* time is in nanosecond */
    int64_t timestamp;

    struct data {
        float data2[16];
        unsigned int dataLen = 0;
    } data;

    struct u64 u64;

    /* Reserved flags for internal use. Set to zero. */
    uint32_t flags;

    uint32_t reserved1[3];
};
