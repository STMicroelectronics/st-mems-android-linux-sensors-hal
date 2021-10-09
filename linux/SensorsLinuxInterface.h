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

#include <vector>

#include <ISTMSensorsHAL.h>
#include <IUtils.h>

using stm::core::ISTMSensorsHAL;
using stm::core::ISTMSensorsCallback;
using stm::core::ISTMSensorsCallbackData;
using stm::core::STMSensor;
using stm::core::IConsole;

struct SensorsLinuxInterface : public ISTMSensorsCallback {
public:
    SensorsLinuxInterface(void);
    ~SensorsLinuxInterface(void) = default;

    int initialize(void);

    const std::vector<STMSensor>& getSensorsList(void) const;

    int enable(uint32_t handle, bool enable);

    int setRate(uint32_t handle,
                int64_t samplingPeriodNanoSec,
                int64_t maxReportLatencyNanoSec);

    void onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData) override;

    int onSaveDataRequest(const std::string& resourceID, const void *data, ssize_t len) override;

    int onLoadDataRequest(const std::string& resourceID, void *data, ssize_t len) override;

private:
    /**
     * Core library object interface
     */
    ISTMSensorsHAL &sensorsCore;

    /**
     * Print console
     */
    IConsole &console;
};
