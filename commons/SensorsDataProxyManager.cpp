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

#include <cerrno>
#include <string>
#include <sstream>
#include <IUtils.h>

#include "SensorsDataProxyManager.h"

stm::core::IUtils &utils = stm::core::IUtils::getInstance();

/**
 * reset: reset the status of the proxy manager
 */
void SensorsDataProxyManager::reset(void)
{
    mSensorToChannel.clear();
    mChannelToSensor.clear();
}

/**
 * addChannel: add channel that can be used for binding with sensor/s
 * @channelHandle: unique channel handle.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsDataProxyManager::addChannel(int32_t channelHandle)
{
    auto search_channel = mChannelToSensor.find(channelHandle);
    if (search_channel != mChannelToSensor.end()) {
        return -EADDRINUSE;
    }

    auto ret = mChannelToSensor.insert(std::make_pair(channelHandle, std::unordered_set<int32_t>()));

    return ret.second ? 0 : -ENOMEM;
}

/**
 * removeChannel: remove channel, must not be in use (bind-ed to sensor/s)
 * @channelHandle: channel handle to be removed.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsDataProxyManager::removeChannel(int32_t channelHandle)
{
    auto search_channel = mChannelToSensor.find(channelHandle);
    if (search_channel == mChannelToSensor.end()) {
        return 0;
    }

    if (!search_channel->second.empty()) {
        return -EBUSY;
    }

    mChannelToSensor.erase(channelHandle);

    return 0;
}

/**
 * registerSensorToChannel: bind a sensor and a channel
 * @sensorHandle: sensor handle to bind.
 * @channelHandle: channel handle to bind.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsDataProxyManager::registerSensorToChannel(int32_t sensorHandle,
                                                     int32_t channelHandle)
{
    auto search_channel = mChannelToSensor.find(channelHandle);
    if (search_channel == mChannelToSensor.end()) {
        return -EINVAL;
    }

    auto search_sensor = search_channel->second.find(sensorHandle);
    if (search_sensor != search_channel->second.end()) {
        return 0;
    }

    auto search_sensor_2 = mSensorToChannel.find(sensorHandle);
    if (search_sensor_2 == mSensorToChannel.end()) {
        auto ret = mSensorToChannel.insert(std::make_pair(sensorHandle,
                                                          std::unordered_map<int32_t, struct ProxyData>()));
        if (!ret.second) {
            return -ENOMEM;
        }
        search_sensor_2 = ret.first;
    }

    auto search_channel_2 = search_sensor_2->second.find(channelHandle);
    if (search_channel_2 != search_sensor_2->second.end()) {
        return -EINVAL;
    }

    struct ProxyData pdata;
    pdata.pollrateNs = 0;
    pdata.samplesCounter = 0;

    search_channel->second.insert(sensorHandle);
    search_sensor_2->second.insert(std::make_pair(channelHandle, std::move(pdata)));

    return 0;
}

/**
 * unregisterSensorFromChannel: remove binding from sensor and channel
 * @sensorHandle: sensor handle.
 * @channelHandle: channel handle.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsDataProxyManager::unregisterSensorFromChannel(int32_t sensorHandle,
                                                         int32_t channelHandle)
{
    auto search_channel = mChannelToSensor.find(channelHandle);
    if (search_channel == mChannelToSensor.end()) {
        return -EINVAL;
    }

    auto search_sensor_2 = mSensorToChannel.find(sensorHandle);
    if (search_sensor_2 == mSensorToChannel.end()) {
        return -EINVAL;
    }

    search_channel->second.erase(sensorHandle);
    search_sensor_2->second.erase(channelHandle);

    return 0;
}

/**
 * configureSensorInChannel: set new pollrate for a sensor in a specific channel
 * @sensorHandle: sensor handle.
 * @channelHandle: channel handle.
 * @pollrateNs: pollrate in nanoseconds.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsDataProxyManager::configureSensorInChannel(int32_t sensorHandle,
                                                      int32_t channelHandle,
                                                      int64_t pollrateNs)
{
    auto search_sensor = mSensorToChannel.find(sensorHandle);
    if (search_sensor == mSensorToChannel.end()) {
        return -EINVAL;
    }

    auto search_channel = search_sensor->second.find(channelHandle);
    if (search_channel == search_sensor->second.end()) {
        return -ENODEV;
    }

    struct PollrateSwitchData switchData;
    switchData.timestampOfChange = utils.getTime();
    switchData.pollrateNs = pollrateNs;

    search_channel->second.switchDataFifo.push_back(std::move(switchData));

    return 0;
}

/**
 * getMaxPollrateNs: get the maximum pollrate requested for a sensor
 * @sensorHandle: sensor handle.
 *
 * Return value: max pollrate in nanoseconds, 0 if sensor is disabled.
 */
