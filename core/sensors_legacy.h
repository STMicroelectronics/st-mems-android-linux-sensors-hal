/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019-2020 STMicroelectronics
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

#include <STMSensorsList.h>
#include <ISTMSensorsCallback.h>
#include "temp_struct_porting.h"

namespace stm {
namespace core {

int st_hal_open_sensors(void **data, STMSensorsList &sensorsList);

void st_hal_close_sensors(void *data);

void st_hal_dev_set_callbacks(void *data, const ISTMSensorsCallback &sensorsCallback);

int st_hal_dev_activate(void *data, uint32_t handle, bool enable);

int st_hal_dev_batch(void *data, int handle, int64_t period_ns,
                     int64_t timeout);

int st_hal_dev_flush(void *data, uint32_t handle);

int st_hal_dev_poll(void *data, sensors_event_t *sdata, int count);

} // namespace core
} // namespace stm
