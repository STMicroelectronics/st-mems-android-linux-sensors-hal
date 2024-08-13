/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019-2020 STMicroelectronics
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

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <getopt.h>
#include <regex>
#include <sstream>

#include "SensorsLinuxInterface.h"

using stm::core::IConsole;

static IConsole &console = IConsole::getInstance();
static option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { "list", no_argument, NULL, 'l' },
    { "listshort", no_argument, NULL, 'L' },
    { "timeout", required_argument, NULL, 't' },
    { "sensor", required_argument, NULL, 's' },
    { nullptr, no_argument, nullptr, 0 }
};
static const char *options = "hlLt:s:";

class sensor_test {
public:
    sensor_test(const STMSensor *sensor, int odr) : sensor(sensor), odr(odr) {}
    sensor_test(const STMSensor *sensor, int odr, float fullscale,
                bool update_fullscale) :
                sensor(sensor), odr(odr), fullscale(fullscale),
                update_fullscale(update_fullscale) {}
    void setOdr(int _odr) { odr = _odr; }

    const STMSensor *sensor;
    int odr;
    float fullscale;
    bool update_fullscale;
};

static void printSensorList(const std::vector<stm::core::STMSensor> &sensorsList, bool shortformat)
{
    int index { 0 };

    console.info("<<Sensor List>> - BEGIN");

    for (auto &sensor : sensorsList) {
        console.info(std::to_string(++index) + " " + sensor.getName() +
                     "(" + sensor.getVendor() + ")");

        if (shortformat == false) {
            console.info("\tmodule id: " + std::to_string(sensor.getModuleId()));
            console.info("\thandle: " + std::to_string(sensor.getHandle()));
            console.info("\tversion: " + std::to_string(sensor.getVersion()));
            console.info("\ttype: " + std::to_string(static_cast<uint16_t>(sensor.getType())) +
                         " (" + stm::core::sensorTypeToString.at(sensor.getType()) + ")");
            console.info("\trange: " + std::to_string(sensor.getMaxRange()));
            console.info("\tresolution: " + std::to_string(sensor.getResolution()));
            console.info("\tpower: " + std::to_string(sensor.getPower()));
            console.info("\trate: " + std::to_string(sensor.getMinRateHz()) +
                         " - " + std::to_string(sensor.getMaxRateHz()));
            console.info("\tonchange: " + std::to_string(sensor.isOnChange()));
            console.info("\tfifo: " + std::to_string(sensor.getFifoMaxCount()));
            console.info("\twakeup: " + std::to_string(sensor.isWakeUp()));
        }
    }

    console.info("<<Sensor List>> - END\n");
}

[[maybe_unused]]
static const STMSensor* getSensorByType(const std::vector<stm::core::STMSensor> &sensorsList,
                                        stm::core::SensorType sensorType)
{
    for (auto &sensor : sensorsList) {
        if (sensor.getType() == sensorType)
            return &sensor;
    }

    return nullptr;
}

[[maybe_unused]]
static const STMSensor* getSensorByTypeModuleId(const std::vector<stm::core::STMSensor> &sensorsList,
                                                stm::core::SensorType sensorType, int moduleId)
{
    for (auto &sensor : sensorsList) {
        if (sensor.getType() == sensorType &&
            sensor.getModuleId() == moduleId) {
            console.info("Found sensor with module id " + std::to_string(moduleId));

            return &sensor;
        }
    }

    return nullptr;
}

[[maybe_unused]]
static const STMSensor* getSensorByName(const std::vector<stm::core::STMSensor> &sensorsList,
                                        std::string name)
{
    for (auto &sensor : sensorsList) {
        if (sensor.getName() == name) {
            console.info("Found sensor with module id " + name);

            return &sensor;
        }
    }

    return nullptr;
}

static void help(char *name)
{
    const std::regex hidechars("[:./]");
    std::stringstream progname_clean;
    std::string progname(name);
    std::string string_options(options);
    std::stringstream options_clean;

    std::regex_replace(std::ostream_iterator<char>(progname_clean),
                       progname.begin(), progname.end(), hidechars, "");
    std::regex_replace(std::ostream_iterator<char>(options_clean),
                       string_options.begin(), string_options.end(),
                       hidechars, "");
    console.info("Usage: " + progname_clean.str() +
                 " -[" + options_clean.str() + "] <device list>");
    console.info("\t--list (-l):\t\trequest sensor list");
    console.info("\t--listshort (-L):\trequest sensor list in short format");
    console.info("\t--timeout (-t):\t\tset test duration (seconds)");
    console.info("\t--sensor (-s):\t\tselect a sensor configuration");
    console.info("\t              \t\twhere a sensor configuration consists in a string:");
    console.info("\t              \t\t\"sensor=<type> odr=<odr> id=<instance> fullscale=<fs>\"");
    console.info("\t--help (-h):\t\tshow this help");
}

