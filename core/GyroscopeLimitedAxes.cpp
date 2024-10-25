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

#include "GyroscopeLimitedAxes.h"

namespace stm {
namespace core {

GyroscopeLimitedAxes::GyroscopeLimitedAxes(HWSensorBaseCommonData *data, const char *name,
                     struct device_iio_sampling_freqs *sfa, int handle,
                     unsigned int hw_fifo_len, float power_consumption,
                     bool wakeup, int module,
                     bool x_is_supp = true,
                     bool y_is_supp = true,
                     bool z_is_supp = true)
    : HWSensorBaseWithPollrate(data, name, sfa, handle,
                               GyroSensorLimitedAxisType,
                               hw_fifo_len, power_consumption, module,
                               x_is_supp, y_is_supp, z_is_supp),
      bias_last_pollrate(0),
      gyroCalibration(STMGyroCalibration::getInstance())
{
    (void) wakeup;

    rotMatrix = propertiesManager.getRotationMatrix(handle);
    biasFileName = std::string("gyro_bias_") + std::to_string(moduleId) + std::string(".dat");

    sensor_t_data.resolution = data->channels[0].scale;
    sensor_t_data.maxRange = sensor_t_data.resolution * (std::pow(2, data->channels[0].bits_used - 1) - 1);

    /* limited axes sensors include supported axes flag in the event payload */
    sensor_event.data.dataLen = 7;

    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
        dependencies_type_list.push_back(AccelSensorLimitedAxisType);
    }
}

int GyroscopeLimitedAxes::libsInit(void)
{
    std::string libVersionMsg { "gyro calibration library: " };

    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
        libVersionMsg += gyroCalibration.getLibVersion();
        console.info(libVersionMsg);

        // TODO fix the values used as parameters
        return gyroCalibration.init(1.0f,
                                    1.0f,
                                    20.0f,
                                    sensor_t_data.maxRange);
    } else {
        libVersionMsg += std::string("not enabled!");
        console.info(libVersionMsg);
    }

    return 0;
}

void GyroscopeLimitedAxes::postSetup(void)
{
    loadBiasValues();
}

int GyroscopeLimitedAxes::Enable(int handle, bool enable, bool lock_en_mutex)
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

void GyroscopeLimitedAxes::saveBiasValues(void) const
{
    Matrix<4, 3, float> bias;

    gyroCalibration.getBias(bias);

    if (sensorsCallback != nullptr) {
        if (sensorsCallback->onSaveDataRequest(biasFileName, &bias, sizeof(bias)) <= 0) {
            console.warning("failed to save gyro bias");
        }
    }
}

void GyroscopeLimitedAxes::loadBiasValues(void)
{
    Matrix<4, 3, float> bias;

    gyroCalibration.resetBiasMatrix(bias);

    if (sensorsCallback != nullptr) {
        if (sensorsCallback->onLoadDataRequest(biasFileName, &bias, sizeof(bias)) <= 0) {
            console.warning("failed to load gyro bias");
        }
    }

    gyroCalibration.reset(bias);
}

void GyroscopeLimitedAxes::ProcessData(SensorBaseData *data)
{
    std::array<float, 3> gyroTmp;
    int ret;

    ret = copyAxesData(gyroTmp, data);
    if (ret)
        return;

    gyroTmp = rotMatrix * gyroTmp;
    memcpy(data->raw, gyroTmp.data(), SENSOR_DATA_3AXIS * sizeof(float));

    if (HAL_ENABLE_GYRO_CALIBRATION != 0 &&
        dependencies_type_list.size() > 0) {
        SensorBaseData accel_data;
        int err, nomaxdata = 10;

        do {
            err = GetLatestValidDataFromDependency(SENSOR_DEPENDENCY_ID_0,
                                                   &accel_data, data->timestamp);
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
    sensor_event.data.data2[3] = (float)data->accuracy;

    sensor_event.data.data2[4] = (float)isXSupported();
    sensor_event.data.data2[5] = (float)isYSupported();
    sensor_event.data.data2[6] = (float)isZSupported();

    sensor_event.timestamp = data->timestamp;

    HWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    HWSensorBaseWithPollrate::ProcessData(data);
}

} // namespace core
} // namespace stm
