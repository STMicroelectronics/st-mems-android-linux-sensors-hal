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

#pragma once

#include <array>
#include <string>

#include <PropertiesManager.h>
#include <SensorType.h>

struct SensorPlacement {
public:
    SensorPlacement(void);
    ~SensorPlacement(void) = default;

    const std::array<float, 12> &getPayload(void) const;

    void loadFromProp(stm::core::SensorType sensorType, stm::core::SensorHandle sensorHandle);

private:
    bool invertRotationMatrix(Matrix<3, 3, float>& matrix);

    /**
     * Sensor placement payload (4x3 matrix)
     */
    std::array<float, 12> data;

    /**
     * Print console
     */
    stm::core::IConsole &console = stm::core::IConsole::getInstance();
};
