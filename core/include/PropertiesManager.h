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

#pragma once

#include <string>
#include <array>
#include <unordered_map>

#include <IConsole.h>
#include <SensorType.h>
#include <Matrix.h>
#include "STMSensorsList.h"

namespace stm {
namespace core {

enum class SensorPropertyId {
    ROTATION_MATRIX_1,
    SENSOR_PLACEMENT_1,
    ROTATION_MATRIX_2,
    SENSOR_PLACEMENT_2,
    MAX_RANGE,
};

enum class PropertyId {
    MAX_ODR
};

struct PropertiesLoader {
    virtual int readInt(PropertyId property) const;

    virtual std::string readString(SensorPropertyId property,
                                   SensorType sensorType,
                                   uint32_t index) const;

    virtual int readInt(SensorPropertyId property,
                        SensorType sensorType) const;

    virtual ~PropertiesLoader() = default;
};

class PropertiesManager {
public:
    static PropertiesManager& getInstance(void);
    ~PropertiesManager(void) = default;

    int getMaxRanges(const PropertiesLoader& loader);

    int load(const PropertiesLoader& loader, const STMSensorsList& sensorList);

    const Matrix<3, 3, float>& getRotationMatrix(SensorHandle sensorHandle) const;

    const std::array<float, 3> getSensorPlacement(SensorHandle sensorHandle) const;

    float getMaxRangeOfMeasurements(SensorType sensorType) const;

    float getMaxOdr() const;

private:
    enum class PropertyNum {
        ONE,
        TWO
    };

    PropertiesManager(void);

    void loadUniqueSensorMap(const STMSensorsList& sensorList);

    void loadRotationMatrices(const PropertiesLoader& loader, PropertyNum propNum);

    void loadSensorsPlacement(const PropertiesLoader& loader, PropertyNum propNum);

    void loadMaxRanges(const PropertiesLoader& loader);

    void loadMaxOdrs(const PropertiesLoader& loader);

    void calculateFinalRotationMatrices();

    void calculateFinalSensorsPlacement();

    bool isSupported(SensorType sensorType);

    int getSensorInstance(SensorType typeSensor);

    Matrix<3, 3, float> createIdentityMatrix() const;

    std::unordered_map<SensorHandle, SensorType> uniqueSensorMap;

    std::unordered_map<SensorHandle, int> uniqueSensorInstance;

    std::unordered_map<SensorHandle, Matrix<3, 3, float>> rotationMatrices;

    std::unordered_map<SensorHandle, std::array<float, 3>> sensorsPlacement;

    std::unordered_map<SensorHandle, Matrix<3, 3, float>> rotationMatrices_1;

    std::unordered_map<SensorHandle, std::array<float, 3>> sensorsPlacement_1;

    std::unordered_map<SensorHandle, Matrix<3, 3, float>> rotationMatrices_2;

    std::unordered_map<SensorHandle, std::array<float, 3>> sensorsPlacement_2;

    std::unordered_map<SensorType, float> maxRanges;

    float maxOdr;

    Matrix<3, 3, float> identityMatrix;

    IConsole& console;
};

} // namespace core
} // namespace stm
