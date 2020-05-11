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

#include <STMSensorsCallbackData.h>
#include <STMSensorsHAL.h>

STMSensorsHAL::STMSensorsHAL(void)
              : sensorsCallback(&emptySTMSensorCallback),
                console(IConsole::getInstance())
{
}

/**
 * initialize: implementation of an interface,
 *             reference: ISTMSensorsHAL.h
 */
void STMSensorsHAL::initialize(const ISTMSensorsCallback &sensorsCallback)
{
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
    (void) enable;

    if (!handleIsValid(handle)) {
        return -EINVAL;
    }

    return 0;
}

/**
 * setRate: implementation of an interface,
 *          reference: ISTMSensorsHAL.h
 */
int STMSensorsHAL::setRate(uint32_t handle,
                           int64_t samplingPeriodNanoSec,
                           int64_t maxReportLatencyNanoSec)
{
    (void) samplingPeriodNanoSec;
    (void) maxReportLatencyNanoSec;

    if (!handleIsValid(handle)) {
        return -EINVAL;
    }

    return 0;
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

    return 0;
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
