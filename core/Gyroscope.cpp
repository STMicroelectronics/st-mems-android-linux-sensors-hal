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

#include "Gyroscope.h"

namespace stm {
namespace core {

Gyroscope::Gyroscope(HWSensorBaseCommonData *data, const char *name,
                     struct device_iio_sampling_freqs *sfa, int handle,
                     unsigned int hw_fifo_len, float power_consumption, bool wakeup)
    : HWSensorBaseWithPollrate(data, name, sfa, handle,
                               GyroSensorType, hw_fifo_len, power_consumption),
      bias_last_pollrate(0),
      gyroCalibration(STMGyroCalibration::getInstance())
{
    (void) wakeup;

    sensor_t_data.resolution = data->channels[0].scale;
    sensor_t_data.maxRange = sensor_t_data.resolution * (std::pow(2, data->channels[0].bits_used - 1) - 1);
    sensor_event.data.dataLen = 3;

    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
        dependencies_type_list.push_back(AccelSensorType);
    }
}

int Gyroscope::libsInit(void)
{
    std::string libVersionMsg { "gyro calibration library: " };
    int err = 0;

    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
        libVersionMsg += gyroCalibration.getLibVersion();
        console.info(libVersionMsg);

        // TODO fix the values used as parameters
        err = gyroCalibration.init(1.0f,
                                   1.0f,
                                   20.0f,
                                   sensor_t_data.maxRange);

        loadBiasValues();
    } else {
        libVersionMsg += std::string("not enabled!");
        console.info(libVersionMsg);
    }

    return err;
}

int Gyroscope::Enable(int handle, bool enable, bool lock_en_mutex)
{
    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
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

void Gyroscope::saveBiasValues(void) const
{
    Matrix<4, 3, float> bias;

    gyroCalibration.getBias(bias);

    if (sensorsCallback != nullptr) {
        if (sensorsCallback->onSaveDataRequest("gyro_bias.dat", &bias, sizeof(bias))) {
            console.warning("failed to save gyro bias");
        }
    }
}

void Gyroscope::loadBiasValues(void)
{
    Matrix<4, 3, float> bias;

    gyroCalibration.resetBiasMatrix(bias);

    if (sensorsCallback != nullptr) {
        if (sensorsCallback->onLoadDataRequest("gyro_bias.dat", &bias, sizeof(bias))) {
            console.warning("failed to load gyro bias");
        }
    }

    gyroCalibration.reset(bias);
}

void Gyroscope::ProcessData(SensorBaseData *data)
{
    float tmp_raw_data[SENSOR_DATA_3AXIS];

    memcpy(tmp_raw_data, data->raw, SENSOR_DATA_3AXIS * sizeof(float));

    data->raw[0] = SENSOR_X_DATA(tmp_raw_data[0],
                                 tmp_raw_data[1],
                                 tmp_raw_data[2],
                                 CONFIG_ST_HAL_GYRO_ROT_MATRIX);
    data->raw[1] = SENSOR_Y_DATA(tmp_raw_data[0],
                                 tmp_raw_data[1],
                                 tmp_raw_data[2],
                                 CONFIG_ST_HAL_GYRO_ROT_MATRIX);
    data->raw[2] = SENSOR_Z_DATA(tmp_raw_data[0],
                                 tmp_raw_data[1],
                                 tmp_raw_data[2],
                                 CONFIG_ST_HAL_GYRO_ROT_MATRIX);

    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
        SensorBaseData accel_data;
        int err, nomaxdata = 10;

        do {
            err = GetLatestValidDataFromDependency(SENSOR_DEPENDENCY_ID_0, &accel_data, data->timestamp);
            if (err < 0) {
                nomaxdata--;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                continue;
            }
        } while ((nomaxdata >= 0) && (err < 0));

        if (nomaxdata > 0) {
            if (bias_last_pollrate != data->pollrate_ns) {
                bias_last_pollrate = data->pollrate_ns;
                gyroCalibration.setFrequency(NS_TO_FREQUENCY(data->pollrate_ns));
            }

            std::array<float, 3> accelData({ accel_data.raw[0], accel_data.raw[1], accel_data.raw[2] });
            std::array<float, 3> gyroData({ data->raw[0], data->raw[1], data->raw[2] });
            gyroCalibration.run(accelData, gyroData, data->timestamp);
        }

        Matrix<4, 3, float> bias;
        gyroCalibration.getBias(bias);

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
    // sensor_event.gyro.status = data->accuracy;
    sensor_event.timestamp = data->timestamp;

    HWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    HWSensorBaseWithPollrate::ProcessData(data);
}

} // namespace core
} // namespace stm
