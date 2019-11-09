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

#include <android/hardware/sensors/2.0/ISensors.h>
#include <hardware_legacy/power.h>
#include <hidl/MQDescriptor.h>
#include <fmq/MessageQueue.h>
#include <hidl/Status.h>
#include <atomic>
#include <thread>

#include <ISTMSensorsHAL.h>
#include <IUtils.h>

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

using ::android::hardware::sensors::V1_0::Result;
using ::android::hardware::sensors::V1_0::Event;
using ::android::hardware::MQDescriptor;
using ::android::hardware::MessageQueue;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct SensorsHidlInterface : public ISensors, public ISTMSensorsCallback {
public:
    SensorsHidlInterface(void);
    ~SensorsHidlInterface(void);

    Return<void> getSensorsList(getSensorsList_cb _hidl_cb) override;

    Return<Result> setOperationMode(V1_0::OperationMode mode) override;

    Return<Result> activate(int32_t sensorHandle,
                            bool enabled) override;

    Return<Result> initialize(const MQDescriptorSync<Event>& eventQueueDescriptor,
                              const MQDescriptorSync<uint32_t>& wakeLockDescriptor,
                              const sp<V2_0::ISensorsCallback>& sensorsCallback) override;

    Return<Result> batch(int32_t sensorHandle,
                         int64_t samplingPeriodNs,
                         int64_t maxReportLatencyNs) override;

    Return<Result> flush(int32_t sensorHandle) override;

    Return<Result> injectSensorData(const Event& event) override;

    Return<void> registerDirectChannel(const V1_0::SharedMemInfo& mem,
                                       registerDirectChannel_cb _hidl_cb) override;

    Return<Result> unregisterDirectChannel(int32_t channelHandle) override;

    Return<void> configDirectReport(int32_t sensorHandle,
                                    int32_t channelHandle,
                                    V1_0::RateLevel rate,
                                    configDirectReport_cb _hidl_cb) override;

    void onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData) override;

    void postEvents(const std::vector<Event> &events, bool wakeup);

private:
    using EventMessageQueue = MessageQueue<Event, kSynchronizedReadWrite>;
    using WakeLockMessageQueue = MessageQueue<uint32_t, kSynchronizedReadWrite>;

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
     * Lock to protect writes to the FMQ
     */
    std::mutex mWriteLock;

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
     * Core library object interface
     */
    ISTMSensorsHAL &sensorsCore;

    /**
     * Print console
     */
    IConsole &console;

    void deleteEventFlag(void);

    static void startReadWakeLockThread(SensorsHidlInterface *sensors);

    void readWakeLockFMQ(void);

    void updateWakeLock(uint32_t eventsWritten, uint32_t eventsHandled);
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
