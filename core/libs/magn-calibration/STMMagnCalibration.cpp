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

#include "STMMagnCalibration.h"

static const int minFrequencyHz = 17;
static const int maxFrequencyHz = 100;

STMMagnCalibration& STMMagnCalibration::getInstance(void)
{
    static STMMagnCalibration *magnCalib = new STMMagnCalibration();
    return *magnCalib;
}

STMMagnCalibration::STMMagnCalibration(void)
    : lastTimestamp(1)
{
}

int STMMagnCalibration::init(float magnRange)
{
    if (magnRange < 1.0f) return -1;

    return 0;
}

int STMMagnCalibration::reset(const Matrix<4, 3, float> &initialBias)
{
    outBias = initialBias;
    lastTimestamp = 1;

    return 0;
}

int STMMagnCalibration::setFrequency(int frequencyHz)
{
    if ((frequencyHz < STMMagnCalibration::getMinFrequencyHz()) ||
        (frequencyHz > STMMagnCalibration::getMaxFrequencyHz())) {
        return -1;
    }

    return 0;
}

int STMMagnCalibration::run(const std::array<float, 3> &magnData,
                            int64_t timestamp)
{
    if (timestamp <= lastTimestamp) return -1;

    (void) magnData;
    lastTimestamp = timestamp;

    return 0;
}

int STMMagnCalibration::getBias(Matrix<4, 3, float> &bias) const
{
    bias = outBias;

    return 0;
}

const std::string& STMMagnCalibration::getLibVersion(void) const
{
    static const std::string libVersion("stm-magn-lib-mock");

    return libVersion;
}

void STMMagnCalibration::resetBiasMatrix(Matrix<4, 3, float> &bias)
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

int STMMagnCalibration::getMinFrequencyHz(void) { return minFrequencyHz; }

int STMMagnCalibration::getMaxFrequencyHz(void) { return maxFrequencyHz; }
