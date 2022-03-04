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

#include <android/hardware/sensors/2.0/types.h>
#include <memory>
#include <utility>

#include <IConsole.h>
#include "Convert.h"
#include "SensorsHidlInterface.h"
#include "AndroidPropertiesLoader.h"

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

constexpr const char *kWakeLockName = "STM_SensorsHAL_WAKELOCK";

constexpr int64_t hzToNs(int64_t hz) { return 1e9 / hz; };

SensorsHidlInterface::SensorsHidlInterface(void)
                     : mReadWakeLockQueueRun(false),
                       mOutstandingWakeUpEvents(0),
                       mHasWakeLock(false),
                       mEventQueueFlag(nullptr),
                       sensorsCore(ISTMSensorsHAL::getInstance()),
                       console(IConsole::getInstance()),
                       lastDirectChannelHandle(0),
                       propertiesManager(PropertiesManager::getInstance())
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
    const std::vector<stm::core::STMSensor> &list = sensorsCore.getSensorsList().getList();
    hidl_vec<V1_0::SensorInfo> sensorsList;
    size_t n = 0, count = list.size();

    sensorsList.resize(count);

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
    const stm::core::STMSensor *sensor = getSTMSensor(sensorHandle);
    int64_t maxReportLatencyNs = 0;
    int64_t samplingPeriodNs = 0;

    if (sensor == nullptr) {
        return Result::BAD_VALUE;
    }

    if (sensor->isOnChange()) {
        if (sensorsCore.activate(sensorHandle, enabled)) {
            return Result::BAD_VALUE;
        }
    } else {
        if (enabled) {
            samplingPeriodNs = frameworkRequestPollrateNs[sensorHandle];
            maxReportLatencyNs = frameworkRequestLatencyNs[sensorHandle];
            if (mSensorProxyMngr.registerSensorToChannel(sensorHandle, frameworkChHandle)) {
                return Result::BAD_VALUE;
            }
        }

        int ret = updateSensorsRequests(sensorHandle, frameworkChHandle, samplingPeriodNs, maxReportLatencyNs);
        if ((ret < 0) && (ret != -ENODEV)) {
            mSensorProxyMngr.unregisterSensorFromChannel(sensorHandle, frameworkChHandle);
            return Result::BAD_VALUE;
        }

        if (!enabled) {
            mSensorProxyMngr.unregisterSensorFromChannel(sensorHandle, frameworkChHandle);
        }
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

    std::lock_guard<std::mutex> lock(mInitLock);

    frameworkRequestPollrateNs.clear();
    frameworkRequestLatencyNs.clear();
    sensorCurrentPollrateNs.clear();

    AndroidPropertiesLoader propertiesLoader;
    propertiesManager.load(propertiesLoader);

    sensorsCore.initialize(*dynamic_cast<ISTMSensorsCallback *>(this));

    addInfoMng = std::make_unique<AdditionalInfoManager>(sensorsCore.getSensorsList());

    mSensorProxyMngr.reset();
    mSensorProxyMngr.addChannel(frameworkChHandle);

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
    const stm::core::STMSensor *sensor = getSTMSensor(sensorHandle);
    if (sensor == nullptr) {
        return Result::BAD_VALUE;
    }

    if (sensor->isOnChange()) {
        if (sensorsCore.setRate(sensorHandle, samplingPeriodNs, maxReportLatencyNs)) {
            return Result::BAD_VALUE;
        }
    } else {
        frameworkRequestPollrateNs[sensorHandle] = samplingPeriodNs;
        frameworkRequestLatencyNs[sensorHandle] = maxReportLatencyNs;

        auto channelsHandles = mSensorProxyMngr.getChannels(sensorHandle);
        for (auto &ch : channelsHandles) {
            if (ch == frameworkChHandle) {
              if (updateSensorsRequests(sensorHandle, frameworkChHandle, samplingPeriodNs, maxReportLatencyNs)) {
                return Result::BAD_VALUE;
              }
            }
        }
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
Return<void> SensorsHidlInterface::registerDirectChannel(const V1_0::SharedMemInfo &mem,
                                                         registerDirectChannel_cb _hidl_cb)
{
    std::unique_ptr<DirectChannelBufferBase> directChannelBuffer;

    // TODO verify memory region is valid

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

/**
 * unregisterDirectChannel: HIDL defined function,
 *                          reference: hardware/interfaces/sensors/2.0/ISensors.hal
 */
Return<Result> SensorsHidlInterface::unregisterDirectChannel(int32_t channelHandle)
{
    if (mSensorProxyMngr.removeChannel(channelHandle)) {
        return Result::BAD_VALUE;
    }

    std::lock_guard<std::mutex> lock(mDirectChannelBufferLock);
    mDirectChannelBuffer.erase(channelHandle);

    return Result::OK;
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

    const stm::core::STMSensor *sensor = getSTMSensor(sensorHandle);
    if (sensor == nullptr) {
        _hidl_cb(Result::BAD_VALUE, 0);
        return Void();
    }

    if (!(sensorFlags[sensorHandle] &
          (V1_0::SensorFlagBits::DIRECT_CHANNEL_ASHMEM | V1_0::SensorFlagBits::DIRECT_CHANNEL_GRALLOC))) {
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

/**
 * onNewSensorsData: receive data from STMSensorsHAL,
 *                   reference: ISTMSensorsCallbackData class
 */
void
SensorsHidlInterface::onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData)
{
    bool containsWakeUpEvents = false;
    std::vector<Event> eventsList;

    if (!mInitLock.try_lock()) {
        return;
    }

    eventsList.reserve(sensorsData.size());

    for (auto &sdata : sensorsData) {
        Event event;

        if (!convertFromSTMSensorType(sdata.getSensorType(), event.sensorType)) {
            if (sdata.getSensorType() != stm::core::SensorType::ODR_SWITCH_INFO) {
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

        if (sdata.getSensorType() == stm::core::SensorType::META_DATA) {
            eventsList.push_back(event);

            auto addInfoEvents = addInfoMng->getPayload(sdata.getSensorHandle(), event.timestamp);
            for (auto &el : addInfoEvents) {
                eventsList.push_back(el);
            }
        } else {
            const stm::core::STMSensor *sensor = getSTMSensor(sdata.getSensorHandle());
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

    mInitLock.unlock();
}

/**
 * onSaveDataRequest: store persistent data to disk
 * @resourceID: identifier of the resource.
 * @data: data pointer.
 * @len: data buffer size.
 *
 * Return value: number of bytes written on success, else a negative error code.
 */
int SensorsHidlInterface::onSaveDataRequest(const std::string& resourceID,
                                            const void *data, ssize_t len)
{
    (void) resourceID;
    (void) data;
    (void) len;

    return -EIO;
}

/**
 * onLoadDataRequest: load previously stored data from disk
 * @resourceID: identifier of the resource.
 * @data: data pointer.
 * @len: data buffer size.
 *
 * Return value: number of bytes read on success, else a negative error code.
 */
int SensorsHidlInterface::onLoadDataRequest(const std::string& resourceID,
                                            void *data, ssize_t len)
{
    (void) resourceID;
    (void) data;
    (void) len;

    return -EIO;
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

/**
 * updateSensorsRequests: manage the update requests from different channels
 * @sensorHandle: sensor handle.
 * @channelHandle: channel handle.
 * @samplingPeriodNs: requested pollrate in nanoseconds.
 * @maxReportLatencyNs: maximum latency of data reporting in nanoseconds.
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsHidlInterface::updateSensorsRequests(int32_t sensorHandle,
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
        if (sensorsCore.setRate(sensorHandle, maxPollrateNs, maxReportLatencyNs)) {
            return -1;
        }
        if (sensorsCore.activate(sensorHandle, true)) {
            return -1;
        }

        const stm::core::IUtils &utils = stm::core::IUtils::getInstance();
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

/**
 * getSTMSensor: return the pointer of an STMSensor object
 * @sensorHandle: sensor handle.
 *
 * Return value: STMSensor object pointer if sensorHandle is valid, else a nullptr.
 */
const stm::core::STMSensor *SensorsHidlInterface::getSTMSensor(int32_t sensorHandle) const
{
    const std::vector<stm::core::STMSensor> &list = sensorsCore.getSensorsList().getList();

    for (const auto &sensor : list) {
        if ((int32_t)sensor.getHandle() == sensorHandle) {
            return &sensor;
        }
    }

    return nullptr;
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
