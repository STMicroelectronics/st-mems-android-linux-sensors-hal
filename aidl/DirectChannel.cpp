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

using SharedMemInfo = ::aidl::android::hardware::sensors::ISensors::SharedMemInfo;
using ISensors = ::aidl::android::hardware::sensors::ISensors;

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
{
    (void)mem;
}

AshmemDirectChannelBuffer::~AshmemDirectChannelBuffer(void)
{
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
      mSize(size / static_cast<int>(ISensors::DIRECT_REPORT_SENSOR_EVENT_TOTAL_LENGTH)),
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
    (void)ev;
    (void)mCounter;
    (void)mWritePos;
    (void)mSize;
}
