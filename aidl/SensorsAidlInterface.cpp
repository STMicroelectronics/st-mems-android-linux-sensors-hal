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

#include <memory>
#include <utility>
#include <fstream>

#include <IConsole.h>

#include "Convert.h"
#include "SensorsAidlInterface.h"
#include "AndroidPropertiesLoader.h"

#include <aidl/android/hardware/common/fmq/SynchronizedReadWrite.h>

using ::aidl::android::hardware::common::fmq::MQDescriptor;
using ::aidl::android::hardware::common::fmq::SynchronizedReadWrite;
using ::aidl::android::hardware::sensors::Event;
using ::aidl::android::hardware::sensors::ISensors;
using ::aidl::android::hardware::sensors::ISensorsCallback;
using ::aidl::android::hardware::sensors::SensorInfo;
using ::ndk::ScopedAStatus;

namespace aidl {
namespace android {
namespace hardware {
namespace sensors {

constexpr int64_t hzToNs(int64_t hz) { return 1e9 / hz; };

SensorsAidlInterface::SensorsAidlInterface()
                     : mReadWakeLockQueueRun(false),
                       mOutstandingWakeUpEvents(0),
                       mHasWakeLock(false),
                       mEventQueueFlag(nullptr),
                       sensorsCore(ISTMSensorsHAL::getInstance()),
                       console(IConsole::getInstance()),
                       lastDirectChannelHandle(0),
                       propertiesManager(PropertiesManager::getInstance()) {
}

SensorsAidlInterface::~SensorsAidlInterface()
{
    deleteEventFlag();
    mReadWakeLockQueueRun = false;
    mWakeLockThread.join();
}

ScopedAStatus SensorsAidlInterface::activate(int32_t in_sensorHandle, bool in_enabled)
{
    const stm::core::STMSensor *sensor = getSTMSensor(in_sensorHandle);
    int64_t maxReportLatencyNs = 0;
    int64_t samplingPeriodNs = 0;

    if (sensor == nullptr) {
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (sensor->isOnChange()) {
        if (sensorsCore.activate(in_sensorHandle, in_enabled)) {
            return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
    } else {
        if (in_enabled) {
            samplingPeriodNs = frameworkRequestPollrateNs[in_sensorHandle];
            maxReportLatencyNs = frameworkRequestLatencyNs[in_sensorHandle];
            if (mSensorProxyMngr.registerSensorToChannel(in_sensorHandle, frameworkChHandle)) {
                return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
            }
        }

        int ret = updateSensorsRequests(in_sensorHandle, frameworkChHandle,
                                        samplingPeriodNs, maxReportLatencyNs);
        if ((ret < 0) && (ret != -ENODEV)) {
            mSensorProxyMngr.unregisterSensorFromChannel(in_sensorHandle, frameworkChHandle);
            return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }

        if (!in_enabled) {
            mSensorProxyMngr.unregisterSensorFromChannel(in_sensorHandle, frameworkChHandle);
        }
    }

    return ScopedAStatus::ok();
}

ScopedAStatus SensorsAidlInterface::batch(int32_t in_sensorHandle, int64_t in_samplingPeriodNs,
                                          int64_t in_maxReportLatencyNs)
{
    const stm::core::STMSensor *sensor = getSTMSensor(in_sensorHandle);
    if (sensor == nullptr) {
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (sensor->isOnChange()) {
        if (sensorsCore.setRate(in_sensorHandle, in_samplingPeriodNs, in_maxReportLatencyNs)) {
            return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
    } else {
        frameworkRequestPollrateNs[in_sensorHandle] = in_samplingPeriodNs;
        frameworkRequestLatencyNs[in_sensorHandle] = in_maxReportLatencyNs;

        auto channelsHandles = mSensorProxyMngr.getChannels(in_sensorHandle);
        for (auto &ch : channelsHandles) {
            if (ch == frameworkChHandle) {
              if (updateSensorsRequests(in_sensorHandle, frameworkChHandle,
                                        in_samplingPeriodNs, in_maxReportLatencyNs)) {
                return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
              }
            }
        }
    }

    return ScopedAStatus::ok();
}

ScopedAStatus SensorsAidlInterface::flush(int32_t in_sensorHandle)
{
    if (sensorsCore.flushData(in_sensorHandle)) {
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    return ScopedAStatus::ok();
}

ScopedAStatus SensorsAidlInterface::getSensorsList(std::vector<SensorInfo>* _aidl_return)
{
    const std::vector<stm::core::STMSensor> &list = sensorsCore.getSensorsList().getList();
    std::vector<SensorInfo> sensorsList;
    size_t n = 0, count = list.size();

    sensorsList.resize(count);

    for (size_t i = 0; i < count; i++) {
        if (::android::hardware::sensors::implementation::convertFromSTMSensor(list.at(i), &sensorsList[n])) {
            if (addInfoMng->isSupported(sensorsList[n].sensorHandle)) {
                sensorsList[n].flags |= SensorInfo::SENSOR_FLAG_BITS_ADDITIONAL_INFO;
            }
            sensorFlags[sensorsList[n].sensorHandle] = sensorsList[n].flags;
            n++;
        }
    }

    if (n != count) {
        sensorsList.resize(n);
    }

    for (auto& sensor : sensorsList) {
        _aidl_return->push_back(sensor);
    }

    return ScopedAStatus::ok();
}

ScopedAStatus SensorsAidlInterface::initialize(
        const MQDescriptor<Event, SynchronizedReadWrite>& in_eventQueueDescriptor,
        const MQDescriptor<int32_t, SynchronizedReadWrite>& in_wakeLockDescriptor,
        const std::shared_ptr<::aidl::android::hardware::sensors::ISensorsCallback>&in_sensorsCallback)
{
    // TODO store sensorsCallback reference
    (void) in_sensorsCallback;

    std::lock_guard<std::mutex> lock(mInitLock);

    frameworkRequestPollrateNs.clear();
    frameworkRequestLatencyNs.clear();
    sensorCurrentPollrateNs.clear();

    AndroidPropertiesLoader propertiesLoader;
    propertiesManager.load(propertiesLoader);

    if (sensorsCore.initialize(*dynamic_cast<ISTMSensorsCallback *>(this))) {
        console.error("failed to initialize the core library");
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    addInfoMng = std::make_unique<AdditionalInfoManager>(sensorsCore.getSensorsList());

    mSensorProxyMngr.reset();
    mSensorProxyMngr.addChannel(frameworkChHandle);

    if (mReadWakeLockQueueRun.load()) {
        mReadWakeLockQueueRun = false;
        mWakeLockThread.join();
    }

    /* resetPointers */
    mEventQueue = std::make_unique<EventMessageQueue>(in_eventQueueDescriptor, true);
    if ((mEventQueue == nullptr) || !mEventQueue->isValid()) {
        console.error("failed to create MessageQueue for sensors events");
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (mEventQueueFlag != nullptr) {
        if (EventFlag::deleteEventFlag(&mEventQueueFlag) != ::android::OK) {
            console.error("failed to delete eventQueue flag");
            return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
    }

    if (EventFlag::createEventFlag(mEventQueue->getEventFlagWord(),
                                   &mEventQueueFlag) != ::android::OK) {
        console.error("failed to create eventQueue flag");
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    mWakeLockQueue = std::make_unique<WakeLockMessageQueue>(in_wakeLockDescriptor,
                                                            true /* resetPointers */);
    if ((mWakeLockQueue == nullptr) || !mWakeLockQueue->isValid()) {
        console.error("failed to create MessageQueue for wake locks");
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    mReadWakeLockQueueRun = true;
    mWakeLockThread = std::thread(startReadWakeLockThread, this);

    return ScopedAStatus::ok();
}

ScopedAStatus SensorsAidlInterface::injectSensorData(const Event& in_event)
{
    (void)in_event;

    return ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ScopedAStatus SensorsAidlInterface::configDirectReport(int32_t in_sensorHandle,
                                          int32_t in_channelHandle,
                                          ISensors::RateLevel in_rate,
                                          int32_t* _aidl_return)
{
    if (in_sensorHandle == -1) {
        if (in_rate == ISensors::RateLevel::STOP) {
            // stop all active sensors in that particular channel handle
            auto sensorsHandles = mSensorProxyMngr.getRegisteredSensorsInChannel(in_channelHandle);
            for (auto &sh : sensorsHandles) {
                updateSensorsRequests(sh, in_channelHandle, 0, 0);
                mSensorProxyMngr.unregisterSensorFromChannel(sh, in_channelHandle);
            }
            *_aidl_return = in_sensorHandle;
            return ScopedAStatus::ok();
        } else {
            *_aidl_return = 0;

            return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
    }

    const stm::core::STMSensor *sensor = getSTMSensor(in_sensorHandle);
    if (sensor == nullptr) {
        *_aidl_return = 0;

        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (!(sensorFlags[in_sensorHandle] &
          (SensorInfo::SENSOR_FLAG_BITS_DIRECT_CHANNEL_ASHMEM | SensorInfo::SENSOR_FLAG_BITS_DIRECT_CHANNEL_GRALLOC))) {
        *_aidl_return = 0;

        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    uint64_t rateInNs;

    int32_t supportedMaxRate = (sensorFlags[in_sensorHandle] & SensorInfo::SENSOR_FLAG_BITS_MASK_DIRECT_REPORT) >>
                               static_cast<uint8_t>(SensorInfo::SENSOR_FLAG_SHIFT_DIRECT_REPORT);

    switch (in_rate) {
    case ISensors::RateLevel::STOP:
        rateInNs = 0;
        break;
    case ISensors::RateLevel::NORMAL:
        rateInNs = hzToNs(50);
        break;
    case ISensors::RateLevel::FAST:
        if (supportedMaxRate < static_cast<int32_t>(ISensors::RateLevel::FAST)) {
            *_aidl_return = 0;

            return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
        rateInNs = hzToNs(200);
        break;
    case ISensors::RateLevel::VERY_FAST:
        if (supportedMaxRate < static_cast<int32_t>(ISensors::RateLevel::VERY_FAST)) {
            *_aidl_return = 0;

            return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
        rateInNs = hzToNs(800);
        break;
    default:
        *_aidl_return = 0;

        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (rateInNs != 0)
        mSensorProxyMngr.registerSensorToChannel(in_sensorHandle, in_channelHandle);

    if (updateSensorsRequests(in_sensorHandle, in_channelHandle, rateInNs, 0)) {
        mSensorProxyMngr.unregisterSensorFromChannel(in_sensorHandle, in_channelHandle);
         *_aidl_return = 0;

        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (rateInNs == 0)
        mSensorProxyMngr.unregisterSensorFromChannel(in_sensorHandle, in_channelHandle);

    *_aidl_return = in_sensorHandle;

    return ScopedAStatus::ok();
}

ScopedAStatus SensorsAidlInterface::registerDirectChannel(const ISensors::SharedMemInfo& in_mem,
                                                          int32_t* _aidl_return)
{
    std::unique_ptr<DirectChannelBufferBase> directChannelBuffer;

    // TODO verify memory region is valid
    switch (in_mem.type) {
    case ISensors::SharedMemInfo::SharedMemType::ASHMEM:
        directChannelBuffer = std::make_unique<AshmemDirectChannelBuffer>(in_mem);
        break;
    case ISensors::SharedMemInfo::SharedMemType::GRALLOC:
        directChannelBuffer = std::make_unique<GrallocDirectChannelBuffer>(in_mem);
        break;
    default:
        *_aidl_return = EX_ILLEGAL_ARGUMENT;
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (directChannelBuffer == nullptr) {
        *_aidl_return = EX_NULL_POINTER;
        return ScopedAStatus::fromExceptionCode(EX_NULL_POINTER);
    }
    if (!directChannelBuffer->isValid()) {
        *_aidl_return = EX_ILLEGAL_ARGUMENT;
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    int32_t channelHandle = lastDirectChannelHandle + 1;

    if (mSensorProxyMngr.addChannel(channelHandle)) {
        *_aidl_return = EX_ILLEGAL_ARGUMENT;
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    {
        std::lock_guard<std::mutex> lock(mDirectChannelBufferLock);
        mDirectChannelBuffer.insert(std::make_pair(channelHandle, std::move(directChannelBuffer)));
    }

    lastDirectChannelHandle = channelHandle;

    *_aidl_return = lastDirectChannelHandle;

    return ScopedAStatus::ok();
}

ScopedAStatus SensorsAidlInterface::unregisterDirectChannel(int32_t in_channelHandle)
{
    if (mSensorProxyMngr.removeChannel(in_channelHandle)) {
        return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    std::lock_guard<std::mutex> lock(mDirectChannelBufferLock);
    mDirectChannelBuffer.erase(in_channelHandle);

    return ScopedAStatus::ok();
}

ScopedAStatus SensorsAidlInterface::setOperationMode(OperationMode mode)
{
    if (mode == OperationMode::NORMAL)
        return ScopedAStatus::ok();

    return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
}

/**
 * onNewSensorsData: receive data from STMSensorsHAL,
 *                   reference: ISTMSensorsCallbackData class
 */
void
SensorsAidlInterface::onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData)
{
    bool containsWakeUpEvents = false;
    std::vector<Event> eventsList;

    if (!mInitLock.try_lock()) {
        return;
    }

    eventsList.reserve(sensorsData.size());

    for (auto &sdata : sensorsData) {
        Event event;

        if (!::android::hardware::sensors::implementation::convertFromSTMSensorType(sdata.getSensorType(), event.sensorType)) {
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
        ::android::hardware::sensors::implementation::convertFromSTMSensorData(sdata, event);

        if (sdata.getSensorType() == stm::core::SensorType::META_DATA) {
            eventsList.push_back(event);

            auto addInfoEvents = addInfoMng->getPayload(sdata.getSensorHandle(), event.timestamp);
            for (auto &el : addInfoEvents) {
                eventsList.push_back(el);
            }
        } else {
            const stm::core::STMSensor *sensor = getSTMSensor(sdata.getSensorHandle());
            if (sensor == nullptr)
                continue;

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
int SensorsAidlInterface::onSaveDataRequest(const std::string& resourceID,
                                            const void *data, ssize_t len)
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

/**
 * onLoadDataRequest: load previously stored data from disk
 * @resourceID: identifier of the resource.
 * @data: data pointer.
 * @len: data buffer size.
 *
 * Return value: number of bytes read on success, else a negative error code.
 */
int SensorsAidlInterface::onLoadDataRequest(const std::string& resourceID,
                                            void *data, ssize_t len)
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

/**
 * postEvents: write sensors events to queue
 * @events: sensor events
 * @wakeup: true if it's a wakeup sensor
 */
void SensorsAidlInterface::postEvents(const std::vector<Event> &events, bool wakeup)
{
    std::lock_guard<std::mutex> lock(mWriteLock);

    size_t numToWrite = events.size();

    do {
        if (mEventQueue->write(events.data(), events.size())) {
            mEventQueueFlag->wake(static_cast<uint32_t>(BnSensors::EVENT_QUEUE_FLAG_BITS_READ_AND_PROCESS));

            if (wakeup) {
               updateWakeLock(events.size(), 0);
            }
        }

        if (events.size() > mEventQueue->availableToWrite())
            numToWrite = events.size() - mEventQueue->availableToWrite();
        else
            numToWrite = 0;
    } while (numToWrite > 0);
}

/**
 * deleteEventFlag: Utility function to delete the Event Flag
 */
void SensorsAidlInterface::deleteEventFlag()
{
    if (mEventQueueFlag != nullptr) {
        status_t status = EventFlag::deleteEventFlag(&mEventQueueFlag);
        if (status != OK) {
          ALOGI("Failed to delete event flag: %d", status);
        }
    }
}

/**
 * startReadWakeLockThread: thread entry point function for reading wake lock FMQ
 * @sensors: this class instance since the method needs to be static.
 */
void SensorsAidlInterface::startReadWakeLockThread(SensorsAidlInterface* sensors)
{
    sensors->readWakeLockFMQ();
}

// Function to read the Wake Lock FMQ and release the wake lock when appropriate
void SensorsAidlInterface::readWakeLockFMQ()
{
    while (mReadWakeLockQueueRun.load()) {
        constexpr int64_t kReadTimeoutNs = 500 * 1000 * 1000;  // 500 ms
        int32_t eventsHandled = 0;

        // Read events from the Wake Lock FMQ. Timeout after a reasonable amount of time to
        // ensure that any held wake lock is able to be released if it is held for too long.
        mWakeLockQueue->readBlocking(
            &eventsHandled, 1 /* count */, 0 /* readNotification */,
            static_cast<uint32_t>(WAKE_LOCK_QUEUE_FLAG_BITS_DATA_WRITTEN), kReadTimeoutNs);
            updateWakeLock(0 /* eventsWritten */, eventsHandled);
    }
}

/**
 * Responsible for acquiring and releasing a wake lock when there are unhandled WAKE_UP events
 */
void SensorsAidlInterface::updateWakeLock(uint32_t eventsWritten, uint32_t eventsHandled)
{
    std::lock_guard<std::mutex> lock(mWakeLockLock);
    int32_t newVal = mOutstandingWakeUpEvents + eventsWritten - eventsHandled;

    if (newVal < 0) {
        mOutstandingWakeUpEvents = 0;
    } else {
        mOutstandingWakeUpEvents = newVal;
    }

    if (eventsWritten > 0) {
        // Update the time at which the last WAKE_UP event was sent
        mAutoReleaseWakeLockTime = ::android::uptimeMillis() +
                                       BnSensors::WAKE_LOCK_TIMEOUT_SECONDS * 1000;
    }

    if (!mHasWakeLock && mOutstandingWakeUpEvents > 0 &&
        acquire_wake_lock(PARTIAL_WAKE_LOCK, kWakeLockName) == 0) {
        mHasWakeLock = true;
        } else if (mHasWakeLock) {
        // Check if the wake lock should be released automatically if
        // SensorTimeout::WAKE_LOCK_SECONDS has elapsed since the last WAKE_UP event was written
        // to the Wake Lock FMQ.
        if (::android::uptimeMillis() > mAutoReleaseWakeLockTime) {
            ALOGD("No events read from wake lock FMQ for %d seconds, auto releasing wake lock",
                  WAKE_LOCK_TIMEOUT_SECONDS);
            mOutstandingWakeUpEvents = 0;
        }

        if (mOutstandingWakeUpEvents == 0 && release_wake_lock(kWakeLockName) == 0) {
            mHasWakeLock = false;
        }
    }
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
int SensorsAidlInterface::updateSensorsRequests(int32_t sensorHandle,
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
const stm::core::STMSensor *SensorsAidlInterface::getSTMSensor(int32_t sensorHandle) const
{
    const std::vector<stm::core::STMSensor> &list = sensorsCore.getSensorsList().getList();

    for (const auto &sensor : list) {
        if ((int32_t)sensor.getHandle() == sensorHandle) {
            return &sensor;
        }
    }

    return nullptr;
}

}  // namespace sensors
}  // namespace hardware
}  // namespace android
}  // namespace aidl
