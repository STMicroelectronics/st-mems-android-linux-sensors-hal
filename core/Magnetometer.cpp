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

#include "Magnetometer.h"

#define GAUSS_TO_UTESLA(x)                  ((x) * 100.0f)

#define CALIBRATION_FREQUENCY               25
#define CALIBRATION_PERIOD_MS               (1000.0f / CALIBRATION_FREQUENCY)

namespace stm {
namespace core {

Magnetometer::Magnetometer(HWSensorBaseCommonData *data, const char *name,
                           struct device_iio_sampling_freqs *sfa, int handle,
                           unsigned int hw_fifo_len, float power_consumption, bool wakeup)
    : HWSensorBaseWithPollrate(data, name, sfa, handle,
                               MagnSensorType,
                               hw_fifo_len, power_consumption),
      bias_last_pollrate(0),
      magnCalibration(STMMagnCalibration::getInstance())
{
    (void) wakeup;

    sensor_t_data.resolution = GAUSS_TO_UTESLA(data->channels[0].scale);
    sensor_t_data.maxRange = sensor_t_data.resolution * (std::pow(2, data->channels[0].bits_used - 1) - 1);
    sensor_event.data.dataLen = 3;
}

int Magnetometer::libsInit(void)
{
    std::string libVersionMsg { "magn calibration library: " };
    int err = 0;

    if (HAL_ENABLE_MAGN_CALIBRATION != 0) {
        libVersionMsg += magnCalibration.getLibVersion();
        console.info(libVersionMsg);

        err = magnCalibration.init(sensor_t_data.maxRange);

        loadBiasValues();
    } else {
        libVersionMsg += std::string("not enabled!");
        console.info(libVersionMsg);
    }

    return err;
}

int Magnetometer::Enable(int handle, bool enable, bool lock_en_mutex)
{
    if (HAL_ENABLE_MAGN_CALIBRATION != 0) {
        bool old_status;
        int err;

        if (lock_en_mutex) {
            pthread_mutex_lock(&enable_mutex);
        }

        old_status = GetStatus(false);

        err = HWSensorBaseWithPollrate::Enable(handle, enable, false);
        if (err < 0) {
            if (lock_en_mutex) {
                pthread_mutex_unlock(&enable_mutex);
            }

            return err;
        }

        bool new_status = GetStatus(false);
        if (!new_status && (new_status != old_status)) {
            saveBiasValues();
        }

        if (lock_en_mutex) {
            pthread_mutex_unlock(&enable_mutex);
        }

        return 0;
    }

    return HWSensorBaseWithPollrate::Enable(handle, enable, lock_en_mutex);
}

void Magnetometer::saveBiasValues(void) const
{
    Matrix<4, 3, float> bias;

    magnCalibration.getBias(bias);

    if (sensorsCallback != nullptr) {
        if (sensorsCallback->onSaveDataRequest("magn_bias.dat", &bias, sizeof(bias))) {
            console.error("failed to save magn bias");
        }
    }
}

void Magnetometer::loadBiasValues(void)
{
    Matrix<4, 3, float> bias;

    magnCalibration.resetBiasMatrix(bias);

    if (sensorsCallback != nullptr) {
        if (sensorsCallback->onLoadDataRequest("magn_bias.dat", &bias, sizeof(bias))) {
            console.error("failed to load magn bias");
        }
    }

    magnCalibration.reset(bias);
}

void Magnetometer::ProcessData(SensorBaseData *data)
{
    float tmp_raw_data[SENSOR_DATA_3AXIS];

    memcpy(tmp_raw_data, data->raw, SENSOR_DATA_3AXIS * sizeof(float));

    data->raw[0] = GAUSS_TO_UTESLA(SENSOR_X_DATA(tmp_raw_data[0],
                                                 tmp_raw_data[1],
                                                 tmp_raw_data[2],
                                                 CONFIG_ST_HAL_MAGN_ROT_MATRIX));
    data->raw[1] = GAUSS_TO_UTESLA(SENSOR_Y_DATA(tmp_raw_data[0],
                                                 tmp_raw_data[1],
                                                 tmp_raw_data[2],
                                                 CONFIG_ST_HAL_MAGN_ROT_MATRIX));
    data->raw[2] = GAUSS_TO_UTESLA(SENSOR_Z_DATA(tmp_raw_data[0],
                                                 tmp_raw_data[1],
                                                 tmp_raw_data[2],
                                                 CONFIG_ST_HAL_MAGN_ROT_MATRIX));

#ifdef CONFIG_ST_HAL_FACTORY_CALIBRATION
    data->raw[0] = (data->raw[0] - factory_offset[0]) * factory_scale[0];
    data->raw[1] = (data->raw[1] - factory_offset[1]) * factory_scale[1];
    data->raw[2] = (data->raw[2] - factory_offset[2]) * factory_scale[2];
#endif /* CONFIG_ST_HAL_FACTORY_CALIBRATION */

    if (HAL_ENABLE_MAGN_CALIBRATION != 0) {
        std::array<float, 3> magnData;
        Matrix<4, 3, float> bias;

        memcpy(magnData.data(), data->raw, 3 * sizeof(float));

        if (bias_last_pollrate != data->pollrate_ns) {
            bias_last_pollrate = data->pollrate_ns;
            magnCalibration.setFrequency(NS_TO_FREQUENCY(data->pollrate_ns));
        }

        magnCalibration.run(magnData, data->timestamp);
        magnCalibration.getBias(bias);

        data->offset[0] = bias[3][0];
        data->offset[1] = bias[3][1];
        data->offset[2] = bias[3][2];

        data->accuracy = SENSOR_STATUS_ACCURACY_HIGH;
    } else {
        data->accuracy = SENSOR_STATUS_UNRELIABLE;
        memset(data->offset, 0, 3 * sizeof(float));
    }

    data->processed[0] = data->raw[0] - data->offset[0];
    data->processed[1] = data->raw[1] - data->offset[1];
    data->processed[2] = data->raw[2] - data->offset[2];

    sensor_event.data.data2[0] = data->processed[0];
    sensor_event.data.data2[1] = data->processed[1];
    sensor_event.data.data2[2] = data->processed[2];
    // sensor_event.magnetic.status = data->accuracy;
    sensor_event.timestamp = data->timestamp;

    HWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    HWSensorBaseWithPollrate::ProcessData(data);
}

} // namespace core
} // namespace stm
