/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <aidl/android/hardware/common/fmq/SynchronizedReadWrite.h>
#include <aidl/android/hardware/sensors/BnSensors.h>
#include <fmq/AidlMessageQueue.h>
#include <hardware_legacy/power.h>
#include <utils/Mutex.h>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <map>

#include <PropertiesManager.h>
#include <ISTMSensorsHAL.h>
#include <IUtils.h>

#include "DirectChannel.h"
#include "SensorsDataProxyManager.h"
#include "AdditionalInfoManager.h"

#include <utils/Mutex.h>
#include <unordered_map>
#include <atomic>
#include <thread>

namespace aidl {
namespace android {
namespace hardware {
namespace sensors {

using aidl::android::hardware::common::fmq::SynchronizedReadWrite;
using ::android::AidlMessageQueue;
using ::android::OK;
using ::android::status_t;
using ::android::hardware::EventFlag;

using stm::core::ISTMSensorsCallback;
using stm::core::ISTMSensorsCallbackData;
using stm::core::ISTMSensorsHAL;
using stm::core::IConsole;
using stm::core::PropertiesManager;

/*
This is a C++ code snippet defining a class called SensorsAidlInterface.

It inherits from two classes: BnSensors and ISensorsEventCallback.

The SensorsAidlInterface class has a constructor that initializes several member variables.
These variables include
 - a flag for the event queue, a handle for the next sensor event
 - a count of outstanding wake-up events
 - a boolean flag for running the wake lock queue
 - a time for auto-releasing the wake lock
 - a boolean flag indicating whether the wake lock is currently held

The constructor also calls the AddSensor function to add an instance of the
AccelSensor class to the sensor list.

The SensorsAidlInterface class likely represents a sensor service that provides access
to various sensors on a device.

The ISTMSensorsCallback interface is likely used to notify clients of
sensor events, while the BnSensors class provides the implementation of the
AIDL interface methods for the sensor service.
*/
class SensorsAidlInterface : public BnSensors, public ISTMSensorsCallback {
    static constexpr const char* kWakeLockName = "SensorsHAL_WAKEUP";

  public:
    SensorsAidlInterface();

    virtual ~SensorsAidlInterface();

    ::ndk::ScopedAStatus activate(int32_t in_sensorHandle, bool in_enabled) override;
    ::ndk::ScopedAStatus batch(int32_t in_sensorHandle, int64_t in_samplingPeriodNs,
                               int64_t in_maxReportLatencyNs) override;
    ::ndk::ScopedAStatus configDirectReport(
            int32_t in_sensorHandle, int32_t in_channelHandle,
            ::aidl::android::hardware::sensors::ISensors::RateLevel in_rate,
            int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus flush(int32_t in_sensorHandle) override;
    ::ndk::ScopedAStatus getSensorsList(
            std::vector<::aidl::android::hardware::sensors::SensorInfo>* _aidl_return) override;
    ::ndk::ScopedAStatus initialize(
            const ::aidl::android::hardware::common::fmq::MQDescriptor<
                    ::aidl::android::hardware::sensors::Event,
                    ::aidl::android::hardware::common::fmq::SynchronizedReadWrite>&
                    in_eventQueueDescriptor,
            const ::aidl::android::hardware::common::fmq::MQDescriptor<
                    int32_t, ::aidl::android::hardware::common::fmq::SynchronizedReadWrite>&
                    in_wakeLockDescriptor,
            const std::shared_ptr<::aidl::android::hardware::sensors::ISensorsCallback>&
                    in_sensorsCallback) override;
    ::ndk::ScopedAStatus injectSensorData(
            const ::aidl::android::hardware::sensors::Event& in_event) override;
    ::ndk::ScopedAStatus registerDirectChannel(
            const ::aidl::android::hardware::sensors::ISensors::SharedMemInfo& in_mem,
            int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus setOperationMode(
            ::aidl::android::hardware::sensors::ISensors::OperationMode in_mode) override;
    ::ndk::ScopedAStatus unregisterDirectChannel(int32_t in_channelHandle) override;

    void onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData) override;

    int onSaveDataRequest(const std::string& resourceID, const void *data, ssize_t len) override;

    int onLoadDataRequest(const std::string& resourceID, void *data, ssize_t len) override;

    void postEvents(const std::vector<Event> &events, bool wakeup);

  private:
    using EventMessageQueue = AidlMessageQueue<Event, SynchronizedReadWrite>;
    using WakeLockMessageQueue = AidlMessageQueue<int32_t, SynchronizedReadWrite>;

    /**
     * FMQ where sensor events are written into
     */
    std::unique_ptr<EventMessageQueue> mEventQueue;

    /**
     * FMQ where wake locks number of handled events are read from
     */
    std::unique_ptr<WakeLockMessageQueue> mWakeLockQueue;

    /**
     * Flag used to indicate if the wake lock thread needs to be stopped or keept running
     */
    std::atomic_bool mReadWakeLockQueueRun;

    /**
     * Thread used to read wake lock FMQ
     */
    std::thread mWakeLockThread;

    /**
     * Lock to make updateWakelock function exclusively executed
     */
    std::mutex mWakeLockLock;

    /**
     * Number of pending WAKE_UP events that the framework did not handled yet
     */
    uint32_t mOutstandingWakeUpEvents;

    /**
     * Timestamp value when the taken wake lock should be (auto)released because of timeout
     */
    int64_t mAutoReleaseWakeLockTime;

    /**
     * Flag used to indicate if the partial wake lock has been taken
     */
    bool mHasWakeLock;

    /**
     * Event flag used to signal the framework when sensors events are available
     * to be read from mEventQueue
     */
    EventFlag *mEventQueueFlag;

    /**
     * Callback for asynchronous events, such as dynamic sensor connections
     */
    std::shared_ptr<::aidl::android::hardware::sensors::ISensorsCallback> mCallback;

    /**
     * Lock to protect writes to the FMQ
     */
    std::mutex mWriteLock;

    /**
     * Lock to protect data push (and writes to the FMQ) during initialization
     */
    std::mutex mInitLock;

    /**
     * Core library object interface
     */
    ISTMSensorsHAL &sensorsCore;

    /**
     * Print console
     */
    IConsole &console;

    /**
     * Sensors additional info manager
     */
    std::unique_ptr<AdditionalInfoManager> addInfoMng;

    /**
     * Sensors proxy manager. Manage different requests on different channels down-sampling if necessary
     */
    SensorsDataProxyManager mSensorProxyMngr;

    /**
     * Identify the framework channel always with 0, direct report channels always uses > 0
     */
    static const int32_t frameworkChHandle = 0;

    /**
     * Last used  channel handle for direct report channel
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
     * map: sensorHandle -> sensor flags
     */
    std::unordered_map<int32_t, uint32_t> sensorFlags;

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

    std::mutex mLoadAndSaveLock;

    PropertiesManager& propertiesManager;

    const stm::core::STMSensor *getSTMSensor(int32_t sensorHandle) const;

    void deleteEventFlag(void);

    static void startReadWakeLockThread(SensorsAidlInterface *sensors);

    void readWakeLockFMQ(void);

    void updateWakeLock(uint32_t eventsWritten, uint32_t eventsHandled);

    int updateSensorsRequests(int32_t sensorHandle,
                              int32_t channelHandle,
                              int64_t samplingPeriodNs,
                              int64_t maxReportLatencyNs);
};

}  // namespace sensors
}  // namespace hardware
}  // namespace android
}  // namespace aidl
