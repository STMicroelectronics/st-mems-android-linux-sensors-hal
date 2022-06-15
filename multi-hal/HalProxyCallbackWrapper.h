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

namespace android {
namespace hardware {
namespace sensors {
namespace stm {
namespace multihal {

class IHalProxyCallbackWrapperBase {
public:
    virtual ~IHalProxyCallbackWrapperBase() {};

    virtual void postEvents(const std::vector<V2_1::Event>& events,
                            V2_0::implementation::ScopedWakelock wakelock) = 0;

    virtual V2_0::implementation::ScopedWakelock createScopedWakelock(bool lock) = 0;
};

template <typename T>
class HalProxyCallbackWrapperBase : public IHalProxyCallbackWrapperBase {
public:
    HalProxyCallbackWrapperBase(sp<T> callback) : mCallback(callback) {};

    V2_0::implementation::ScopedWakelock createScopedWakelock(bool lock) override {
        return mCallback->createScopedWakelock(lock);
    };

protected:
    sp<T> mCallback;
};

class HalProxyCallbackWrapperV2_0 : public HalProxyCallbackWrapperBase<V2_0::implementation::IHalProxyCallback> {
public:
    HalProxyCallbackWrapperV2_0(sp<V2_0::implementation::IHalProxyCallback> callback)
        : HalProxyCallbackWrapperBase(callback) {};

    void postEvents(const std::vector<V2_1::Event>& events,
                    V2_0::implementation::ScopedWakelock wakelock) {
        return mCallback->postEvents(V2_1::implementation::convertToOldEvents(events),
                                     std::move(wakelock));
    }
};

class HalProxyCallbackWrapperV2_1 : public HalProxyCallbackWrapperBase<V2_1::implementation::IHalProxyCallback> {
public:
    HalProxyCallbackWrapperV2_1(sp<V2_1::implementation::IHalProxyCallback> callback)
        : HalProxyCallbackWrapperBase(callback) {};

    void postEvents(const std::vector<V2_1::Event>& events,
                    V2_0::implementation::ScopedWakelock wakelock) {
        return mCallback->postEvents(events, std::move(wakelock));
    }
};

}  // namespace multihal
}  // namespace stm
}  // namespace sensors
}  // namespace hardware
}  // namespace android
