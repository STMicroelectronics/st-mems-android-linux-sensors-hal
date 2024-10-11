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
                     unsigned int hw_fifo_len, float power_consumption,
                     bool wakeup, int module)
    : HWSensorBaseWithPollrate(data, name, sfa, handle,
                               GyroSensorType,
                               hw_fifo_len, power_consumption, module),
      bias_last_pollrate(0),
      gyroCalibration(STMGyroCalibration::getInstance()),
      gyroTempCalibration(STMGyroTempCalibration::getInstance())
{
    (void) wakeup;

    rotMatrix = propertiesManager.getRotationMatrix(GyroSensorType);
    biasFileName = std::string("gyro_bias_") + std::to_string(moduleId) + std::string(".dat");
    biasTFileName = std::string("gyro_bias_temperature_") + std::to_string(moduleId) + std::string(".dat");

    sensor_t_data.resolution = data->channels[0].scale;
    sensor_t_data.maxRange = sensor_t_data.resolution * (std::pow(2, data->channels[0].bits_used - 1) - 1);

    sensor_event.data.dataLen = 4;

    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
        dependencies_type_list.push_back(AccelSensorType);

        if (HAL_ENABLE_GYRO_TEMPERATURE_CALIBRATION != 0)
            dependencies_type_list.push_back(IntTemperatureSensorType);
    }
}

int Gyroscope::libsInit(void)
{
    std::string libVersionGTMsg { "gyro temperature calibration library: " };
    std::string libVersionGCMsg { "gyro calibration library: " };
    int ret = 0;

    if (HAL_ENABLE_GYRO_CALIBRATION != 0) {
        ret = gyroCalibration.init(1.0f,
                                   1.0f,
                                   20.0f,
                                   sensor_t_data.maxRange);
        if (ret < 0) {
            console.error("Gyro calibration library init error " + std::to_string(ret));

            return ret;
        }

        libVersionGCMsg += gyroCalibration.getLibVersion();
        console.info(libVersionGCMsg);

        /* Gyro temperature calibration requests Gyro calibration */
        if (HAL_ENABLE_GYRO_TEMPERATURE_CALIBRATION != 0) {
            ret = gyroTempCalibration.initialize();
            if (ret < 0) {
                console.error("Gyro temperature calibration library init error " + std::to_string(ret));

                return ret;
            }

            libVersionGTMsg += gyroTempCalibration.getLibVersion();
            console.info(libVersionGTMsg);
        } else {
            libVersionGTMsg += std::string("not enabled!");
            console.info(libVersionGTMsg);
        }
    } else {
        libVersionGCMsg += std::string("not enabled!");
        console.info(libVersionGCMsg);
    }

    return ret;
}

void Gyroscope::postSetup(void)
{
    loadBiasValues();
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
        if (sensorsCallback->onSaveDataRequest(biasFileName, &bias, sizeof(bias)) <= 0) {
            console.warning("failed to save gyro bias");
        }
    }

    if (HAL_ENABLE_GYRO_TEMPERATURE_CALIBRATION != 0) {
        std::array<unsigned char, STMGyroTempCalibration::STMGyroTempCalibrationStateSize> state = { 0 };

        gyroTempCalibration.getState((void *)&state);

        if (sensorsCallback != nullptr) {
            if (sensorsCallback->onSaveDataRequest(biasTFileName, &state, sizeof(state)) <= 0) {
                console.warning("failed to save temperature-based gyro bias");
            }
        }
    }
}

void Gyroscope::loadBiasValues(void)
{
    Matrix<4, 3, float> bias;

    gyroCalibration.resetBiasMatrix(bias);

    if (sensorsCallback != nullptr) {
        if (sensorsCallback->onLoadDataRequest(biasFileName, &bias, sizeof(bias)) <= 0) {
            console.warning("failed to load gyro bias");
        }
    }

    gyroCalibration.reset(bias);

    if (HAL_ENABLE_GYRO_TEMPERATURE_CALIBRATION != 0) {
        std::array<unsigned char, STMGyroTempCalibration::STMGyroTempCalibrationStateSize> state = { 0 };

        if (sensorsCallback != nullptr) {
            if (sensorsCallback->onLoadDataRequest(biasTFileName, &state, sizeof(state)) <= 0) {
                console.warning("failed to load temperature-based gyro bias");
            }

            gyroTempCalibration.setState((void *)&state);
        }
    }
}

void Gyroscope::ProcessData(SensorBaseData *data)
{
    std::array<float, 3> gyroTmp;
    int acc_dep_id = SENSOR_DEPENDENCY_ID_0;
    int temp_dep_id = SENSOR_DEPENDENCY_ID_0;

    memcpy(gyroTmp.data(), data->raw, SENSOR_DATA_3AXIS * sizeof(float));
    gyroTmp = rotMatrix * gyroTmp;
    memcpy(data->raw, gyroTmp.data(), SENSOR_DATA_3AXIS * sizeof(float));

    /*
     * in case both GC and GT libs enabled adjust sensor dependency
     * list accordingly
     */
    if (HAL_ENABLE_GYRO_CALIBRATION != 0 &&
        HAL_ENABLE_GYRO_TEMPERATURE_CALIBRATION != 0) {
        acc_dep_id = SENSOR_DEPENDENCY_ID_1;
    }

    if (HAL_ENABLE_GYRO_CALIBRATION != 0 &&
        dependencies_type_list.size() > 0) {
        SensorBaseData accel_data;
        int err, nomaxdata = 10;

        do {
            err = GetLatestValidDataFromDependency(acc_dep_id, &accel_data, data->timestamp);
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

    if (HAL_ENABLE_GYRO_CALIBRATION != 0 &&
        HAL_ENABLE_GYRO_TEMPERATURE_CALIBRATION != 0 &&
        dependencies_type_list.size() > 0) {
        // Run MotionGT @ 1Hz
        if ((++gyro_decimator) >= ceil(getHWSamplingRate())) {
            SensorBaseData temperature_data;
            int err, nomaxdata = 10, update = 0;

            gyro_decimator = 0;

            do {
                err = GetLatestValidDataFromDependency(temp_dep_id, &temperature_data, data->timestamp);
                if (err < 0) {
                    nomaxdata--;
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                    continue;
                }
            } while ((nomaxdata >= 0) && (err < 0));

            /* Gyro temperature calibration input data are the offset calculated by gyro calibration */
            std::array<float, 3> gyroData({ data->offset[0], data->offset[1], data->offset[2] });
            gyroTempCalibration.run(gyroData, temperature_data.raw[0], data->timestamp, &update);

            if (update) {
                std::array<float, 3> bias;

                gyroTempCalibration.getBias(&temperature_data.raw[0], bias);
                data->offset[0] += bias[0];
                data->offset[1] += bias[1];
                data->offset[2] += bias[2];
            }
        }
    }

    data->processed[0] = data->raw[0] - data->offset[0];
    data->processed[1] = data->raw[1] - data->offset[1];
    data->processed[2] = data->raw[2] - data->offset[2];

    sensor_event.data.data2[0] = data->processed[0];
    sensor_event.data.data2[1] = data->processed[1];
    sensor_event.data.data2[2] = data->processed[2];
    sensor_event.data.data2[3] = (float)data->accuracy;

    sensor_event.timestamp = data->timestamp;

    HWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    HWSensorBaseWithPollrate::ProcessData(data);
}

} // namespace core
} // namespace stm
