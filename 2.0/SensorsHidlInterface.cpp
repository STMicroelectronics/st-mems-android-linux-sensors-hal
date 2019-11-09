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

#define LOG_TAG "android.hardware.sensors@2.0-Sensors"

#include <android/hardware/sensors/2.0/types.h>
#include <log/log.h>

#include "SensorsHidlInterface.h"

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

constexpr const char *kWakeLockName = "STM_SensorsHAL_WAKELOCK";

SensorsHidlInterface::SensorsHidlInterface(void)
                     : mReadWakeLockQueueRun(false),
                       mOutstandingWakeUpEvents(0),
                       mHasWakeLock(false),
                       mEventQueueFlag(nullptr)
{
    // TODO implement real scan of IIO sensors
}

SensorsHidlInterface::~SensorsHidlInterface(void)
{
    deleteEventFlag();
    mReadWakeLockQueueRun = false;
    mWakeLockThread.join();
}

/**
 * getSensorsList: HIDL defined function,
 *                 reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<void> SensorsHidlInterface::getSensorsList(getSensorsList_cb _hidl_cb)
{
    std::vector<V1_0::SensorInfo> sensorsList;

    _hidl_cb(sensorsList);

    return Void();
}

/**
 * setOperationMode: HIDL defined function,
 *                   reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::setOperationMode(V1_0::OperationMode mode)
{
    (void) mode;

    // TODO implement

    return Result::INVALID_OPERATION;
}

/**
 * activate: HIDL defined function,
 *           reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::activate(int32_t sensorHandle,
                                              bool enabled)
{
    (void) sensorHandle;
    (void) enabled;

    // TODO implement

    return Result::INVALID_OPERATION;
}

/**
 * initialize: HIDL defined function,
 *             reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::initialize(const MQDescriptorSync<Event>& eventQueueDescriptor,
                                                const MQDescriptorSync<uint32_t>& wakeLockDescriptor,
                                                const sp<ISensorsCallback>& sensorsCallback)
{
    (void) sensorsCallback;
    // TODO store sensorsCallback reference
    // TODO make sure all sensors are disabled

    if (mReadWakeLockQueueRun.load()) {
        mReadWakeLockQueueRun = false;
        mWakeLockThread.join();
    }

    mEventQueue = std::make_unique<EventMessageQueue>(eventQueueDescriptor, true /* resetPointers */);
    if ((mEventQueue == nullptr) || !mEventQueue->isValid()) {
        ALOGE("Failed to create MessageQueue for sensors events");
        return Result::BAD_VALUE;
    }

    if (mEventQueueFlag != nullptr) {
        if (EventFlag::deleteEventFlag(&mEventQueueFlag) != ::android::OK) {
            ALOGE("Failed to delete eventQueue flag");
            return Result::BAD_VALUE;
        }
    }

    if (EventFlag::createEventFlag(mEventQueue->getEventFlagWord(), &mEventQueueFlag) != ::android::OK) {
        ALOGE("Failed to create eventQueue flag");
        return Result::BAD_VALUE;
    }

    mWakeLockQueue = std::make_unique<WakeLockMessageQueue>(wakeLockDescriptor, true /* resetPointers */);
    if ((mWakeLockQueue == nullptr) || !mWakeLockQueue->isValid()) {
        ALOGE("Failed to create MessageQueue for wake locks");
        return Result::BAD_VALUE;
    }

    mReadWakeLockQueueRun = true;
    mWakeLockThread = std::thread(startReadWakeLockThread, this);

    return Result::OK;
}

/**
 * batch: HIDL defined function,
 *        reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::batch(int32_t sensorHandle,
                                           int64_t samplingPeriodNs,
                                           int64_t maxReportLatencyNs)
{
    (void) sensorHandle;
    (void) samplingPeriodNs;
    (void) maxReportLatencyNs;

    // TODO implement

    return Result::INVALID_OPERATION;
}

/**
 * flush: HIDL defined function,
 *        reference: hardware/interfaces/sensors/2.0/ISensors.Hal
 */
Return<Result> SensorsHidlInterface::flush(int32_t sensorHandle)
{
    (void) sensorHandle;

    // TODO implement

    return Result::INVALID_OPERATION;
}

