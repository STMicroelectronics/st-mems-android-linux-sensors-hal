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

#pragma once

#include <thread>
#include <memory>
#include <atomic>

#include <ISTMSensorsHAL.h>

namespace stm {
namespace core {

class STMSensorsHAL : public ISTMSensorsHAL {
public:
    static STMSensorsHAL& getInstance(void);
    ~STMSensorsHAL(void);

    void initialize(const ISTMSensorsCallback &sensorsCallback) final;

    const STMSensorsList& getSensorsList(void) final;

    int activate(uint32_t handle, bool enable) final;

    int setRate(uint32_t handle,
                int64_t samplingPeriodNanoSec,
                int64_t maxReportLatencyNanoSec) final;

    int flushData(uint32_t handle) final;

private:
    STMSensorsHAL(void);

    /**
     * Sensors list
     */
    STMSensorsList sensorsList;

    /**
     * Interface used to talk with the wrappers (hild / legacy)
     */
    ISTMSensorsCallback *sensorsCallback;

    /**
     * Print console
     */
    IConsole &console;

    class MySTMSensorCallback : public ISTMSensorsCallback {
        void onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData) override
        {
            (void) sensorsData;
        }

        int onSaveDataRequest(const std::string& resourceID, const void *data, ssize_t len) override
        {
            (void) resourceID;
            (void) data;
            (void) len;

            return -EIO;
        }

        int onLoadDataRequest(const std::string& resourceID, void *data, ssize_t len) override
        {
            (void) resourceID;
            (void) data;
            (void) len;

            return -EIO;
        }
    };

    /**
     * Empty default sensors callback
     */
    MySTMSensorCallback emptySTMSensorCallback;

    std::atomic<bool> dataReceivedThreadRunning;

    std::unique_ptr<std::thread> dataReceivedThread;

    void *hal_data;

    bool handleIsValid(uint32_t handle) const;

    static void internalPoll(STMSensorsHAL *hal, std::atomic<bool> *running);
};

} // namespace core
} // namespace stm
