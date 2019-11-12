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

#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <cstring>

#include "SensorsLinuxInterface.h"
#include <IConsole.h>

SensorsLinuxInterface::SensorsLinuxInterface(void)
                      : sensorsCore(ISTMSensorsHAL::getInstance()),
                        console(IConsole::getInstance())
{
}

/**
 * initialize: initialize the interface
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsLinuxInterface::initialize(void)
{
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
 * @handle: sensor handle ID (retrieved from sensors list)
 * @enable: enable or disable flag.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsLinuxInterface::enable(uint32_t handle, bool enable)
{
    return sensorsCore.activate(handle, enable);
}

/**
 * onNewSensorsData: receive data from STMSensorsHAL,
 *                   reference: ISTMSensorsCallbackData class
 */
void
SensorsLinuxInterface::onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData)
{
    (void) sensorsData;
}