/**
 * injectSensorData: HIDL defined function,
 *                   reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::injectSensorData(const Event& event)
{
    (void) event;

    // TODO implement

    return Result::INVALID_OPERATION;
}

/**
 * registerDirectChannel: HIDL defined function,
 *                        reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<void> SensorsHidlInterface::registerDirectChannel(const V1_0::SharedMemInfo& mem,
                                                         registerDirectChannel_cb _hidl_cb)
{
    (void) mem;

    _hidl_cb(Result::INVALID_OPERATION, -1);

    // TODO implement

    return Void();
}

/**
 * unregisterDirectChannel: HIDL defined function,
 *                          reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::unregisterDirectChannel(int32_t channelHandle)
{
    (void) channelHandle;

    // TODO implement

    return Result::INVALID_OPERATION;
}

/**
 * configDirectReport: HIDL defined function,
 *                     reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<void> SensorsHidlInterface::configDirectReport(int32_t sensorHandle,
                                                      int32_t channelHandle,
                                                      V1_0::RateLevel rate,
                                                      configDirectReport_cb _hidl_cb)
{
    (void) sensorHandle;
    (void) channelHandle;
    (void) rate;

    _hidl_cb(Result::INVALID_OPERATION, 0);

    // TODO implement

    return Void();
}

/**
 * deleteEventFlag: delete event queue flag (mEventQueueFlag)
 */
void SensorsHidlInterface::deleteEventFlag(void)
{
    ::android::status_t status = EventFlag::deleteEventFlag(&mEventQueueFlag);
    if (status != ::android::OK) {
        ALOGE("Failed to delete eventQueue flag (error: %d)", status);
    }
}

/**
 * updateWakeLock: update the number of WAKE_UP events that are still pending on framework side
 *                 after a read or write of FMQ has been performed, if needed,
 *                 acquire or release partial wake lock in order to make sure framework
 *                 manage the data before system goes to suspend
 * @eventsWritten: number of events that have been written by the caller.
 * @eventsHandled: number of events that have been read by the caller.
 */
void SensorsHidlInterface::updateWakeLock(uint32_t eventsWritten, uint32_t eventsHandled)
{
    std::lock_guard<std::mutex> lock(mWakeLockLock);

    int32_t newValue = mOutstandingWakeUpEvents + eventsWritten - eventsHandled;
    if (newValue <= 0) {
        newValue = 0;
    } else {
        mOutstandingWakeUpEvents = newValue;
    }

    if (eventsWritten > 0) {
        mAutoReleaseWakeLockTime = ::android::uptimeMillis() +
                                   static_cast<int32_t>(SensorTimeout::WAKE_LOCK_SECONDS) * 1000;
    }

    if (mHasWakeLock) {
        if (::android::uptimeMillis() > mAutoReleaseWakeLockTime) {
            ALOGD("No events read from wake lock FMQ for %d seconds, auto realizing wake lock",
                  SensorTimeout::WAKE_LOCK_SECONDS);
            mOutstandingWakeUpEvents = 0;
        }
        if ((mOutstandingWakeUpEvents == 0) && release_wake_lock(kWakeLockName)) {
            mHasWakeLock = false;
        }
    } else if ((mOutstandingWakeUpEvents > 0) &&
               (acquire_wake_lock(PARTIAL_WAKE_LOCK, kWakeLockName) == 0)) {
        mHasWakeLock = true;
    }
}

/**
 * readWakeLockFMQ: read the wake lock FMQ and update the events status
 */
void SensorsHidlInterface::readWakeLockFMQ(void)
{
    constexpr int64_t kRealTimeoutNs = 500 * 1000 * 1000;

    while (mReadWakeLockQueueRun.load()) {
        unsigned int eventsHandled = 0;

        if (mWakeLockQueue->readBlocking(&eventsHandled, 1, 0,
                                         static_cast<uint32_t>(WakeLockQueueFlagBits::DATA_WRITTEN),
                                         kRealTimeoutNs)) {
            updateWakeLock(0, eventsHandled);
        }
    }
}

/**
 * startReadWakeLockThread: thread entry point function for reading wake lock FMQ
 * @sensors: this class instance since the method needs to be static.
 */
void SensorsHidlInterface::startReadWakeLockThread(SensorsHidlInterface *sensors)
{
    sensors->readWakeLockFMQ();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android