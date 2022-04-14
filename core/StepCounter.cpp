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
#include <cmath>

#include "StepCounter.h"

namespace stm {
namespace core {

StepCounter::StepCounter(HWSensorBaseCommonData *data, const char *name,
                         int handle, unsigned int hw_fifo_len, float power_consumption,
                         bool wakeup, int module)
    : HWSensorBase(data, name, handle,
                   StepCounterSensorType, hw_fifo_len, power_consumption, module)
{
    (void)wakeup;

    sensor_t_data.resolution = 1.0f;
    sensor_t_data.maxRange = std::pow(2, data->channels[0].bits_used) - 1;
}

int StepCounter::Enable(int handle, bool enable, bool lock_en_mutex)
{
    int err;
    bool old_status;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    old_status = GetStatus(false);

    err = HWSensorBase::Enable(handle, enable, false);
    if (err < 0) {
        if (lock_en_mutex) {
            pthread_mutex_unlock(&enable_mutex);
        }

        return err;
    }

    if (GetStatus(false) && !old_status) {
        last_data_timestamp = 0;
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
}

int StepCounter::SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mutex)
{
    int err;
    int64_t min_pollrate_ns;
    int64_t restore_min_period_ms;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    err = HWSensorBase::SetDelay(handle, period_ns, timeout, false);
    if (err < 0) {
        if (lock_en_mutex) {
            pthread_mutex_unlock(&enable_mutex);
        }

        return err;
    }

    restore_min_period_ms = sensors_pollrates[handle];
    sensors_pollrates[handle] = period_ns;
    min_pollrate_ns = GetMinPeriod(false);

    if (timeout != INT64_MAX) {
        err = device_iio_utils::set_max_delivery_rate(common_data.device_iio_sysfs_path,
                                                      NS_TO_MS(min_pollrate_ns));
        if (err < 0) {
            console.error(GetName() + std::string(": failed to set max delivery rate"));
            if (lock_en_mutex) {
                pthread_mutex_unlock(&enable_mutex);
            }
            sensors_pollrates[handle] = restore_min_period_ms;

            return err;
        }

        console.debug(GetName() + std::string(": changed max delivery rate time to ") +
                      std::to_string(((int)NS_TO_MS(min_pollrate_ns))) + "ms");
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
}

void StepCounter::ProcessData(SensorBaseData *data)
{
    sensor_event.u64.step_counter = (uint64_t)data->raw[0];
    sensor_event.timestamp = data->timestamp;

    HWSensorBase::WriteDataToPipe(0);
    HWSensorBase::ProcessData(data);
}

} // namespace core
} // namespace stm
