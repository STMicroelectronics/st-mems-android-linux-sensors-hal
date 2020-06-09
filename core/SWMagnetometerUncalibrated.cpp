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

#include "SWMagnetometerUncalibrated.h"

namespace stm {
namespace core {

SWMagnetometerUncalibrated::SWMagnetometerUncalibrated(const char *name, int handle)
                          : SWSensorBaseWithPollrate(name, handle,
                                                     MagnUncalibSensorType,
                                                     true, true, true, true)
{
    dependencies_type_list.push_back(MagnSensorType);
    id_sensor_trigger = SENSOR_DEPENDENCY_ID_0;
    sensor_event.data.dataLen = 6;
}

void SWMagnetometerUncalibrated::ProcessData(SensorBaseData *data)
{
    memcpy(sensor_event.data.data2, data->raw, SENSOR_DATA_3AXIS * sizeof(float));
    memcpy(sensor_event.data.data2 + 3, data->offset, SENSOR_DATA_3AXIS * sizeof(float));
    sensor_event.timestamp = data->timestamp;

    SWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
}

} // namespace core
} // namespace stm
