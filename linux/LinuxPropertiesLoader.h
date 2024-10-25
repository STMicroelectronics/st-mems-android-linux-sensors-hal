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

#include <IConsole.h>
#include <PropertiesManager.h>

using stm::core::PropertyId;
using stm::core::SensorPropertyId;
using stm::core::SensorType;

class LinuxPropertiesLoader : public stm::core::PropertiesLoader {
private:
    stm::core::IConsole& console = stm::core::IConsole::getInstance();

    std::unordered_map<SensorPropertyId,
                       std::unordered_map<SensorType, std::string>> sensorProperties;

    std::unordered_map<PropertyId, std::string> properties;

    SensorType getSensorTypeForProperty(const std::string& line);

    void parseConfigLine(std::string& line);

public:
    int loadFromConfigFile(const std::string& filename);

    virtual int readInt(PropertyId property) const override;

    virtual std::string readString(SensorPropertyId property,
                                   SensorType sensorType,
                                   uint32_t index) const override;

    virtual int readInt(SensorPropertyId property,
                        SensorType sensorType) const override;
};
