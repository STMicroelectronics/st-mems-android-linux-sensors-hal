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

#include <poll.h>
#include <hardware/sensors.h>

#include <ISTMSensorsHAL.h>
#include <IUtils.h>

struct SensorsLegacyInterface : public ISTMSensorsCallback {
public:
    SensorsLegacyInterface(void);
    ~SensorsLegacyInterface(void);

    int initialize(void);

    unsigned int getSensorsList(struct sensor_t const **sensorsList) const;

    static int activate(struct sensors_poll_device_t *dev,
                        int sensor_handle, int enabled);

    static int setDelay(struct sensors_poll_device_t *dev,
                        int sensor_handle, int64_t sampling_period_ns);

    static int poll(struct sensors_poll_device_t *dev,
                    sensors_event_t *data, int count);

    static int batch(struct sensors_poll_device_1 *dev,
                     int sensor_handle, int flags,
                     int64_t sampling_period_ns,
                     int64_t max_report_latency_ns);

    static int flush(struct sensors_poll_device_1 *dev,
                     int sensor_handle);

    static int injectSensorData(struct sensors_poll_device_1 *dev,
                                const sensors_event_t *data);

    static int registerDirectChannel(struct sensors_poll_device_1 *dev,
                                     const struct sensors_direct_mem_t *mem,
                                     int channel_handle);

    static int configDirectChannel(struct sensors_poll_device_1 *dev,
                                   int sensor_handle, int channel_handle,
                                   const struct sensors_direct_cfg_t *config);

    void onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData) override;

private:
    int initializePipe(void);

    /**
     * Core library object interface
     */
    ISTMSensorsHAL &sensorsCore;

    /**
     * Print console
     */
    IConsole &console;

    /**
     * Poll object used by poll() function
     */
    struct pollfd sensorsDataPollFd;

    /**
     * Pipe fds (read and write)
     */
    int pipeFd[2] = {-1, -1};
};
