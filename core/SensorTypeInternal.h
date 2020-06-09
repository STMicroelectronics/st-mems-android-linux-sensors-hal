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

namespace stm {
namespace core {

/**
 * Sensors core internal types
 */
enum class SensorTypeInternal : uint16_t {
    ACCEL_GYRO_FUSION_6X,           /* ACCEL AND GYRO FUSION */
    ACCEL_MAGN_GYRO_FUSION_9X,      /* ACCEL, MAGN AND GYRO FUSION */
};

} // namespace core
} // namespace stm
