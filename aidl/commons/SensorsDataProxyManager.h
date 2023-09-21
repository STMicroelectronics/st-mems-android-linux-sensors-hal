/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2020 STMicroelectronics
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
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>

struct SensorsDataProxyManager {
public:
    SensorsDataProxyManager(void) = default;
    ~SensorsDataProxyManager(void) = default;

    void reset(void);

    int addChannel(int32_t channelHandle);

    int removeChannel(int32_t channelHandle);

    int registerSensorToChannel(int32_t sensorHandle, int32_t channelHandle);

    int unregisterSensorFromChannel(int32_t sensorHandle, int32_t channelHandle);

    int configureSensorInChannel(int32_t sensorHandle,
                                 int32_t channelHandle,
                                 int64_t pollrateNs);

    int64_t getMaxPollrateNs(int32_t sensorHandle) const;

    std::vector<int32_t> getChannels(int32_t sensorHandle) const;

    std::vector<int32_t> getRegisteredSensorsInChannel(int32_t channelHandle) const;

    std::vector<int32_t> getValidPushChannels(int64_t timestamp,
                                              int32_t sensorHandle,
                                              int64_t pollrateNs);

private:
    struct PollrateSwitchData {
        int64_t timestampOfChange;
        int64_t pollrateNs;
    };

    struct ProxyData {
        int samplesCounter;
        int64_t pollrateNs;

        std::list<struct PollrateSwitchData> switchDataFifo;
    };

    /**
     * map: sensorHandle -> channelHandle -> ProxyData
     */
    std::unordered_map<int32_t,
        std::unordered_map<int32_t,
        struct ProxyData>> mSensorToChannel;

    /**
     * map: channelHandle -> sensorHandle
     */
    std::unordered_map<int32_t,
        std::unordered_set<int32_t>> mChannelToSensor;

    int calculateDecimator(int64_t dividend, int64_t divisor) const;
};
