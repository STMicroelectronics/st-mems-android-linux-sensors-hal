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

#include <cstdint>
#include <string>
#include <array>

#include <STMSensorType.h>
#include "utils.h"

namespace stm {
namespace core {

/*
 * indicate the axes actually supported by the hardware sensor (for accel and
 * gyro only)
 */
enum limitedaxis:int { x, y, z, xy, xz, yz, xyz };

struct SensorsSupported {
    SensorsSupported(const std::string &d_name,
                     STMSensorType type,
                     device_iio_chan_type_t c_type,
                     const std::string &a_name,
                     float power,
                     limitedaxis lim_axis)
    : driver_name(d_name),
      android_name(a_name),
      android_sensor_type(type),
      iio_sensor_type(c_type),
      power_consumption(power),
      limited_axis(lim_axis) {}

    const std::string driver_name;
    const std::string android_name;
    const STMSensorType android_sensor_type;
    const device_iio_chan_type_t iio_sensor_type;
    const float power_consumption;
    const limitedaxis limited_axis;

    static SensorsSupported Accel(const std::string &d_name, const std::string &a_name, float power, limitedaxis lim_axis);
    static SensorsSupported Magn(const std::string &d_name, const std::string &a_name, float power);
    static SensorsSupported Gyro(const std::string &d_name, const std::string &a_name, float power, limitedaxis lim_axis);
    static SensorsSupported StepDetector(const std::string &d_name, const std::string &a_name, float power);
    static SensorsSupported StepCounter(const std::string &d_name, const std::string &a_name, float power);
    static SensorsSupported SignMotion(const std::string &d_name, const std::string &a_name, float power);
    static SensorsSupported Pressure(const std::string &d_name, const std::string &a_name, float power);
    static SensorsSupported AmbientTemperature(const std::string &d_name, const std::string &a_name, float power);
    static SensorsSupported InternalTemperature(const std::string &d_name, const std::string &a_name, float power);
    static SensorsSupported RelativeHumidity(const std::string &d_name, const std::string &a_name, float power);
};

extern const std::array<struct SensorsSupported, 200> sensorsSupportedList;

struct SWSensorsSupported {
    SWSensorsSupported(STMSensorType type) : type(type) {}

    const STMSensorType type;
};

extern const std::array<struct SWSensorsSupported, 12> sensorsSWSupportedList;

} // namespace core
} // namespace stm
