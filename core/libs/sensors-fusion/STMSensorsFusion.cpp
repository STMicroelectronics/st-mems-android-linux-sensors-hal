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

#include "STMSensorsFusion.h"

int STMSensorsFusion::getQuaternion(std::array<float, 4> &data) const
{
    (void) data;

    return 0;
}

int STMSensorsFusion::getEulerAngles(std::array<float, 3> &data) const
{
    (void) data;

    return 0;
}

int STMSensorsFusion::getGravity(std::array<float, 3> &data) const
{
    (void) data;

    return 0;
}

int STMSensorsFusion::getLinearAccel(std::array<float, 3> &data) const
{
    (void) data;

    return 0;
}
