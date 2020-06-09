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
    SENSOR_HAL_NORMAL_MODE = 0,
    SENSOR_HAL_DATA_INJECTION_MODE = 1,
};

enum {
    SENSOR_STATUS_NO_CONTACT = -1 /* (-1) */,
    SENSOR_STATUS_UNRELIABLE = 0,
    SENSOR_STATUS_ACCURACY_LOW = 1,
    SENSOR_STATUS_ACCURACY_MEDIUM = 2,
    SENSOR_STATUS_ACCURACY_HIGH = 3,
};

typedef enum {
    AINFO_BEGIN = 0u,
    AINFO_END = 1u,
    AINFO_UNTRACKED_DELAY = 65536u /* 0x10000 */,
    AINFO_INTERNAL_TEMPERATURE = 65537u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_UNTRACKED_DELAY implicitly + 1) */,
    AINFO_VEC3_CALIBRATION = 65538u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_INTERNAL_TEMPERATURE implicitly + 1) */,
    AINFO_SENSOR_PLACEMENT = 65539u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_VEC3_CALIBRATION implicitly + 1) */,
    AINFO_SAMPLING = 65540u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_SENSOR_PLACEMENT implicitly + 1) */,
    AINFO_CHANNEL_NOISE = 131072u /* 0x20000 */,
    AINFO_CHANNEL_SAMPLER = 131073u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_CHANNEL_NOISE implicitly + 1) */,
    AINFO_CHANNEL_FILTER = 131074u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_CHANNEL_SAMPLER implicitly + 1) */,
    AINFO_CHANNEL_LINEAR_TRANSFORM = 131075u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_CHANNEL_FILTER implicitly + 1) */,
    AINFO_CHANNEL_NONLINEAR_MAP = 131076u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_CHANNEL_LINEAR_TRANSFORM implicitly + 1) */,
    AINFO_CHANNEL_RESAMPLER = 131077u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_CHANNEL_NONLINEAR_MAP implicitly + 1) */,
    AINFO_LOCAL_GEOMAGNETIC_FIELD = 196608u /* 0x30000 */,
    AINFO_LOCAL_GRAVITY = 196609u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_LOCAL_GEOMAGNETIC_FIELD implicitly + 1) */,
    AINFO_DOCK_STATE = 196610u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_LOCAL_GRAVITY implicitly + 1) */,
    AINFO_HIGH_PERFORMANCE_MODE = 196611u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_DOCK_STATE implicitly + 1) */,
    AINFO_MAGNETIC_FIELD_CALIBRATION = 196612u /* (::android::hardware::sensors::V1_0::AdditionalInfoType.AINFO_HIGH_PERFORMANCE_MODE implicitly + 1) */,
    AINFO_CUSTOM_START = 268435456u /* 0x10000000 */,
    AINFO_DEBUGGING_START = 1073741824u /* 0x40000000 */,
} additional_info_type_t;

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
    const char  *vendor;
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

typedef struct {
    union {
        float v[3];
        struct Triaxial xyz;
        struct TriaxialEuler hpr;
    };
    int8_t status;
    uint8_t reserved[3];
} sensors_vec_t;

typedef struct {
  union {
    float uncalib[3];
    struct TriaxialUn xyz_un;
  };
  union {
    float bias[3];
    struct TriaxialBias xyz_bias;
  };
} uncalibrated_event_t;

typedef struct meta_data_event {
    int32_t what;
    int32_t sensor;
} meta_data_event_t;

/**
 * Dynamic sensor meta event. See the description of SENSOR_TYPE_DYNAMIC_SENSOR_META type for
 * details.
 */
typedef struct dynamic_sensor_meta_event {
    int32_t  connected;
    int32_t  handle;
    const struct sensor_t * sensor; // should be NULL if connected == false
    uint8_t uuid[16];               // UUID of a dynamic sensor (using RFC 4122 byte order)
                                    // For UUID 12345678-90AB-CDEF-1122-334455667788 the uuid field
                                    // should be initialized as:
                                    // {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x11, ...}
} dynamic_sensor_meta_event_t;

typedef struct {
    int32_t type;                           // type of payload data, see additional_info_type_t
    int32_t serial;                         // sequence number of this frame for this type
    union {
        // for each frame, a single data type, either int32_t or float, should be used.
        int32_t data_int32[14];
        float   data_float[14];
    };
} additional_info_event_t;

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
