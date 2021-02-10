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

#include "STMSensorsFusion9Axis.h"

STMSensorsFusion9Axis& STMSensorsFusion9Axis::getInstance(void)
{
    static STMSensorsFusion9Axis *fusionLib = new STMSensorsFusion9Axis();
    return *fusionLib;
}

int STMSensorsFusion9Axis::init(void)
{
    return 0;
}

int STMSensorsFusion9Axis::reset(void *data)
{
    (void) data;

    return 0;
}

int STMSensorsFusion9Axis::run(const std::array<float, 3> &accelData,
                               const std::array<float, 3> &magnData,
                               const std::array<float, 3> &gyroData,
                               int64_t timestamp)
{
    (void) accelData;
    (void) magnData;
    (void) gyroData;
    (void) timestamp;

    return 0;
}

const std::string& STMSensorsFusion9Axis::getLibVersion(void) const
{
    static const std::string libVersion("stm-sensors-fusion-9X-lib-mock");

    return libVersion;
}
