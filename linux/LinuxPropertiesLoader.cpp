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

#include <fstream>
#include <string>
#include <regex>

#include "LinuxPropertiesLoader.h"

static const std::string initialSpacesRegex = "^[ \t\r\f]*";

static const std::unordered_map<std::string, PropertyId> configsRegex = {
    { initialSpacesRegex + "max-odr[ \t\r\f]*=.*", PropertyId::MAX_ODR }
};

static const std::unordered_map<std::string, SensorPropertyId> sensorsConfigsRegex = {
    { initialSpacesRegex + "max-range.(accel|gyro|magn)-\\d+[ \t\r\f]*=.*", SensorPropertyId::MAX_RANGE },
    { initialSpacesRegex + "rot-matrix-1.(accel|gyro|magn)-\\d+[ \t\r\f]*=.*", SensorPropertyId::ROTATION_MATRIX_1 },
    { initialSpacesRegex + "rot-matrix-2.(accel|gyro|magn)-\\d+[ \t\r\f]*=.*", SensorPropertyId::ROTATION_MATRIX_2 },
    { initialSpacesRegex + "placement-1.(accel|gyro|magn)-\\d+[ \t\r\f]*=.*", SensorPropertyId::SENSOR_PLACEMENT_1 },
    { initialSpacesRegex + "placement-2.(accel|gyro|magn)-\\d+[ \t\r\f]*=.*", SensorPropertyId::SENSOR_PLACEMENT_2 },
};

SensorType LinuxPropertiesLoader::getSensorTypeForProperty(const std::string& line)
{
    if (line.find("gyro") != std::string::npos) {
        return SensorType::GYROSCOPE;
    } else if (line.find("accel") != std::string::npos) {
        return SensorType::ACCELEROMETER;
    } else if (line.find("magn") != std::string::npos) {
        return SensorType::MAGNETOMETER;
    }

    return SensorType::META_DATA;
}

void LinuxPropertiesLoader::parseConfigLine(std::string& line)
{
    if (std::regex_match(line, std::regex(initialSpacesRegex + "#.*"))) {
        return;
    }

    for (auto& configRegex : configsRegex) {
        if (std::regex_match(line, std::regex(configRegex.first))) {
            std::size_t found = line.find("=");
            if (found != std::string::npos) {
                properties[configRegex.second] = line.substr(found + 1);
            }
            return;
        }
    }

    for (auto& configRegex : sensorsConfigsRegex) {
        if (std::regex_match(line, std::regex(configRegex.first))) {
            SensorType sensorType = getSensorTypeForProperty(line);
            std::size_t found = line.find("=");
            if (found != std::string::npos) {
                sensorProperties[configRegex.second][sensorType] = line.substr(found + 1);
            }
            return;
        }
    }
}

int LinuxPropertiesLoader::loadFromConfigFile(const std::string& filename)
{
    std::ifstream configFile(filename, std::ifstream::in);
    if (configFile.rdstate() == configFile.failbit) {
        console.warning("failed to open config file [" + filename + "], using defaults");
        return -EIO;
    }

    properties.clear();
    sensorProperties.clear();

    std::string line;
    while (std::getline(configFile, line)) {
        parseConfigLine(line);
    }
    configFile.close();

    return 0;
}

int LinuxPropertiesLoader::readInt(PropertyId property) const
{
    auto itr = properties.find(property);
    if (itr != properties.end()) {
        return std::atoi(itr->second.c_str());
    }

    return 0;
}

std::string LinuxPropertiesLoader::readString(SensorPropertyId property,
                                              SensorType sensorType,
                                              uint32_t index) const
{
    (void)index;

    auto itr = sensorProperties.find(property);
    if (itr != sensorProperties.end()) {
        auto itr_2 = itr->second.find(sensorType);
        if (itr_2 != itr->second.end()) {
            return itr_2->second;
        }
    }

    return "";
}

int LinuxPropertiesLoader::readInt(SensorPropertyId property,
                                   SensorType sensorType) const
{
    auto itr = sensorProperties.find(property);
    if (itr != sensorProperties.end()) {
        auto itr_2 = itr->second.find(sensorType);
        if (itr_2 != itr->second.end()) {
            return std::atoi(itr_2->second.c_str());
        }
    }

    return 0;
}
