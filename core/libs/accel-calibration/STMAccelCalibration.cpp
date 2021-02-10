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

#include "STMAccelCalibration.h"

static const int minFrequencyHz = 1;
static const int maxFrequencyHz = 3000;

STMAccelCalibration& STMAccelCalibration::getInstance(void)
{
    static STMAccelCalibration *accelCalib = new STMAccelCalibration();
    return *accelCalib;
}

STMAccelCalibration::STMAccelCalibration(void)
    : lastTimestamp(1)
{
}

int STMAccelCalibration::init(float accelRange)
{
    if (accelRange < 1.0f) return -1;

    return 0;
}

int STMAccelCalibration::reset(const Matrix<3, 4, float> &initialBias)
{
    outBias = initialBias;
    lastTimestamp = 1;

    return 0;
}

int STMAccelCalibration::setFrequency(int frequencyHz)
{
    if ((frequencyHz < STMAccelCalibration::getMinFrequencyHz()) ||
        (frequencyHz > STMAccelCalibration::getMaxFrequencyHz())) {
        return -1;
    }

    return 0;
}

int STMAccelCalibration::run(const std::array<float, 3> &accelData,
                             int64_t timestamp)
{
    if (timestamp <= lastTimestamp) return -1;

    (void) accelData;

    lastTimestamp = timestamp;

    return 0;
}

int STMAccelCalibration::getBias(Matrix<3, 4, float> &bias) const
{
    bias = outBias;

    return 0;
}

const std::string& STMAccelCalibration::getLibVersion(void) const
{
    static const std::string libVersion("stm-accel-lib-mock");

    return libVersion;
}

void STMAccelCalibration::resetBiasMatrix(Matrix<3, 4, float> &bias)
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

int STMAccelCalibration::getMinFrequencyHz(void) { return minFrequencyHz; }

int STMAccelCalibration::getMaxFrequencyHz(void) { return maxFrequencyHz; }
