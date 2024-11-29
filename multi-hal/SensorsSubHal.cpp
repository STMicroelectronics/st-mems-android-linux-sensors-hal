/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2022 STMicroelectronics
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

#include <memory>
#include <utility>
#include <fstream>

#include "SensorsSubHal.h"
#include "Convert.h"

#include <AndroidPropertiesLoader.h>

#ifdef HAL_MULTIHAL_2_0
::android::hardware::sensors::V2_0::implementation::ISensorsSubHal* sensorsHalGetSubHal(uint32_t *version)
{
    static ::android::hardware::sensors::stm::multihal::SensorsSubHalV2_0 subHal;
    *version = SUB_HAL_2_0_VERSION;

    return &subHal;
}
#endif /* HAL_MULTIHAL_2_0 */

#ifdef HAL_MULTIHAL_2_1
::android::hardware::sensors::V2_1::implementation::ISensorsSubHal* sensorsHalGetSubHal_2_1(uint32_t *version)
{
    static ::android::hardware::sensors::stm::multihal::SensorsSubHalV2_1 subHal;
    *version = SUB_HAL_2_1_VERSION;

    return &subHal;
}
#endif /* HAL_MULTIHAL_2_1 */

namespace android {
namespace hardware {
namespace sensors {
namespace stm {
namespace multihal {

constexpr int64_t hzToNs(int64_t hz) { return 1e9 / hz; };

template <class SubHalClass>
SensorsSubHalBase<SubHalClass>::SensorsSubHalBase()
    : sensorsCore(ISTMSensorsHAL::getInstance()),
      console(IConsole::getInstance()),
      initializedOnce(false),
      propertiesManager(PropertiesManager::getInstance())
{
    AndroidPropertiesLoader androidPropertiesLoader;
    propertiesManager.load(androidPropertiesLoader);
}

template <class SubHalClass>
Return<void> SensorsSubHalBase<SubHalClass>::getSensorsList_(V2_1::ISensors::getSensorsList_2_1_cb _hidl_cb)
{
    if (sensorsCore.initialize(*dynamic_cast<ISTMSensorsCallback *>(this))) {
        console.error("failed to initialize the core library");
        return Void();
    }

    const std::vector<::stm::core::STMSensor> &list = sensorsCore.getSensorsList().getList();
    hidl_vec<V2_1::SensorInfo> sensorsList(list.size());
    size_t n= 0, count = list.size();

    sensorsList.resize(count);
    addInfoMng = std::make_unique<AdditionalInfoManager>(sensorsCore.getSensorsList());

    for (size_t i = 0; i < count; i++) {
        if (convertFromSTMSensor(list.at(i), &sensorsList[n])) {
            if (addInfoMng->isSupported(sensorsList[n].sensorHandle)) {
                sensorsList[n].flags |= V1_0::SensorFlagBits::ADDITIONAL_INFO;
            }
            sensorFlags[sensorsList[n].sensorHandle] = sensorsList[n].flags;

            n++;
        }
    }

    if (n != count) {
        sensorsList.resize(n);
    }

    _hidl_cb(sensorsList);

    return Void();
}

template <class SubHalClass>
Return<V1_0::Result> SensorsSubHalBase<SubHalClass>::initialize(std::unique_ptr<IHalProxyCallbackWrapperBase>& halProxyCallback)
{
    mCallback = std::move(halProxyCallback);

    if (initializedOnce) {
        if (sensorsCore.initialize(*dynamic_cast<ISTMSensorsCallback *>(this))) {
            console.error("failed to initialize the core library");
            return V1_0::Result::BAD_VALUE;
        }

        initializedOnce = true;
    }

    frameworkRequestPollrateNs.clear();
    frameworkRequestLatencyNs.clear();
    sensorCurrentPollrateNs.clear();

    mSensorProxyMngr.reset();
    mSensorProxyMngr.addChannel(frameworkChHandle);

    return V1_0::Result::OK;
}

template <class SubHalClass>
Return<V1_0::Result> SensorsSubHalBase<SubHalClass>::injectSensorData(const V2_1::Event& event)
{
    (void) event;

    return V1_0::Result::INVALID_OPERATION;
}

template <class SubHalClass>
Return<V1_0::Result> SensorsSubHalBase<SubHalClass>::setOperationMode(V1_0::OperationMode mode)
{
    if (mode == V1_0::OperationMode::NORMAL) {
        return V1_0::Result::OK;
    }

    return V1_0::Result::BAD_VALUE;
}

template <class SubHalClass>
Return<V1_0::Result> SensorsSubHalBase<SubHalClass>::activate(int32_t sensorHandle, bool enabled)
{
    const ::stm::core::STMSensor *sensor = getSTMSensor(sensorHandle);
    int64_t maxReportLatencyNs = 0;
    int64_t samplingPeriodNs = 0;
    int ret;

    if (sensor == nullptr) {
        return V1_0::Result::BAD_VALUE;
    }

    if (sensor->isOnChange()) {
        if (sensorsCore.activate(sensorHandle, enabled)) {
            return Result::BAD_VALUE;
        }
    } else {
        if (enabled) {
            samplingPeriodNs = frameworkRequestPollrateNs[sensorHandle];
            maxReportLatencyNs = frameworkRequestLatencyNs[sensorHandle];

            ret = mSensorProxyMngr.registerSensorToChannel(sensorHandle, frameworkChHandle);
            if (ret) {
                return Result::BAD_VALUE;
            }
        }

        ret = updateSensorsRequests(sensorHandle, frameworkChHandle,
                                    samplingPeriodNs, maxReportLatencyNs);
        if ((ret < 0) && (ret != -ENODEV)) {
            mSensorProxyMngr.unregisterSensorFromChannel(sensorHandle, frameworkChHandle);
            return Result::BAD_VALUE;
        }

        if (!enabled) {
            mSensorProxyMngr.unregisterSensorFromChannel(sensorHandle, frameworkChHandle);
        }
    }

    return V1_0::Result::OK;
}

template <class SubHalClass>
Return<V1_0::Result> SensorsSubHalBase<SubHalClass>::batch(int32_t sensorHandle,
                                                           int64_t samplingPeriodNs,
                                                           int64_t maxReportLatencyNs)
{
    const ::stm::core::STMSensor *sensor = getSTMSensor(sensorHandle);
    if (sensor == nullptr) {
        return V1_0::Result::BAD_VALUE;
    }

    if (sensor->isOnChange()) {
        if (sensorsCore.setRate(sensorHandle, samplingPeriodNs,
                                maxReportLatencyNs)) {
            return Result::BAD_VALUE;
        }
    } else {
        frameworkRequestPollrateNs[sensorHandle] = samplingPeriodNs;
        frameworkRequestLatencyNs[sensorHandle] = maxReportLatencyNs;

        auto channelsHandles = mSensorProxyMngr.getChannels(sensorHandle);
        for (auto &ch : channelsHandles) {
            if (ch == frameworkChHandle) {
                if (updateSensorsRequests(sensorHandle, frameworkChHandle,
                                          samplingPeriodNs, maxReportLatencyNs)) {
                    return Result::BAD_VALUE;
                }
            }
        }
    }

    return V1_0::Result::OK;
}

template <class SubHalClass>
Return<V1_0::Result> SensorsSubHalBase<SubHalClass>::flush(int32_t sensorHandle)
{
    if (sensorsCore.flushData(sensorHandle)) {
        return V1_0::Result::BAD_VALUE;
    }

    return V1_0::Result::OK;
}

template <class SubHalClass>
Return<void> SensorsSubHalBase<SubHalClass>::registerDirectChannel(const V1_0::SharedMemInfo& mem,
                                                                   V2_0::ISensors::registerDirectChannel_cb _hidl_cb)
{
    std::unique_ptr<DirectChannelBufferBase> directChannelBuffer;

    switch (mem.type) {
    case V1_0::SharedMemType::ASHMEM:
        directChannelBuffer = std::make_unique<AshmemDirectChannelBuffer>(mem);
        break;
    case V1_0::SharedMemType::GRALLOC:
        directChannelBuffer = std::make_unique<GrallocDirectChannelBuffer>(mem);
        break;
    default:
        _hidl_cb(Result::BAD_VALUE, -1);
        return Void();
        break;
    }

    if (directChannelBuffer == nullptr) {
        _hidl_cb(Result::NO_MEMORY, -1);
        return Void();
    }
    if (!directChannelBuffer->isValid()) {
        _hidl_cb(Result::BAD_VALUE, -1);
        return Void();
    }

    int32_t channelHandle = lastDirectChannelHandle + 1;

    if (mSensorProxyMngr.addChannel(channelHandle)) {
        _hidl_cb(Result::BAD_VALUE, -1);
        return Void();
    }

    {
        std::lock_guard<std::mutex> lock(mDirectChannelBufferLock);
        mDirectChannelBuffer.insert(std::make_pair(channelHandle, std::move(directChannelBuffer)));
    }

    lastDirectChannelHandle = channelHandle;
    _hidl_cb(Result::OK, channelHandle);

    return Void();
}

template <class SubHalClass>
Return<V1_0::Result> SensorsSubHalBase<SubHalClass>::unregisterDirectChannel(int32_t channelHandle)
{
    if (mSensorProxyMngr.removeChannel(channelHandle)) {
        return Result::BAD_VALUE;
    }

    std::lock_guard<std::mutex> lock(mDirectChannelBufferLock);
    mDirectChannelBuffer.erase(channelHandle);

    return Result::OK;
}

template <class SubHalClass>
Return<void> SensorsSubHalBase<SubHalClass>::configDirectReport(int32_t sensorHandle,
                                                                int32_t channelHandle,
                                                                V1_0::RateLevel rate,
                                                                V2_0::ISensors::configDirectReport_cb _hidl_cb)
{
    if (channelHandle <= 0) {
        _hidl_cb(Result::BAD_VALUE, 0);

        return Void();
    }

    if (sensorHandle == -1) {
        if (rate == V1_0::RateLevel::STOP) {
            // stop all active sensors in that particular channel handle
            auto sensorsHandles = mSensorProxyMngr.getRegisteredSensorsInChannel(channelHandle);
            for (auto &sh : sensorsHandles) {
                updateSensorsRequests(sh, channelHandle, 0, 0);
                mSensorProxyMngr.unregisterSensorFromChannel(sh, channelHandle);
            }

            _hidl_cb(Result::OK, sensorHandle);
            return Void();
        } else {
            _hidl_cb(Result::BAD_VALUE, 0);
            return Void();
        }
    }

    const ::stm::core::STMSensor *sensor = getSTMSensor(sensorHandle);
    if (sensor == nullptr) {
        _hidl_cb(Result::BAD_VALUE, 0);
        return Void();
    }

    if (!(sensorFlags[sensorHandle] &
          (V1_0::SensorFlagBits::DIRECT_CHANNEL_ASHMEM |
           V1_0::SensorFlagBits::DIRECT_CHANNEL_GRALLOC))) {
        _hidl_cb(Result::BAD_VALUE, 0);
        return Void();
    }

    uint64_t rateInNs;

    int32_t supportedMaxRate = (sensorFlags[sensorHandle] & V1_0::SensorFlagBits::MASK_DIRECT_REPORT) >>
                               static_cast<uint8_t>(V1_0::SensorFlagShift::DIRECT_REPORT);

    switch (rate) {
    case V1_0::RateLevel::STOP:
        rateInNs = 0;
        break;
    case V1_0::RateLevel::NORMAL:
        rateInNs = hzToNs(50);
        break;
    case V1_0::RateLevel::FAST:
        if (supportedMaxRate < static_cast<int32_t>(V1_0::RateLevel::FAST)) {
            _hidl_cb(Result::BAD_VALUE, 0);
            return Void();
        }
        rateInNs = hzToNs(200);
        break;
    case V1_0::RateLevel::VERY_FAST:
        if (supportedMaxRate < static_cast<int32_t>(V1_0::RateLevel::VERY_FAST)) {
            _hidl_cb(Result::BAD_VALUE, 0);
            return Void();
        }
        rateInNs = hzToNs(800);
        break;
    default:
        _hidl_cb(Result::BAD_VALUE, 0);
        return Void();
    }

    if (rateInNs != 0) {
        mSensorProxyMngr.registerSensorToChannel(sensorHandle, channelHandle);
    }

    if (updateSensorsRequests(sensorHandle, channelHandle, rateInNs, 0)) {
        mSensorProxyMngr.unregisterSensorFromChannel(sensorHandle, channelHandle);
       _hidl_cb(Result::BAD_VALUE, 0);
        return Void();
    }

    if (rateInNs == 0) {
        mSensorProxyMngr.unregisterSensorFromChannel(sensorHandle, channelHandle);
    }

    _hidl_cb(Result::OK, sensorHandle);

    return Void();
}

template <class SubHalClass>
Return<void> SensorsSubHalBase<SubHalClass>::debug(const hidl_handle& fd,
                                                   const hidl_vec<hidl_string>& args)
{
    (void) fd;
    (void) args;

    return Void();
}

template <class SubHalClass>
void SensorsSubHalBase<SubHalClass>::onNewSensorsData(const std::vector<ISTMSensorsCallbackData>& sensorsData)
{
    std::vector<V2_1::Event> eventsList;
    bool containsWakeUpEvents = false;

    eventsList.reserve(sensorsData.size());

    for (auto& sdata : sensorsData) {
        V2_1::Event event;

        if (!convertFromSTMSensorType(sdata.getSensorType(), event.sensorType)) {
            if (sdata.getSensorType() != ::stm::core::SensorType::ODR_SWITCH_INFO) {
                console.error("sensor event unknown, discarding...");
            } else {
                sensorCurrentPollrateNs[sdata.getSensorHandle()] = sdata.getData()[0];
            }
            continue;
        }

        event.sensorHandle = sdata.getSensorHandle();
        event.timestamp = sdata.getTimestamp();
        containsWakeUpEvents |= sdata.isWakeUpSensor();
        convertFromSTMSensorData(sdata, event);

        if (sdata.getSensorType() == ::stm::core::SensorType::META_DATA) {
            eventsList.push_back(event);

            auto addInfoEvents = addInfoMng->getPayload(sdata.getSensorHandle(), event.timestamp);
            for (auto &el : addInfoEvents) {
                eventsList.push_back(el);
            }
        } else {
            const ::stm::core::STMSensor *sensor = getSTMSensor(sdata.getSensorHandle());
            if (sensor == nullptr) continue;

            if (sensor->isOnChange()) {
                eventsList.push_back(event);
            } else {
                auto channelsHandles =
                    mSensorProxyMngr.getValidPushChannels(sdata.getTimestamp(),
                                                          sdata.getSensorHandle(),
                                                          sensorCurrentPollrateNs[sdata.getSensorHandle()]);

                for (auto &channel : channelsHandles) {
                    if (channel == frameworkChHandle) {
                        eventsList.push_back(event);
                    } else {
                        if (mDirectChannelBufferLock.try_lock()) {
                            mDirectChannelBuffer[channel]->writeEvent(event);
                            mDirectChannelBufferLock.unlock();
                        }
                    }
                }
            }
        }
    }

    postEvents(eventsList, containsWakeUpEvents);
}

template <class SubHalClass>
int SensorsSubHalBase<SubHalClass>::onSaveDataRequest(const std::string& resourceID,
                                                      const void *data,
                                                      ssize_t len)
{
    std::string filename = std::string(HAL_PRIVATE_DATA_PATH) + "/" + resourceID;
    std::ofstream resourceFile;

    std::lock_guard<std::mutex> lock(mLoadAndSaveLock);

    resourceFile.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!resourceFile.is_open()) {
        return -ENOENT;
    }

