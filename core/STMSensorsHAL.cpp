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

#include <functional>
#include <cerrno>
#include <cstring>

#include <STMSensorsCallbackData.h>
#include <STMSensorsHAL.h>
#include "sensors_legacy.h"

namespace stm {
namespace core {

STMSensorsHAL& STMSensorsHAL::getInstance(void)
{
    static STMSensorsHAL instance;

    return instance;
}

STMSensorsHAL::STMSensorsHAL(void)
              : sensorsCallback(&emptySTMSensorCallback),
                console(IConsole::getInstance()),
                dataReceivedThreadRunning(false)
{
    st_hal_open_sensors(&hal_data, sensorsList);

    if (hal_data) {
        dataReceivedThreadRunning = true;
        dataReceivedThread = std::make_unique<std::thread>(internalPoll, this, &dataReceivedThreadRunning);
    }
}

STMSensorsHAL::~STMSensorsHAL(void)
{
    dataReceivedThreadRunning = false;
    if (dataReceivedThread != nullptr) {
        dataReceivedThread->join();
    }
}

void STMSensorsHAL::internalPoll(STMSensorsHAL *hal, std::atomic<bool> *running)
{
    struct sensors_event_t sdata[10];

    while (running->load()) {
        auto n = st_hal_dev_poll(hal->hal_data, sdata, 10);

        {
            std::vector<ISTMSensorsCallbackData> sensorsData;
            std::vector<float> payload;

            for (auto i = 0; i < n; ++i) {
                payload.resize(sdata[i].data.dataLen);
                memcpy(payload.data(), sdata[i].data.data2, sdata[i].data.dataLen * sizeof(float));
                sensorsData.push_back(STMSensorsCallbackData(sdata[i].sensor,
                                                             sdata[i].type,
                                                             sdata[i].timestamp,
                                                             payload));
            }

            if (sensorsData.size() > 0) {
                hal->sensorsCallback->onNewSensorsData(sensorsData);
            }
        }
    }
}

/**
 * initialize: implementation of an interface,
 *             reference: ISTMSensorsHAL.h
 */
void STMSensorsHAL::initialize(const ISTMSensorsCallback &sensorsCallback)
{
    for (auto &sensor : getSensorsList().getList()) {
        activate(sensor.getHandle(), false);
    }

    this->sensorsCallback = (ISTMSensorsCallback *)&sensorsCallback;
}

/**
 * getSensorslist: implementation of an interface,
 *                 reference: ISTMSensorsHAL.h
 */
const STMSensorsList& STMSensorsHAL::getSensorsList(void)
{
    return sensorsList;
}

/**
 * activate: implementation of an interface,
 *           reference: ISTMSensorsHAL.h
 */
int STMSensorsHAL::activate(uint32_t handle, bool enable)
{
    if (!handleIsValid(handle)) {
        return -EINVAL;
    }

    return st_hal_dev_activate(hal_data, handle, enable);
}

/**
 * setRate: implementation of an interface,
 *          reference: ISTMSensorsHAL.h
 */
int STMSensorsHAL::setRate(uint32_t handle,
                           int64_t samplingPeriodNanoSec,
                           int64_t maxReportLatencyNanoSec)
{
    if (!handleIsValid(handle)) {
        return -EINVAL;
    }

    return st_hal_dev_batch(hal_data, handle, samplingPeriodNanoSec, maxReportLatencyNanoSec);
}

/**
 * flushData: implementation of an interface,
 *            reference: ISTMSensorsHAL.h
 */
int STMSensorsHAL::flushData(uint32_t handle)
{
    if (!handleIsValid(handle)) {
        return -EINVAL;
    }

    return st_hal_dev_flush(hal_data, handle);
}

/**
 * handleIsValid: check if given handle is valid or not
 * @handle: sensor handle to check
 *
 * Return value: true if handle is valid, false otherwise.
 */
bool STMSensorsHAL::handleIsValid(uint32_t handle) const
{
    if ((handle == 0) || (handle > sensorsList.getList().size())) {
        return false;
    }

    return true;
}

} // namespace core
} // namespace stm