static int parseSensorString(const std::string input, int& sensor_type,
                             int& odr, int& id, float& fullscale,
                             bool& updatefs)
{
    std::unordered_map<std::string, std::string> keyValuePairs;
    std::istringstream iss(input);
    std::string token;

    sensor_type = -1;

    updatefs = false;
    while (std::getline(iss, token, ' ')) {
        std::istringstream keyValueStream(token);
        std::string key, value;

        if (std::getline(keyValueStream, key, '=') &&
            std::getline(keyValueStream, value))
            keyValuePairs[key] = value;
    }

    if (keyValuePairs.find("sensor") != keyValuePairs.end())
        sensor_type = std::stoi(keyValuePairs["sensor"]);

    if (keyValuePairs.find("odr") != keyValuePairs.end())
        odr = std::stoi(keyValuePairs["odr"]);

    if (keyValuePairs.find("id") != keyValuePairs.end())
        id = std::stoi(keyValuePairs["id"]);

    if (keyValuePairs.find("fullscale") != keyValuePairs.end()) {
        fullscale = std::stof(keyValuePairs["fullscale"]);
        updatefs = true;
    }

    /* invalid configuration */
    if (sensor_type < 0)
        return -1;

    return 0;
}

static int
parse_sensor_option(const std::vector<stm::core::STMSensor> &sensorsList,
                    std::string& sensor_option, sensor_test *&newsensortest)
{
    stm::core::SensorType sensorType;
    float fullscale = 0.0f;
    bool updatefs = false;
    int sensor_type = -1;
    int id = 1;
    int odr = 0;
    int ret;

    ret = parseSensorString(sensor_option, sensor_type, odr, id,
                            fullscale, updatefs);
    if (ret < 0)
        return ret;

    sensorType = static_cast<stm::core::SensorType>(sensor_type);
    const STMSensor* sensor = getSensorByTypeModuleId(sensorsList,
                                                      sensorType, id);

    if (sensor == nullptr)
        return -1;

    newsensortest = new sensor_test(sensor, odr, fullscale, updatefs);

    return 0;
}

int main(int argc, char** argv)
{
    std::vector<sensor_test *> sensor_config_array;
    std::vector<std::string> sensor_options;
    bool shortformat = false;
    bool getlist = false;
    int timeout = 1;
    int ret;

    while (1) {
        const int opt = getopt_long(argc, argv, options, longopts, 0);

        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 'L':
                shortformat = true;
                getlist = true;
                break;
            case 'l':
                getlist = true;
                break;
            case 't':
                timeout = std::stoi(optarg);
                break;
           case 's':
                std::cout << "Parametro s: " + std::string(optarg);
                sensor_options.push_back(optarg);
                break;
            default:
                help(argv[0]);
                exit(0);
        }
    }

    /* Process any remaining command line arguments (not options) */
    if (optind < argc) {
        int index = 0;

        std::cout << "Non-option arguments: ";
        while (optind < argc)
            std::cout << (index++) << " " << argv[optind++] << " " << std::endl;
    }

    std::unique_ptr<SensorsLinuxInterface> sensors = std::make_unique<SensorsLinuxInterface>();
    int err;

    err = sensors->initialize();
    if (err) {
        console.error("failed to initialize the sensors interface");

        return err;
    }

    const std::vector<stm::core::STMSensor> &sensorsList = sensors->getSensorsList();
    if (!sensorsList.size()) {
        console.error("no sensors available!");

        return -ENODEV;
    }

    if (getlist == true) {
        printSensorList(sensorsList, shortformat);

        exit(0);
    }

    for (std::string sensor_configuration_string: sensor_options) {
        sensor_test *sensor_config;

        ret = parse_sensor_option(sensorsList, sensor_configuration_string,
                                  sensor_config);
        if (ret)
            goto roll_over;

        sensor_config_array.push_back(sensor_config);
    }

    console.info("Running test for " + std::to_string(timeout) + " seconds");

    if (sensor_config_array.size() > 0) {
        for (sensor_test *sensor_under_test: sensor_config_array) {
            auto s = sensor_under_test->sensor;

            if (s == nullptr)
                continue;

            std::cout << "sensor: "
                      << static_cast<uint16_t>(s->getType())
                      << " odr: "
                      << sensor_under_test->odr
                      << std::endl;
            sensors->setRate(s->getHandle(), 1e9 / sensor_under_test->odr, 0);
            if (sensor_under_test->update_fullscale == true)
                sensors->setFullScale(s->getHandle(),
                                      sensor_under_test->fullscale);
            sensors->enable(s->getHandle(), true);
        }

        std::this_thread::sleep_for(std::chrono::seconds(timeout));

roll_over:
        for (sensor_test *sensor_under_test: sensor_config_array) {
            auto s = sensor_under_test->sensor;

            if (s != nullptr)
                sensors->enable(s->getHandle(), false);
            delete(sensor_under_test);
        }
    }

    sensor_config_array.clear();

    sensor_config_array.shrink_to_fit();

    return 0;
}
