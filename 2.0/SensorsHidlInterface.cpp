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

#include <android/hardware/sensors/2.0/types.h>
#include <memory>

#include <IConsole.h>
#include "Convert.h"
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
                       mEventQueueFlag(nullptr),
                       sensorsCore(ISTMSensorsHAL::getInstance()),
                       console(IConsole::getInstance())
{
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
    const std::vector<STMSensor> &list = sensorsCore.getSensorsList().getList();
    hidl_vec<V1_0::SensorInfo> sensorsList;
    size_t n = 0, count = list.size();

    sensorsList.resize(count);

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

/**
 * setOperationMode: HIDL defined function,
 *                   reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::setOperationMode(V1_0::OperationMode mode)
{
    if (mode == V1_0::OperationMode::NORMAL) {
        return Result::OK;
    }

    return Result::BAD_VALUE;
}

/**
 * activate: HIDL defined function,
 *           reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::activate(int32_t sensorHandle,
                                              bool enabled)
{
    if (sensorsCore.activate(sensorHandle, enabled)) {
        return Result::BAD_VALUE;
    }

    return Result::OK;
}

/**
 * initialize: HIDL defined function,
 *             reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result>
SensorsHidlInterface::initialize(const MQDescriptorSync<Event>& eventQueueDescriptor,
                                 const MQDescriptorSync<uint32_t>& wakeLockDescriptor,
                                 const sp<ISensorsCallback>& sensorsCallback)
{
    (void) sensorsCallback;
    // TODO store sensorsCallback reference
    // TODO make sure all sensors are disabled

    sensorsCore.initialize(*dynamic_cast<ISTMSensorsCallback *>(this));

    if (mReadWakeLockQueueRun.load()) {
        mReadWakeLockQueueRun = false;
        mWakeLockThread.join();
    }

    mEventQueue = std::make_unique<EventMessageQueue>(eventQueueDescriptor,
                                                      true /* resetPointers */);
    if ((mEventQueue == nullptr) || !mEventQueue->isValid()) {
        console.error("failed to create MessageQueue for sensors events");
        return Result::BAD_VALUE;
    }

    if (mEventQueueFlag != nullptr) {
        if (EventFlag::deleteEventFlag(&mEventQueueFlag) != ::android::OK) {
            console.error("failed to delete eventQueue flag");
            return Result::BAD_VALUE;
        }
    }

    if (EventFlag::createEventFlag(mEventQueue->getEventFlagWord(),
                                   &mEventQueueFlag) != ::android::OK) {
        console.error("failed to create eventQueue flag");
        return Result::BAD_VALUE;
    }

    mWakeLockQueue = std::make_unique<WakeLockMessageQueue>(wakeLockDescriptor,
                                                            true /* resetPointers */);
    if ((mWakeLockQueue == nullptr) || !mWakeLockQueue->isValid()) {
        console.error("failed to create MessageQueue for wake locks");
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
    if (sensorsCore.setRate(sensorHandle, samplingPeriodNs, maxReportLatencyNs)) {
        return Result::BAD_VALUE;
    }

    return Result::OK;
}

/**
 * flush: HIDL defined function,
 *        reference: hardware/interfaces/sensors/2.0/ISensors.Hal
 */
Return<Result> SensorsHidlInterface::flush(int32_t sensorHandle)
{
    if (sensorsCore.flushData(sensorHandle)) {
        return Result::BAD_VALUE;
    }

    return Result::OK;
}

/**
 * injectSensorData: HIDL defined function,
 *                   reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::injectSensorData(const Event& event)
{
    (void) event;

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

    return Void();
}

/**
 * unregisterDirectChannel: HIDL defined function,
 *                          reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::unregisterDirectChannel(int32_t channelHandle)
{
    (void) channelHandle;

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

    return Void();
}

/**
 * onNewSensorsData: receive data from STMSensorsHAL,
 *                   reference: ISTMSensorsCallbackData class
 */
void
SensorsHidlInterface::onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData)
{
    bool containsWakeUpEvents = false;
    std::vector<Event> eventsList;

    eventsList.reserve(sensorsData.size());

    for (auto sdata : sensorsData) {
        Event event;

        if (!convertFromSTMSensorType(sdata.getSensorType(), event.sensorType)) {
            console.error("sensor event unknown, discarding...");
            continue;
        }

        event.sensorHandle = sdata.getSensorHandle();
        event.timestamp = sdata.getTimestamp();
        containsWakeUpEvents |= sdata.isWakeUpSensor();
        convertFromSTMSensorData(sdata, event);
        eventsList.push_back(event);
    }

    postEvents(eventsList, containsWakeUpEvents);
}

/**
 * postEvents: write sensors events to queue
 * @events: sensor events
 * @wakeup: true if it's a wakeup sensor
 */
void SensorsHidlInterface::postEvents(const std::vector<Event> &events, bool wakeup)
{
    std::lock_guard<std::mutex> lock(mWriteLock);

    if (mEventQueue->write(events.data(), events.size())) {
        mEventQueueFlag->wake(static_cast<uint32_t>(EventQueueFlagBits::READ_AND_PROCESS));

        if (wakeup) {
            updateWakeLock(events.size(), 0);
        }
    }
}

/**
 * deleteEventFlag: delete event queue flag (mEventQueueFlag)
 */
void SensorsHidlInterface::deleteEventFlag(void)
{
    ::android::status_t status = EventFlag::deleteEventFlag(&mEventQueueFlag);
    if (status != ::android::OK) {
        console.error("failed to delete eventQueue flag (error:" +
                      std::to_string(status) + ")");
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
            console.debug("no events read from wake lock FMQ for " +
                          std::to_string(static_cast<int32_t>(SensorTimeout::WAKE_LOCK_SECONDS)) +
                          " seconds, auto realizing wake lock");
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
