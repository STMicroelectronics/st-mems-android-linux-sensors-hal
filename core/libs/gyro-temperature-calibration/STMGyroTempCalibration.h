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

struct STMGyroTempCalibration {
    static STMGyroTempCalibration& getInstance(void);
    ~STMGyroTempCalibration(void);
    STMGyroTempCalibration(const STMGyroTempCalibration &) = delete;
    STMGyroTempCalibration(STMGyroTempCalibration &&) = delete;
    STMGyroTempCalibration& operator=(const STMGyroTempCalibration &) = delete;
    STMGyroTempCalibration& operator=(STMGyroTempCalibration &&) = delete;

    int initialize(void);

    int run(std::array<float, 3> &gyroData,
            float temperature,
            uint64_t timestamp,
            int *b_update);
    int getBias(float *temp, std::array<float, 3> &bias);
    int getState(void *state);
    int setState(void *state);
    const std::string& getLibVersion(void);
    static const int STMGyroTempCalibrationStateSize = 40;

private:
    std::array<float, 3> outBias;
    int bias_update;
    uint64_t lastUpdateTime;
    STMGyroTempCalibration(void);
};