int64_t SensorsDataProxyManager::getMaxPollrateNs(int32_t sensorHandle) const
{
    int64_t max = INT64_MAX;

    auto search_sensor = mSensorToChannel.find(sensorHandle);
    if (search_sensor == mSensorToChannel.end()) {
        return 0;
    }

    for (auto &ch : search_sensor->second) {
        const struct ProxyData &pdata = ch.second;
        int64_t pollrateNs;

        if (pdata.switchDataFifo.empty()) {
            pollrateNs = pdata.pollrateNs;
        } else {
            const struct PollrateSwitchData &switchDataLast = pdata.switchDataFifo.back();
            pollrateNs = switchDataLast.pollrateNs;
        }

        if ((max > pollrateNs) && (pollrateNs > 0)) {
            max = pollrateNs;
        }
    }

    return max == INT64_MAX ? 0 : max;
}

/**
 * getChannels: obtain list of channels where a sensor is currently registered on
 * @sensorHandle: sensor handle.
 *
 * Return value: list of channels handles.
 */
std::vector<int32_t> SensorsDataProxyManager::getChannels(int32_t sensorHandle) const
{
    std::vector<int32_t> channelsHandles;

    auto search_sensor = mSensorToChannel.find(sensorHandle);
    if (search_sensor == mSensorToChannel.end()) {
        return channelsHandles;
    }

    for (auto &ch : search_sensor->second) {
        channelsHandles.push_back(ch.first);
    }

    return channelsHandles;
}

/**
 * getRegisteredSensorsInChannel: obtain list of sensors currently registered on a channel
 * @channelHandle: channel handle.
 *
 * Return value: list of sensors handles.
 */
std::vector<int32_t> SensorsDataProxyManager::getRegisteredSensorsInChannel(int32_t channelHandle) const
{
    std::vector<int32_t> sensorsHandles;

    auto search_channel = mChannelToSensor.find(channelHandle);
    for (auto &sh : search_channel->second) {
        sensorsHandles.push_back(sh);
    }

    return sensorsHandles;
}

/**
 * getValidPushChannels: list if channels where the current sensor sample must be pushed to
 * @timestamp: timestamp of current sensor sample.
 * @sensorHandle: sensor handle.
 * @pollrateNs: current pollrate of the sensor stream.
 *
 * Return value: list of channels handles.
 */
std::vector<int32_t> SensorsDataProxyManager::getValidPushChannels(int64_t timestamp,
                                                                   int32_t sensorHandle,
                                                                   int64_t pollrateNs)
{
    std::vector<int32_t> channelsHandles;

    auto search_sensor = mSensorToChannel.find(sensorHandle);
    if (search_sensor == mSensorToChannel.end()) {
        return channelsHandles;
    }

    for (auto &ch : search_sensor->second) {
        struct ProxyData &pdata = ch.second;
        int oldDivisor = calculateDecimator(pdata.pollrateNs, pollrateNs);

        while (!pdata.switchDataFifo.empty()) {
            struct PollrateSwitchData &switchData = pdata.switchDataFifo.front();
            if (timestamp >= switchData.timestampOfChange) {
                pdata.pollrateNs = switchData.pollrateNs;
                pdata.switchDataFifo.pop_front();
            } else {
                break;
            }
        }

        int divisor = calculateDecimator(pdata.pollrateNs, pollrateNs);
        if (divisor) {
            if (oldDivisor != divisor) {
                pdata.samplesCounter = divisor - 1;
            }

            if (++pdata.samplesCounter >= divisor) {
                pdata.samplesCounter = 0;
                channelsHandles.push_back(ch.first);
            }
        }
    }

    return channelsHandles;
}

/**
 * calculateDecimator: calculate decimator factor
 * @dividend: dividend.
 * @divisor: divisor.
 *
 * Return value: >= 1 if divisor is greater than 0. 0 if divisor is also 0.
 */
int SensorsDataProxyManager::calculateDecimator(int64_t dividend, int64_t divisor) const
{
    if (!divisor) {
        return 0;
    }

    int quotient = dividend / divisor;
    if (quotient < 1) {
        quotient = 1;
    }

    return quotient;
}
