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

#pragma once

#include <aidl/android/hardware/sensors/ISensors.h>

using ::aidl::android::hardware::sensors::Event;
using SharedMemInfo = ::aidl::android::hardware::sensors::ISensors::SharedMemInfo;

struct LockfreeBuffer {
    LockfreeBuffer(void) = delete;
    LockfreeBuffer(void *buf, size_t size);
    ~LockfreeBuffer(void) = default;

    void write(const Event &ev);

private:
    /**
     * Actual memory where start to store sensors events
     */
    void *mData;

    /**
     * How many sensor events we can store in this memory region
     */
    size_t mSize;

    /**
     * Keep tracking the write position in the buffer
     */
    size_t mWritePos;

    /**
     * Incremental index as required by API
     */
    uint32_t mCounter;
};

struct DirectChannelBufferBase {
public:
    virtual ~DirectChannelBufferBase(void) = default;

    bool isValid(void) const;

    void writeEvent(const Event &event);

protected:
    /**
     * This object represent the buffer where to write sensor events
     */
    std::unique_ptr<LockfreeBuffer> mBuffer;

    /**
     * Keep track and make sure the direct channel is valid
     */
    bool valid;

    DirectChannelBufferBase(void) : valid(false) {};
};

struct AshmemDirectChannelBuffer : public DirectChannelBufferBase {
public:
    AshmemDirectChannelBuffer(void) = delete;
    AshmemDirectChannelBuffer(const SharedMemInfo &mem);
    ~AshmemDirectChannelBuffer(void) override;

//private:
    /**
     * MemoryFile shared memory file descriptor
     */
//    int mFd;
};

struct GrallocDirectChannelBuffer : public DirectChannelBufferBase {
public:
    GrallocDirectChannelBuffer(void) = delete;
    GrallocDirectChannelBuffer(const SharedMemInfo &mem);
    ~GrallocDirectChannelBuffer(void) override;
};
