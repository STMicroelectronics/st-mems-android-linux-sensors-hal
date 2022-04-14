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

#include "SWGravity.h"

namespace stm {
namespace core {

SWGravity::SWGravity(const char *name, int handle, int module)
    : SWSensorBaseWithPollrate(name, handle,
                               GravitySensorType,
                               true, false, true, false, module)
{
    sensor_t_data.maxRange = ST_HAL_GRAVITY_MAX_ON_EARTH;

    dependencies_type_list.push_back(AccelGyroFusion6XSensorType);
    id_sensor_trigger = SENSOR_DEPENDENCY_ID_0;
    sensor_event.data.dataLen = SENSOR_DATA_3AXIS;
}

void SWGravity::ProcessData(SensorBaseData *data)
{
    memcpy(sensor_event.data.data2, data->processed, SENSOR_DATA_3AXIS * sizeof(float));
    sensor_event.timestamp = data->timestamp;

    SWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    SWSensorBaseWithPollrate::ProcessData(data);
}

} // namespace core
} // namespace stm
