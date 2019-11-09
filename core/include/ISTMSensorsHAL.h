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

#pragma once

#include <cstdint>
#include <string>

#include <IConsole.h>
#include <STMSensor.h>
#include <STMSensorsList.h>
#include <ISTMSensorsCallback.h>

class ISTMSensorsHAL {
public:
    ISTMSensorsHAL(void) = default;
    virtual ~ISTMSensorsHAL(void) = default;
    ISTMSensorsHAL(const ISTMSensorsHAL &) = delete;
    ISTMSensorsHAL& operator= (const ISTMSensorsHAL &) = delete;

    /**
     * getInstance: retrieve sensors HAL instance
     *
     * Return value: valid instance of the library interface.
     */
    static ISTMSensorsHAL& getInstance(void);

    /**
     * initialize: initialize the library to reset condition
     * @sensorsCallback: object receiving sensors data.
     */
    virtual void initialize(const ISTMSensorsCallback &sensorsCallback) = 0;

    /**
     * getSensorsList: retrieve sensors list
     *
     * Return value: const reference of sensors list.
     */
    virtual const STMSensorsList& getSensorsList(void) = 0;

    /**
     * activate: enable or disable specified sensor
     * @handle: sensor handle ID (retrieved from sensors list).
     * @enable: enable or disable flag.
     *
     * Return value: 0 on success, else a negative error code.
     */
    virtual int32_t activate(uint32_t handle, bool enable) = 0;

    /**
     * setRate: request new data rate and deadline delivery time for specified sensor
     * @handle: sensor handle ID (retrieved from sensors list).
     * @samplingPeriodNanoSec: requested sensor data period in nanoseconds.
     * @maxReportLatencyNanoSec: requested sensor data maximum reporting latency in nanoseconds.
     *
     * Return value: 0 on success, else a negative error code.
     */
    virtual int32_t setRate(uint32_t handle,
                            int64_t samplingPeriodNanoSec,
                            int64_t maxReportLatencyNanoSec) = 0;

    /**
     * flushData: request to retrieve all sensor data immediately for specified sensor,
     *            in case FIFO is used, force to read the data at this time.
     *            If FIFO is not used, the function has no effect
     * @handle: sensor handle ID (retrieved from sensors list).
     *
     * Return value: 0 on success, else a negative error code.
     */
    virtual int32_t flushData(uint32_t handle) = 0;
};
