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

#include "STMSensorsFusion.h"

struct STMSensorsFusion9Axis: public STMSensorsFusion {
    static STMSensorsFusion9Axis& getInstance(void);
    virtual ~STMSensorsFusion9Axis(void) = default;
    STMSensorsFusion9Axis(const STMSensorsFusion9Axis &) = delete;
    STMSensorsFusion9Axis(STMSensorsFusion9Axis &&) = delete;
    STMSensorsFusion9Axis& operator=(const STMSensorsFusion9Axis &) = delete;
    STMSensorsFusion9Axis& operator=(STMSensorsFusion9Axis &&) = delete;

    virtual int init(void) override;

    virtual int reset(void *data) override;

    int run(const std::array<float, 3> &accelData,
            const std::array<float, 3> &magnData,
            const std::array<float, 3> &gyroData,
            int64_t timestamp);

    virtual const std::string& getLibVersion(void) const override;

private:
    STMSensorsFusion9Axis(void) = default;
};
