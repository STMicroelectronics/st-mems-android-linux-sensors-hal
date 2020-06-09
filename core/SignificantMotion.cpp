/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2015-2020 STMicroelectronics
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

#include <fcntl.h>
#include <assert.h>
#include <signal.h>

#include "SignificantMotion.h"

namespace stm {
namespace core {

SignMotion::SignMotion(HWSensorBaseCommonData *data, const char *name, int handle,
                       float power_consumption)
    : HWSensorBase(data, name, handle,
                   SignMotionSensorType, 0, power_consumption)
{
    sensor_t_data.resolution = 1.0f;
    sensor_t_data.maxRange = 1.0f;
    sensor_event.data.dataLen = 1;
}

int SignMotion::FlushData(int __attribute__((unused))handle, bool __attribute__((unused))lock_en_mutex)
{
    return -EINVAL;
}

int SignMotion::SetDelay(int __attribute__((unused))handle,
                         int64_t __attribute__((unused))period_ns,
                         int64_t __attribute__((unused))timeout,
                         bool __attribute__((unused))lock_en_mutex)
{
    return 0;
}

void SignMotion::ProcessEvent(struct device_iio_events *event_data)
{
    sensor_event.data.data2[0] = 1.0f;
    sensor_event.timestamp = event_data->event_timestamp;

    HWSensorBase::WriteDataToPipe(0);
    HWSensorBase::ProcessEvent(event_data);
    Enable(sensor_t_data.handle, false, true);
}

} // namespace core
} // namespace stm
