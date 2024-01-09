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

#include <cutils/ashmem.h>
#include <sys/mman.h>

#include "DirectChannel.h"

using ::android::hardware::sensors::V1_0::SensorsEventFormatOffset;

/**
 * isValid: query the direct channel about its own validity
 *
 * Return value: true if valid, false otherwise.
 */
bool DirectChannelBufferBase::isValid(void) const
{
    return valid;
}

/**
 * writeEvent: write single sensor event to the direct channel
 * @event: sensor event to write.
 */
void DirectChannelBufferBase::writeEvent(const Event &event)
{
    mBuffer->write(event);
}

AshmemDirectChannelBuffer::AshmemDirectChannelBuffer(const SharedMemInfo &mem)
    : mFd(mem.memoryHandle->data[0])
{
    if ((mem.size <= 0) || (mFd <= 0)) {
        return;
    }

    void *mBase = mmap(nullptr, mem.size, PROT_WRITE, MAP_SHARED, mFd, 0);
    if (mBase == MAP_FAILED) {
        return;
    }

    mBuffer = std::make_unique<LockfreeBuffer>(mBase, mem.size);
    if (mBuffer == nullptr) {
        close(mFd);
        return;
    }

    valid = true;
}

AshmemDirectChannelBuffer::~AshmemDirectChannelBuffer(void)
{
    if (valid) {
        close(mFd);
    }
}

GrallocDirectChannelBuffer::GrallocDirectChannelBuffer(const SharedMemInfo &mem)
{
    (void) mem;

    // TODO implement gralloc buffer
}

GrallocDirectChannelBuffer::~GrallocDirectChannelBuffer(void)
{

}

LockfreeBuffer::LockfreeBuffer(void *buf, size_t size)
    : mData(buf),
      mSize(size / static_cast<int>(SensorsEventFormatOffset::TOTAL_LENGTH)),
      mWritePos(0),
      mCounter(1)
{
    memset(mData, 0, size);
}

/**
 * write: write a sensor event to the shared memory region
 * @ev: sensor event to write.
 */
void LockfreeBuffer::write(const Event &ev)
{
    static const int sensorEventSize = static_cast<int>(SensorsEventFormatOffset::TOTAL_LENGTH);

    uint8_t *currentSensorEvent  = &((uint8_t *)mData)[mWritePos * sensorEventSize];

    *((int32_t *)(currentSensorEvent +
                  static_cast<uint8_t>(SensorsEventFormatOffset::SIZE_FIELD))) = sensorEventSize;

    *((int32_t *)(currentSensorEvent +
                  static_cast<uint8_t>(SensorsEventFormatOffset::REPORT_TOKEN))) = ev.sensorHandle;

    *((int32_t *)(currentSensorEvent +
                  static_cast<uint8_t>(SensorsEventFormatOffset::SENSOR_TYPE))) = static_cast<int>(ev.sensorType);

    *((int64_t *)(currentSensorEvent +
                  static_cast<uint8_t>(SensorsEventFormatOffset::TIMESTAMP))) = ev.timestamp;

    uint8_t *data = currentSensorEvent + static_cast<uint8_t>(SensorsEventFormatOffset::DATA);
    memcpy(data, &ev.u.data[0], sizeof(android::hardware::sensors::V1_0::EventPayload));

    std::atomic_thread_fence(std::memory_order_release);
    *((uint32_t *)(currentSensorEvent + static_cast<uint8_t>(SensorsEventFormatOffset::ATOMIC_COUNTER))) = mCounter++;
    std::atomic_thread_fence(std::memory_order_release);

    if (++mWritePos >= mSize) {
        mWritePos = 0;
    }
}