    resourceFile.write(static_cast<const char *>(data), len);
    if (!resourceFile.good()) {
        resourceFile.close();
        return -EIO;
    }
    resourceFile.close();

    return len;
}

template <class SubHalClass>
int SensorsSubHalBase<SubHalClass>::onLoadDataRequest(const std::string& resourceID,
                                                      void *data,
                                                      ssize_t len)
{
    std::string filename = std::string(HAL_PRIVATE_DATA_PATH) + "/" + resourceID;
    std::ifstream resourceFile;

    std::lock_guard<std::mutex> lock(mLoadAndSaveLock);

    resourceFile.open(filename, std::ios::in | std::ios::binary);
    if (!resourceFile.is_open()) {
        return -ENOENT;
    }

    resourceFile.read(static_cast<char *>(data), len);
    if (!resourceFile.good()) {
        resourceFile.close();
        return -EIO;
    }

    resourceFile.close();

    return len;
}

template <class SubHalClass>
void SensorsSubHalBase<SubHalClass>::postEvents(const std::vector<V2_1::Event>& events, bool wakeup)
{
    V2_0::implementation::ScopedWakelock wakelock = mCallback->createScopedWakelock(wakeup);
    mCallback->postEvents(events, std::move(wakelock));
}

template <class SubHalClass>
const ::stm::core::STMSensor *SensorsSubHalBase<SubHalClass>::getSTMSensor(int32_t sensorHandle) const
{
    const std::vector<::stm::core::STMSensor>& list = sensorsCore.getSensorsList().getList();

    for (const auto &sensor : list) {
        if ((int32_t)sensor.getHandle() == sensorHandle) {
            return &sensor;
        }
    }

    return nullptr;
}

template <class SubHalClass>
Return<int> SensorsSubHalBase<SubHalClass>::updateSensorsRequests(int32_t sensorHandle,
                                                int32_t channelHandle,
                                                int64_t samplingPeriodNs,
                                                int64_t maxReportLatencyNs)
{
    int err = mSensorProxyMngr.configureSensorInChannel(sensorHandle, channelHandle, samplingPeriodNs);

    if (err < 0) {
        return -ENODEV;
    }

    int64_t maxPollrateNs = mSensorProxyMngr.getMaxPollrateNs(sensorHandle);

    if (maxPollrateNs) {
        // This is a power-on request or a pollrate change request
        if (int32_t ret = sensorsCore.setRate(sensorHandle, maxPollrateNs, maxReportLatencyNs)) {
            return -1;
        }

        if (int32_t ret = sensorsCore.activate(sensorHandle, true)) {
            return -1;
        }

        const ::stm::core::IUtils &utils = ::stm::core::IUtils::getInstance();
        auto addInfoEvents = addInfoMng->getPayload(sensorHandle, utils.getTime());
        if (addInfoEvents.size()) {
            postEvents(addInfoEvents, false);
        }
    } else {
        // This is a power-off request
        if (sensorsCore.activate(sensorHandle, false)) {
            return -1;
        }
    }

    return 0;
}

}  // namespace multihal
}  // namespace stm
}  // namespace sensors
}  // namespace hardware
}  // namespace android
