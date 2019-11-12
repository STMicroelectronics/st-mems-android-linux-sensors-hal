/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019 STMicroelectronics
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

#include "SensorsLinuxInterface.h"
#include "Console.h"

static IConsole &console = IConsole::getInstance();

static void printSensorList(const std::vector<STMSensor> &sensorsList)
{
    int index { 0 };

    console.info("<<Sensor List>> - BEGIN");

    for (auto sensor : sensorsList) {
        console.info(std::to_string(++index) + ". Name: " + sensor.getName());
    }

    console.info("<<Sensor List>> - END\n");
}

int main(int /* argc */, char** /* argv */)
{
    std::unique_ptr<SensorsLinuxInterface> sensors = std::make_unique<SensorsLinuxInterface>();
    int err;

    err = sensors->initialize();
    if (err) {
        console.error("failed to initialize the sensors interface");
        return err;
    }

    const std::vector<STMSensor> &sensorsList = sensors->getSensorsList();
    printSensorList(sensorsList);

    std::this_thread::sleep_for(std::chrono::seconds(2));



    return 0;
}
