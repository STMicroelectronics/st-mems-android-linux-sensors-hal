/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2016-2020 STMicroelectronics
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

#include "HWSensorBase.h"

namespace stm {
namespace core {

/*
 * class WristTiltGesture
 */
class WristTiltGesture : public HWSensorBase {
public:
    WristTiltGesture(HWSensorBaseCommonData *data, const char *name, int handle,
                     unsigned int hw_fifo_len, float power_consumption);

    virtual int SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mute);
    virtual void ProcessData(SensorBaseData *data);
};

} // namespace core
} // namespace stm
