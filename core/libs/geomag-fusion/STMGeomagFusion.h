/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2021 STMicroelectronics
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
#include <string>

#define GRAVITY_EARTH				(9.80665f)
#define LOCAL_EARTH_MAGNETIC_FIELD		(50.0f)
#define DEG2RAD(deg)				(deg * M_PI / 180.0f)

struct STMGeomagFusion {
    static STMGeomagFusion& getInstance(void);
    STMGeomagFusion(const STMGeomagFusion &) = delete;
    STMGeomagFusion(STMGeomagFusion &&) = delete;
    STMGeomagFusion& operator=(const STMGeomagFusion &) = delete;
    STMGeomagFusion& operator=(STMGeomagFusion &&) = delete;
    STMGeomagFusion(void) = default;
    ~STMGeomagFusion(void) = default;

    int init(float freq);

    int run(const std::array<float, 3> &accelData,
            const std::array<float, 3> &magData,
            int64_t delay);

    int getQuaternion(std::array<float, 4> &data) const;

    int getEulerAngles(std::array<float, 3> &data) const;

    int getGravity(std::array<float, 3> &data) const;

    int getLinearAccel(std::array<float, 3> &data) const;

    const std::string& getLibVersion(void) const;
};
