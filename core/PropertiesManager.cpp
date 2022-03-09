/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2022 STMicroelectronics
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

#include <vector>

#include "PropertiesParser.h"
#include "PropertiesManager.h"

namespace stm {
namespace core {

int PropertiesLoader::readInt(PropertyId property) const
{
    (void)property;

    return 0;
}

std::string PropertiesLoader::readString(SensorPropertyId property,
                                         SensorType sensorType) const
{
    (void)property;
    (void)sensorType;

    return "";
}

int PropertiesLoader::readInt(SensorPropertyId property,
                              SensorType sensorType) const
{
    (void)property;
    (void)sensorType;

    return 0;
}

enum class PropertyNum {
    ONE,
    TWO
};

PropertiesManager& PropertiesManager::getInstance(void)
{
    static PropertiesManager propertiesManager;

    return propertiesManager;
}

PropertiesManager::PropertiesManager()
    : maxOdr(HAL_MAX_ODR_HZ),
      identityMatrix(createIdentityMatrix()),
      console(IConsole::getInstance())
{
    maxRanges[SensorType::ACCELEROMETER] = HAL_ACCEL_MAX_RANGE_MS2;
    maxRanges[SensorType::GYROSCOPE] = HAL_GYRO_MAX_RANGE_RPS;
    maxRanges[SensorType::MAGNETOMETER] = HAL_MAGN_MAX_RANGE_UT;
}

Matrix<3, 3, float> PropertiesManager::createIdentityMatrix() const
{
    Matrix<3, 3, float> matrix;

    for (auto i = 0; i < matrix.numRows(); ++i) {
        for (auto j = 0; j < matrix.numColumns(); ++j) {
            if (i == j) {
                matrix[i][j] = 1.0f;
            } else {
                matrix[i][j] = 0.0f;
            }
        }
    }

    return matrix;
}

int PropertiesManager::load(const PropertiesLoader& loader)
{
    loadRotationMatrices(loader, PropertyNum::ONE);
    loadSensorsPlacement(loader, PropertyNum::ONE);
    loadRotationMatrices(loader, PropertyNum::TWO);
    loadSensorsPlacement(loader, PropertyNum::TWO);
    loadMaxRanges(loader);
    loadMaxOdrs(loader);

    calculateFinalRotationMatrices();
    calculateFinalSensorsPlacement();

    return 0;
}

void PropertiesManager::loadRotationMatrices(const PropertiesLoader &loader,
                                             PropertyNum prop)
{
    std::vector<SensorType> sensorsSupported = {
        SensorType::ACCELEROMETER,
        SensorType::MAGNETOMETER,
        SensorType::GYROSCOPE,
    };

    for (auto &sensor : sensorsSupported) {
        std::unordered_map<SensorType, Matrix<3, 3, float>> *rotMatrix;
        SensorPropertyId propertyId;
        std::string text;

        switch (prop) {
        case PropertyNum::ONE:
            rotMatrix = &rotationMatrices_1;
            propertyId = SensorPropertyId::ROTATION_MATRIX_1;
            break;
        case PropertyNum::TWO:
            rotMatrix = &rotationMatrices_2;
            propertyId = SensorPropertyId::ROTATION_MATRIX_2;
            break;
        default:
            return;
        }

        PropertiesParser rotMatrixParser =
            PropertiesParser::makeRotationMatrixParser(loader.readString(propertyId, sensor));
        if (rotMatrixParser.isValid()) {
            for (auto i = 0U; i < rotMatrixParser.getData().size(); ++i) {
                int row = i / 3;
                int col = i % 3;

                ((*rotMatrix)[sensor])[row][col] = rotMatrixParser.getData()[i];
            }
        } else if (prop == PropertyNum::ONE) {
            ((*rotMatrix)[sensor]) = identityMatrix;
        }
    }
}

void PropertiesManager::loadSensorsPlacement(const PropertiesLoader& loader,
                                             PropertyNum prop)
{
    std::vector<SensorType> sensorsSupported = {
        SensorType::ACCELEROMETER,
        SensorType::MAGNETOMETER,
        SensorType::GYROSCOPE,
        SensorType::PRESSURE
    };

    for (auto& sensor : sensorsSupported) {
        std::unordered_map<SensorType, std::array<float, 3>> *placement;
        SensorPropertyId propertyId;
        std::string text;

        switch (prop) {
        case PropertyNum::ONE:
            propertyId = SensorPropertyId::SENSOR_PLACEMENT_1;
            placement = &sensorsPlacement_1;
            break;
        case PropertyNum::TWO:
            propertyId = SensorPropertyId::SENSOR_PLACEMENT_2;
            placement = &sensorsPlacement_2;
            break;
        default:
            return;
        }

        PropertiesParser placementParser =
            PropertiesParser::makeSensorPlacementParser(loader.readString(propertyId, sensor));
        if (placementParser.isValid()) {
            for (auto i = 0U; i < placementParser.getData().size(); ++i) {
                (*placement)[sensor][i] = placementParser.getData()[i];
            }
        } else if (prop == PropertyNum::ONE) {
            (*placement)[sensor] = { 0, 0, 0 };
        }
    }
}

void PropertiesManager::loadMaxRanges(const PropertiesLoader& loader)
{
    std::vector<SensorType> sensorsSupported = {
        SensorType::ACCELEROMETER,
        SensorType::MAGNETOMETER,
        SensorType::GYROSCOPE,
        SensorType::PRESSURE
    };

    for (auto& sensor : sensorsSupported) {
        auto range = loader.readInt(SensorPropertyId::MAX_RANGE, sensor);
        if (range > 0) {
            maxRanges[sensor] = range;
        }
    }
}

void PropertiesManager::loadMaxOdrs(const PropertiesLoader& loader)
{
    std::string propName;

    auto value = loader.readInt(PropertyId::MAX_ODR);
    if (value > 0) {
        maxOdr = value;
    }
}

void PropertiesManager::calculateFinalRotationMatrices()
{
    for (auto& rotMatrix_1 : rotationMatrices_1) {
        auto itrRotMatrix_2 = rotationMatrices_2.find(rotMatrix_1.first);
        if (itrRotMatrix_2 != rotationMatrices_2.end()) {
            rotationMatrices[rotMatrix_1.first] = itrRotMatrix_2->second * rotMatrix_1.second;
        } else {
            rotationMatrices[rotMatrix_1.first] = rotMatrix_1.second;
        }

        std::string sensorName = "sensor";
        auto itrSensorName = sensorTypeToString.find(rotMatrix_1.first);
        if (itrSensorName != sensorTypeToString.end()) {
            sensorName = itrSensorName->second;
        }
        std::string msg = sensorName + std::string(" rotation matrix: ") +
            (std::string)rotationMatrices[rotMatrix_1.first];

        console.info(msg);
    }
}

void PropertiesManager::calculateFinalSensorsPlacement()
{
    for (auto& placement_1 : sensorsPlacement_1) {
        auto itrPlacement_2 = sensorsPlacement_2.find(placement_1.first);
        if (itrPlacement_2 != sensorsPlacement_2.end()) {
            auto itrRotMatrix_2 = rotationMatrices_2.find(placement_1.first);
            if (itrRotMatrix_2 != rotationMatrices_2.end()) {
                std::array<float, 3> placementRemmaped = itrRotMatrix_2->second *
                    placement_1.second;

                for (auto i = 0U; i < placementRemmaped.size(); ++i) {
                    sensorsPlacement[placement_1.first][i] =
                        placementRemmaped[i] + itrPlacement_2->second[i];
                }
            } else {
                sensorsPlacement[placement_1.first] = placement_1.second;
            }
        } else {
            sensorsPlacement[placement_1.first] = placement_1.second;
        }

        std::string sensorName = "sensor";
        auto itrSensorName = sensorTypeToString.find(placement_1.first);
        if (itrSensorName != sensorTypeToString.end()) {
            sensorName = itrSensorName->second;
        }
        std::string msg = sensorName + std::string(" placement: ");
        for (auto i = 0U; i < sensorsPlacement[placement_1.first].size(); ++i) {
            msg += std::to_string(sensorsPlacement[placement_1.first][i]);
            if (i < sensorsPlacement[placement_1.first].size() - 1) {
                msg += ", ";
            }
        }

        console.info(msg);
    }
}

const Matrix<3, 3, float>& PropertiesManager::getRotationMatrix(SensorType sensorType) const
{
    auto itr = rotationMatrices.find(sensorType);
    if (itr == rotationMatrices.end()) {
        return identityMatrix;
    }

    return itr->second;
}

const std::array<float, 3> PropertiesManager::getSensorPlacement(SensorType sensorType) const
{
    auto itr = sensorsPlacement.find(sensorType);
    if (itr == sensorsPlacement.end()) {
        return std::array<float, 3> { 0 };
    }

    return itr->second;
}

float PropertiesManager::getMaxRangeOfMeasurements(SensorType sensorType) const
{
    auto itr = maxRanges.find(sensorType);
    if (itr == maxRanges.end()) {
        return 0;
    }

    return itr->second;
}

float PropertiesManager::getMaxOdr() const
{
    return maxOdr;
}

} // namespace core
} // namespace stm
