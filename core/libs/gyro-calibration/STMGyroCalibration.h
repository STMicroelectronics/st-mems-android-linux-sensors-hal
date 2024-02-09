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
#include <Matrix.h>

struct STMGyroCalibration {
    static STMGyroCalibration& getInstance(void);
    ~STMGyroCalibration(void) = default;
    STMGyroCalibration(const STMGyroCalibration &) = delete;
    STMGyroCalibration(STMGyroCalibration &&) = delete;
    STMGyroCalibration& operator=(const STMGyroCalibration &) = delete;
    STMGyroCalibration& operator=(STMGyroCalibration &&) = delete;

    int init(float accelThreshold,
             float gyroThreshold,
             float accelRange,
             float gyroRange);

    int reset(const Matrix<4, 3, float> &initialBias);

    int setFrequency(int frequencyHz);

    int run(const std::array<float, 3> &accelData,
            const std::array<float, 3> &gyroData,
            int64_t timestamp);

    int getBias(Matrix<4, 3, float> &bias) const;

    const std::string& getLibVersion(void) const;

    static void resetBiasMatrix(Matrix<4, 3, float> &bias);

    static int getMinFrequencyHz(void);

    static int getMaxFrequencyHz(void);

private:
    Matrix<4, 3, float> outBias;
    int64_t lastTimestamp;

    STMGyroCalibration(void);
};
