/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2020 STMicroelectronics
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

#include <cmath>

#include <IConsole.h>
#include "SensorPlacement.h"

using namespace stm::core;

SensorPlacement::SensorPlacement(void)
    : data({ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f })
{

}

/**
 * getPayload: return the payload of the sensor placement (4x3 matrix)
 *
 * Return value: array of 12 elements representing the 4x3 matrix.
 */
   const std::array<float, 12> &SensorPlacement::getPayload(void) const
{
    return data;
}

/**
 * loadFromProp: load the sensor placement data from system properties
 * @sensorType: sensor type.
 */
void SensorPlacement::loadFromProp(stm::core::SensorType sensorType,
                                   stm::core::SensorHandle sensorHandle)
{
    stm::core::PropertiesManager& propertiesManager =
        stm::core::PropertiesManager::getInstance();
    std::array<float, 3> sensorPosition;
    Matrix<3, 3, float> rotMatrix;

    switch (sensorType) {
    case SensorType::ACCELEROMETER:
    case SensorType::ACCELEROMETER_UNCALIBRATED:
    case SensorType::GRAVITY:
    case SensorType::LINEAR_ACCELERATION:
    case SensorType::MAGNETOMETER:
    case SensorType::MAGNETOMETER_UNCALIBRATED:
    case SensorType::GEOMAGNETIC_ROTATION_VECTOR:
    case SensorType::GYROSCOPE:
    case SensorType::GYROSCOPE_UNCALIBRATED:
    case SensorType::ORIENTATION:
    case SensorType::ROTATION_VECTOR:
    case SensorType::GAME_ROTATION_VECTOR:
    case SensorType::PRESSURE:
        rotMatrix = propertiesManager.getRotationMatrix(sensorHandle);
        sensorPosition = propertiesManager.getSensorPlacement(sensorHandle);
        break;
    default:
        return;
    }

    if (!invertRotationMatrix(rotMatrix)) {
        stm::core::IConsole& console = stm::core::IConsole::getInstance();
        console.error("failed to invert the rotation matrix, check input matrix!");
        return;
    }

    int counter = 0;
    for (auto row = 0U; row < 3; ++row) {
        for (auto col = 0U; col < 3; ++col) {
            data[counter++] = rotMatrix[row][col];
        }
        data[counter++] = sensorPosition[row];
    }
}

bool SensorPlacement::invertRotationMatrix(Matrix<3, 3, float>& matrix)
{
    Matrix<3, 3, float> invMatrix;

    invMatrix[0][0] = matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1];
    invMatrix[1][0] = -(matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]);
    invMatrix[2][0] = matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0];

    float determinant = matrix[0][0] * invMatrix[0][0] +
                        matrix[0][1] * invMatrix[1][0] +
                        matrix[0][2] * invMatrix[2][0];
    if (determinant < 1e-6) {
        return false;
    }

    invMatrix[1][1] = matrix[2][2] * matrix[0][0] - matrix[2][0] * matrix[0][2];
    invMatrix[2][1] = -(matrix[2][1] * matrix[0][0] - matrix[2][0] * matrix[0][1]);
    invMatrix[2][2] = matrix[1][1] * matrix[0][0] - matrix[1][0] * matrix[0][1];

    if ((std::fabs(matrix[0][1] - matrix[1][0]) < 1e-6) &&
        (std::fabs(matrix[0][2] - matrix[2][0]) < 1e-6) &&
        (std::fabs(matrix[1][2] - matrix[2][1]) < 1e-6)) {
        invMatrix[0][1] = invMatrix[1][0];
        invMatrix[0][2] = invMatrix[2][0];
        invMatrix[1][2] = invMatrix[2][1];
    } else {
        invMatrix[0][1] = -(matrix[2][2] * matrix[0][1] - matrix[2][1] * matrix[0][2]);
        invMatrix[0][2] = matrix[1][2] * matrix[0][1] - matrix[1][1] * matrix[0][2];
        invMatrix[1][2] = -(matrix[1][2] * matrix[0][0] - matrix[1][0] * matrix[0][2]);
    }

    for (auto row = 0; row < 3; ++row) {
        for (auto col = 0; col < 3; ++col) {
            invMatrix[row][col] /= determinant;
        }
    }

    matrix = invMatrix;

    return true;
}
