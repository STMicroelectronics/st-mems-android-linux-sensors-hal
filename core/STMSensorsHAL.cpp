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

#include <functional>

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
    (void) handle;
    (void) enable;

    return -EINVAL;
}

/**
 * setRate: implementation of an interface,
 *          reference: ISTMSensorsHAL.h
 */
int STMSensorsHAL::setRate(uint32_t handle,
                           int64_t samplingPeriodNanoSec,
                           int64_t maxReportLatencyNanoSec)
{
    (void) handle;
    (void) samplingPeriodNanoSec;
    (void) maxReportLatencyNanoSec;

    return -EINVAL;
}

/**
 * flushData: implementation of an interface,
 *            reference: ISTMSensorsHAL.h
 */
int STMSensorsHAL::flushData(uint32_t handle)
{
    (void) handle;

    return -EINVAL;
}
