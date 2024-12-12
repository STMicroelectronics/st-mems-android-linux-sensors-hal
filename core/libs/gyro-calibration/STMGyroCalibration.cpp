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

#include "STMGyroCalibration.h"
#include <Utils.h>

static const int minFrequencyHz = 1;
static const int maxFrequencyHz = 200;

STMGyroCalibration& STMGyroCalibration::getInstance(void)
{
    static STMGyroCalibration *gyroCalib = new STMGyroCalibration();
    return *gyroCalib;
}

STMGyroCalibration::STMGyroCalibration(void)
    : lastTimestamp(1)
{
}

int STMGyroCalibration::init(float accelThreshold,
                             float gyroThreshold,
                             float accelRange,
                             float gyroRange)
{
    if ((accelRange < 1.0f) || (gyroRange < 1.0f) ||
        (accelThreshold < 1e-6) || (gyroThreshold < 1e-6)) {
        return -1;
    }

    return 0;
}

int STMGyroCalibration::reset(const Matrix<4, 3, float> &initialBias)
{
    outBias = initialBias;
    lastTimestamp = 1;

    return 0;
}

int STMGyroCalibration::setFrequency(int frequencyHz)
{
    if ((frequencyHz < STMGyroCalibration::getMinFrequencyHz()) ||
        (frequencyHz > STMGyroCalibration::getMaxFrequencyHz())) {
        return -1;
    }

    return 0;
}

int STMGyroCalibration::run(const std::array<float, 3> &accelData,
                            const std::array<float, 3> &gyroData,
                            int64_t timestamp)
{
    if (timestamp <= lastTimestamp) return -1;

    (void) accelData;
    (void) gyroData;
    lastTimestamp = timestamp;

    return 0;
}

int STMGyroCalibration::getBias(Matrix<4, 3, float> &bias) const
{
    bias = outBias;

    return 0;
}

const std::string& STMGyroCalibration::getLibVersion(void) const
{
    static const std::string libVersion("stm-gyro-lib-mock");

    return libVersion;
}

void STMGyroCalibration::resetBiasMatrix(Matrix<4, 3, float> &bias)
{
    for (auto i = 0; i < bias.numRows(); ++i) {
        for (auto j = 0; j < bias.numColumns(); ++j) {
            if (i == j) {
                bias[i][j] = 1.0f;
            } else {
                bias[i][j] = 0.0f;
            }
        }
    }
}

int STMGyroCalibration::getMinFrequencyHz(void) { return minFrequencyHz; }

int STMGyroCalibration::getMaxFrequencyHz(void) { return maxFrequencyHz; }
