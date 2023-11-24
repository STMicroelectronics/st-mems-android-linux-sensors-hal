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

template <class SubHalClass>
SensorsSubHalBase<SubHalClass>::SensorsSubHalBase()
    : sensorsCore(ISTMSensorsHAL::getInstance()),
      console(IConsole::getInstance()),
      initializedOnce(false),
      propertiesManager(PropertiesManager::getInstance())
{

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

    for (size_t i = 0; i < count; i++) {
        if (convertFromSTMSensor(list.at(i), &sensorsList[n])) {
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

    AndroidPropertiesLoader propertiesLoader;
    propertiesManager.load(propertiesLoader);
    if (initializedOnce) {
        if (sensorsCore.initialize(*dynamic_cast<ISTMSensorsCallback *>(this))) {
            console.error("failed to initialize the core library");
            return V1_0::Result::BAD_VALUE;
        }

        initializedOnce = true;
    }

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
    if (sensor == nullptr) {
        return V1_0::Result::BAD_VALUE;
    }

    if (sensorsCore.activate(sensorHandle, enabled)) {
        return V1_0::Result::BAD_VALUE;
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

    if (sensorsCore.setRate(sensorHandle, samplingPeriodNs, maxReportLatencyNs)) {
        return V1_0::Result::BAD_VALUE;
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
    (void) mem;

    _hidl_cb(V1_0::Result::BAD_VALUE, -1);

    return Void();
}

template <class SubHalClass>
Return<V1_0::Result> SensorsSubHalBase<SubHalClass>::unregisterDirectChannel(int32_t channelHandle)
{
    (void) channelHandle;

    return V1_0::Result::BAD_VALUE;
}

template <class SubHalClass>
Return<void> SensorsSubHalBase<SubHalClass>::configDirectReport(int32_t sensorHandle,
                                                                int32_t channelHandle,
                                                                V1_0::RateLevel rate,
                                                                V2_0::ISensors::configDirectReport_cb _hidl_cb)
{
    (void) sensorHandle;
    (void) channelHandle;
    (void) rate;

    _hidl_cb(V1_0::Result::BAD_VALUE, 0);

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
            }
            continue;
        }

        event.sensorHandle = sdata.getSensorHandle();
        event.timestamp = sdata.getTimestamp();
        containsWakeUpEvents |= sdata.isWakeUpSensor();
        convertFromSTMSensorData(sdata, event);

        if (sdata.getSensorType() == ::stm::core::SensorType::META_DATA) {
            eventsList.push_back(event);
        } else {
            const ::stm::core::STMSensor *sensor = getSTMSensor(sdata.getSensorHandle());
            if (sensor == nullptr) continue;

            eventsList.push_back(event);
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

}  // namespace multihal
}  // namespace stm
}  // namespace sensors
}  // namespace hardware
}  // namespace android
