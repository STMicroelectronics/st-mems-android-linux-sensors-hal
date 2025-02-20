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

#include "IntTemp.h"

namespace stm {
namespace core {

IntTemp::IntTemp(HWSensorBaseCommonData *data, const char *name,
                 struct device_iio_sampling_freqs *sfa,
                 int handle, unsigned int hw_fifo_len,
                 float power_consumption, bool wakeup, int module)
    : HWSensorBaseWithPollrate(data, name, sfa, handle,
                               IntTemperatureSensorType,
                               hw_fifo_len, power_consumption, module)
{
    (void)wakeup;

    sensor_t_data.resolution = std::fabs(data->channels[0].scale);
    sensor_t_data.maxRange = sensor_t_data.resolution *
                             (std::pow(2, data->channels[0].bits_used) - 1);
    sensor_event.data.dataLen = 1;
}

void IntTemp::ProcessData(SensorBaseData *data)
{
    sensor_event.data.data2[0] = data->raw[0];
    sensor_event.timestamp = data->timestamp;

    HWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    HWSensorBaseWithPollrate::ProcessData(data);
}

} // namespace core
} // namespace stm
