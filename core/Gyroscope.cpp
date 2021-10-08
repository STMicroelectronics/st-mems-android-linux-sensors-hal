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

int Gyroscope::CustomInit()
{
    std::string libVersionMsg { "gyro calibration library: " };
    int err = 0;

    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
        libVersionMsg += gyroCalibration.getLibVersion();

#ifdef CONFIG_ST_HAL_FACTORY_CALIBRATION
        iNemoEngine_API_gbias_Initialization(factory_calibration_updated);

        if (factory_calibration_updated) {
            factory_calibration_updated = false;
        }
#else  /* CONFIG_ST_HAL_FACTORY_CALIBRATION */
        gyroCalibration.resetBiasMatrix(currentBias);

        // TODO fix the values used as parameters
        err = gyroCalibration.init(0.0f,
                                   0.0f,
                                   0.0f,
                                   sensor_t_data.maxRange);
#endif /* CONFIG_ST_HAL_FACTORY_CALIBRATION */
    } else {
        libVersionMsg += std::string("not enabled!");
    }

    console.info(libVersionMsg);

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

        if (GetStatus(false) != old_status) {
            gyroCalibration.reset(currentBias);
        }

        if (lock_en_mutex) {
            pthread_mutex_unlock(&enable_mutex);
        }

        return 0;
    }

    return HWSensorBaseWithPollrate::Enable(handle, enable, lock_en_mutex);
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

#ifdef CONFIG_ST_HAL_FACTORY_CALIBRATION
    data->raw[0] = (data->raw[0] - factory_offset[0]) * factory_scale[0];
    data->raw[1] = (data->raw[1] - factory_offset[1]) * factory_scale[1];
    data->raw[2] = (data->raw[2] - factory_offset[2]) * factory_scale[2];
#endif /* CONFIG_ST_HAL_FACTORY_CALIBRATION */

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
            if (gbias_last_pollrate != data->pollrate_ns) {
                gbias_last_pollrate = data->pollrate_ns;
                gyroCalibration.setFrequency(NS_TO_FREQUENCY(data->pollrate_ns));
            }

            std::array<float, 3> accelData({ accel_data.raw[0], accel_data.raw[0], accel_data.raw[0] });
            std::array<float, 3> gyroData({ data->raw[0], data->raw[0], data->raw[0] });
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
