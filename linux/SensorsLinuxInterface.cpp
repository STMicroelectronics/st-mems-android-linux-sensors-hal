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

#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <cstring>

#include "SensorsLinuxInterface.h"
#include "LinuxPropertiesLoader.h"
#include <IConsole.h>
#include <fstream>

static const std::string configFilename = "/etc/stm-sensors-hal/config";
static std::ofstream logfile("sensorlog.txt");

SensorsLinuxInterface::SensorsLinuxInterface(void)
                      : sensorsCore(ISTMSensorsHAL::getInstance()),
                        console(IConsole::getInstance()),
                        propertiesManager(PropertiesManager::getInstance())
{
}

/**
 * initialize: initialize the interface
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsLinuxInterface::initialize(void)
{
    int ret;

    LinuxPropertiesLoader linuxPropertiesLoader;

    ret = linuxPropertiesLoader.loadFromConfigFile(configFilename);
    if (!ret)
        propertiesManager.getMaxRanges(linuxPropertiesLoader);

    sensorsCore.initialize(*dynamic_cast<ISTMSensorsCallback *>(this));

    return 0;
}

/**
 * getSensorsList: retrieve sensors list
 *
 * Return value: const reference of sensors list.
 */
const std::vector<STMSensor>& SensorsLinuxInterface::getSensorsList(void) const
{
    return sensorsCore.getSensorsList().getList();
}

/**
 * enable: enable or disable specified sensor
 * @handle: sensor handle ID (retrieved from sensors list).
 * @enable: enable or disable flag.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsLinuxInterface::enable(uint32_t handle, bool enable)
{
    return sensorsCore.activate(handle, enable);
}

/**
 * setRate: set sensor sampling period and batch time
 * @handle: sensor handle ID (retrieved from sensors list).
 * @samplingPeriodNanoSec: sensor sampling period in nsec.
 * @maxReportLatencyNanoSec: sensor batch time in nsec.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsLinuxInterface::setRate(uint32_t handle,
                                  int64_t samplingPeriodNanoSec,
                                  int64_t maxReportLatencyNanoSec)
{
    return sensorsCore.setRate(handle, samplingPeriodNanoSec, maxReportLatencyNanoSec);
}

/**
 * setFullScale:
 * @handle: sensor handle ID (retrieved from sensors list).
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsLinuxInterface::setFullScale(uint32_t handle, float fullscale)
{
    return sensorsCore.setFullScale(handle, fullscale);
}

/**
 * onNewSensorsData: receive data from STMSensorsHAL,
 *                   reference: ISTMSensorsCallbackData class
 */
void
SensorsLinuxInterface::onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData)
{
    /* just dump sensors data */
    for (auto& s : sensorsData) {
        std::vector<float> data = s.getData();
        switch (s.getSensorType()) {
        case SensorType::ACCELEROMETER:
        case SensorType::MAGNETOMETER:
        case SensorType::GYROSCOPE:
        case SensorType::LINEAR_ACCELERATION:
        case SensorType::GRAVITY:
        case SensorType::ROTATION_VECTOR:
        case SensorType::GEOMAGNETIC_ROTATION_VECTOR:
        case SensorType::ORIENTATION:
            console.info("#" + std::to_string(s.getSensorHandle()) + ": " +
                         std::to_string(data[0]) + ", " +
                         std::to_string(data[1]) + ", " +
                         std::to_string(data[2]) + " T " +
                         std::to_string(s.getTimestamp()));
            break;
        case SensorType::GAME_ROTATION_VECTOR:
            console.info("#" + std::to_string(s.getSensorHandle()) + ": " +
                         std::to_string(data[0]) + ", " +
                         std::to_string(data[1]) + ", " +
                         std::to_string(data[2]) + ", " +
                         std::to_string(data[3]) + " T " +
                         std::to_string(s.getTimestamp()));
            break;
        case SensorType::AMBIENT_TEMPERATURE:
        case SensorType::INTERNAL_TEMPERATURE:
        case SensorType::PRESSURE:
        case SensorType::LIGHT:
        case SensorType::PROXIMITY:
        case SensorType::RELATIVE_HUMIDITY:
            console.info("#" + std::to_string(s.getSensorHandle()) + ": " +
                         std::to_string(data[0]) + " T " +
                         std::to_string(s.getTimestamp()));
            break;
        case SensorType::ACCELEROMETER_UNCALIBRATED:
        case SensorType::GYROSCOPE_UNCALIBRATED:
        case SensorType::MAGNETOMETER_UNCALIBRATED:
            console.info("#" + std::to_string(s.getSensorHandle()) + ": " +
                         std::to_string(data[0]) + ", " +
                         std::to_string(data[1]) + ", " +
                         std::to_string(data[2]) + " bias " +
                         std::to_string(data[3]) + ", " +
                         std::to_string(data[4]) + ", " +
                         std::to_string(data[5]) + " T " +
                         std::to_string(s.getTimestamp()));
            break;
        default:
            console.info("#" + std::to_string(s.getSensorHandle()) + ": unknown sensor type");
            break;
        }
    }
}

/**
 * onSaveDataRequest: receive data to store,
 *                    reference: ISTMSensorsCallbackData class
 */
int SensorsLinuxInterface::onSaveDataRequest(const std::string& resourceID,
                                             const void *data, ssize_t len)
{
    (void) resourceID;
    (void) data;
    (void) len;

    return 0;
}

/**
 * onLoadDataRequest: load data from disk,
 *                    reference: ISTMSensorsCallbackData class
 */
int SensorsLinuxInterface::onLoadDataRequest(const std::string& resourceID,
                                             void *data, ssize_t len)
{
    (void) resourceID;
    (void) data;
    (void) len;

    return 0;
}
