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

#pragma once

#include "configuration.h"

#define ST_HAL_GRAVITY_MAX_ON_EARTH			(10.7f)

#define CONCATENATE_STRING(x, y)			(x y)

#define ST_HAL_SELFTEST_DATA_PATH			"/data/STSensorHAL/selftest"
#define ST_HAL_SELFTEST_CMD_DATA_PATH			"/data/STSensorHAL/selftest/cmd"
#define ST_HAL_SELFTEST_RESULTS_DATA_PATH		"/data/STSensorHAL/selftest/results"

#define ST_HAL_IIO_MAX_DEVICES				(50)
