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

struct STMSensorsFusion {
    STMSensorsFusion(const STMSensorsFusion &) = delete;
    STMSensorsFusion(STMSensorsFusion &&) = delete;
    STMSensorsFusion& operator=(const STMSensorsFusion &) = delete;
    STMSensorsFusion& operator=(STMSensorsFusion &&) = delete;

    virtual int init(void) = 0;

    virtual int reset(void *data) = 0;

    int getQuaternion(std::array<float, 4> &data) const;

    int getEulerAngles(std::array<float, 3> &data) const;

    int getGravity(std::array<float, 3> &data) const;

    int getLinearAccel(std::array<float, 3> &data) const;

    virtual const std::string& getLibVersion(void) const = 0;

protected:
    STMSensorsFusion(void) = default;
    virtual ~STMSensorsFusion(void) = default;
};
