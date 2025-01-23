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

#include "STMGeomagFusion.h"

STMGeomagFusion& STMGeomagFusion::getInstance(void)
{
    static STMGeomagFusion *fusionLib = new STMGeomagFusion();

    return *fusionLib;
}

int STMGeomagFusion::init(float freq)
{
    (void)freq;

    return 0;
}

int STMGeomagFusion::run(const std::array<float, 3> &accelData,
                         const std::array<float, 3> &magData,
                         int64_t delay)
{
    (void)accelData;
    (void)magData;
    (void)delay;

    return 0;
}

const std::string& STMGeomagFusion::getLibVersion(void) const
{
    static const std::string libVersion("stm-geomag-fusion-lib-mock");

    return libVersion;
}

int STMGeomagFusion::getQuaternion(std::array<float, 4> &data) const
{
    (void)data;

    return 0;
}
