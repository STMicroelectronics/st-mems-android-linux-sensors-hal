/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2024 STMicroelectronics
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

#include "STMGyroTempCalibration.h"

STMGyroTempCalibration& STMGyroTempCalibration::getInstance(void)
{
    static STMGyroTempCalibration *gyroTempCalib = new STMGyroTempCalibration();
    return *gyroTempCalib;
}

STMGyroTempCalibration::STMGyroTempCalibration(void)
{
}

int STMGyroTempCalibration::initialize(void)
{
    outBias = { 0 };
    bias_update = 0;

    return 0;
}

int STMGyroTempCalibration::run(std::array<float, 3> &gyroData,
                                float temperature,
                                uint64_t timestamp,
                                int *b_update)
{
    (void) gyroData;
    (void) temperature;

    lastUpdateTime = timestamp;
    *b_update = bias_update;

    return 0;
}

int STMGyroTempCalibration::getBias(float *temp, std::array<float, 3> &bias)
{
    (void)temp;
    bias = outBias;

    return 0;
}

int STMGyroTempCalibration::getState(void *state)
{
  (void)state;

  return 0;
}

int STMGyroTempCalibration::setState(void *state)
{
  (void)state;

  return 0;
}

const std::string& STMGyroTempCalibration::getLibVersion(void)
{
    static const std::string libVersion("stm-motion-gt-lib-mock");

    return libVersion;
}
