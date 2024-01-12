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

#pragma once

#include "V2_0/SubHal.h"
#include "V2_1/SubHal.h"
#include "convertV2_1.h"

#include <ISTMSensorsHAL.h>
#include <IConsole.h>
#include <IUtils.h>
#include <PropertiesManager.h>

#include "DirectChannel.h"
#include "SensorsDataProxyManager.h"
#include "HalProxyCallbackWrapper.h"
#include "AdditionalInfoManager.h"

namespace android {
namespace hardware {
namespace sensors {
namespace stm {
namespace multihal {

using ::stm::core::ISTMSensorsHAL;
using ::stm::core::ISTMSensorsCallback;
using ::stm::core::ISTMSensorsCallbackData;
using ::stm::core::IConsole;
using ::stm::core::PropertiesManager;
using ::android::hardware::sensors::V1_0::Result;

template <class SubHalClass>
class SensorsSubHalBase : public SubHalClass, public ISTMSensorsCallback {
protected:
    using SubHalClass::getSensorsList;
    using SubHalClass::initialize;
    using SubHalClass::injectSensorData;

    SensorsSubHalBase();

public:
    Return<void> getSensorsList_(V2_1::ISensors::getSensorsList_2_1_cb _hidl_cb);

    Return<V1_0::Result> initialize(std::unique_ptr<IHalProxyCallbackWrapperBase>& halProxyCallback);

    Return<V1_0::Result> injectSensorData(const V2_1::Event& event);

    Return<V1_0::Result> setOperationMode(V1_0::OperationMode mode) override;

    Return<V1_0::Result> activate(int32_t sensorHandle, bool enabled) override;

    Return<V1_0::Result> batch(int32_t sensorHandle,
                               int64_t samplingPeriodNs,
                               int64_t maxReportLatencyNs) override;

    Return<V1_0::Result> flush(int32_t sensorHandle) override;

    Return<void> registerDirectChannel(const V1_0::SharedMemInfo& mem,
                                       V2_0::ISensors::registerDirectChannel_cb _hidl_cb) override;

    Return<V1_0::Result> unregisterDirectChannel(int32_t channelHandle) override;

    Return<void> configDirectReport(int32_t sensorHandle,
                                    int32_t channelHandle,
                                    V1_0::RateLevel rate,
                                    V2_0::ISensors::configDirectReport_cb _hidl_cb) override;

    Return<void> debug(const hidl_handle& fd,
                       const hidl_vec<hidl_string>& args) override;

    const std::string getName() override {
        return "stm-sensors-multihal-2.X";
    };

    void onNewSensorsData(const std::vector<ISTMSensorsCallbackData>& sensorsData) override;

    int onSaveDataRequest(const std::string& resourceID, const void *data, ssize_t len) override;

    int onLoadDataRequest(const std::string& resourceID, void *data, ssize_t len) override;

private:
    ISTMSensorsHAL& sensorsCore;

    IConsole& console;

    std::mutex mLoadAndSaveLock;

    bool initializedOnce;

    std::unique_ptr<IHalProxyCallbackWrapperBase> mCallback;

    void postEvents(const std::vector<V2_1::Event>& events, bool wakeup);

    const ::stm::core::STMSensor *getSTMSensor(int32_t sensorHandle) const;

    PropertiesManager& propertiesManager;

    /**
     * Sensors additional info manager
     */
    std::unique_ptr<AdditionalInfoManager> addInfoMng;

    /**
     * Last used channel handle for direct report channel
     */
    int32_t lastDirectChannelHandle;

    /**
     * Mutex for the direct channel buffer map
     */
    std::mutex mDirectChannelBufferLock;

    /**
     * map: channelHandle -> directChannelBuffer
     */
    std::unordered_map<int32_t, std::unique_ptr<DirectChannelBufferBase>> mDirectChannelBuffer;

    /**
     * map: sensorHandle -> framework requested pollrate
     */
    std::unordered_map<int32_t, int64_t> frameworkRequestPollrateNs;

    /**
     * map: sensorHandle -> framework requested max latency
     */
    std::unordered_map<int32_t, int64_t> frameworkRequestLatencyNs;

    /**
     * map: sensorHandle -> current sensor configured stream pollrate
     */
    std::unordered_map<int32_t, int64_t> sensorCurrentPollrateNs;

    /**
     * map: sensorHandle -> sensor flags
     */
    std::unordered_map<int32_t, uint32_t> sensorFlags;

    /**
     * Sensors proxy manager. Manage different requests on different channels down-sampling if necessary
     */
    SensorsDataProxyManager mSensorProxyMngr;

    /**
     * Identify the framework channel always with 0, direct report channels always uses > 0
     */
    static const int32_t frameworkChHandle = 0;

    Return<int> updateSensorsRequests(int32_t sensorHandle,
                              int32_t channelHandle,
                              int64_t samplingPeriodNs,
                              int64_t maxReportLatencyNs);
};

class SensorsSubHalV2_0 : public SensorsSubHalBase<V2_0::implementation::ISensorsSubHal> {
    using SubHalBase = SensorsSubHalBase<V2_0::implementation::ISensorsSubHal>;

public:
    virtual Return<void> getSensorsList(V2_0::ISensors::getSensorsList_cb _hidl_cb) override {
        return SubHalBase::getSensorsList_([&](const auto& list) {
            _hidl_cb(V2_1::implementation::convertToOldSensorInfos(list));
        });
    };

    Return<V1_0::Result> injectSensorData(const V1_0::Event& event) override {
        return SubHalBase::injectSensorData(V2_1::implementation::convertToNewEvent(event));
    };

    Return<V1_0::Result> initialize(const sp<V2_0::implementation::IHalProxyCallback>& halProxyCallback) override {
        std::unique_ptr<IHalProxyCallbackWrapperBase> wrapper = std::make_unique<HalProxyCallbackWrapperV2_0>(halProxyCallback);
        return SubHalBase::initialize(wrapper);
    };
};

class SensorsSubHalV2_1 : public SensorsSubHalBase<V2_1::implementation::ISensorsSubHal> {
    using SubHalBase = SensorsSubHalBase<V2_1::implementation::ISensorsSubHal>;

public:
    Return<void> getSensorsList_2_1(V2_1::ISensors::getSensorsList_2_1_cb _hidl_cb) override {
        return SubHalBase::getSensorsList_(_hidl_cb);
    };

    Return<V1_0::Result> injectSensorData_2_1(const V2_1::Event& event) override {
        return SubHalBase::injectSensorData(event);
    };

    Return<V1_0::Result> initialize(const sp<V2_1::implementation::IHalProxyCallback>& halProxyCallback) override {
        std::unique_ptr<IHalProxyCallbackWrapperBase> wrapper = std::make_unique<HalProxyCallbackWrapperV2_1>(halProxyCallback);
        return SubHalBase::initialize(wrapper);
    };
};

}  // namespace multihal
}  // namespace stm
}  // namespace sensors
}  // namespace hardware
}  // namespace android
