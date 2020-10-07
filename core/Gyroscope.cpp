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

#ifdef CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED
extern "C" {
    #include "iNemoEngineAPI_gbias_estimation.h"
}
#endif /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */

namespace stm {
namespace core {

Gyroscope::Gyroscope(HWSensorBaseCommonData *data, const char *name,
                     struct device_iio_sampling_freqs *sfa, int handle,
                     unsigned int hw_fifo_len, float power_consumption, bool wakeup)
    : HWSensorBaseWithPollrate(data, name, sfa, handle,
                               GyroSensorType, hw_fifo_len, power_consumption)
{
    (void)wakeup;

    sensor_t_data.resolution = data->channels[0].scale;
    sensor_t_data.maxRange = sensor_t_data.resolution * (std::pow(2, data->channels[0].bits_used - 1) - 1);
    sensor_event.data.dataLen = 3;

#if (CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED)
    supportsSensorAdditionalInfo = true;
#endif /* CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED */

#ifdef CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED
    dependencies_type_list.push_back(AccelSensorType);
#endif /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */
}

int Gyroscope::CustomInit()
{
#ifdef CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED
#ifdef CONFIG_ST_HAL_FACTORY_CALIBRATION
    iNemoEngine_API_gbias_Initialization(factory_calibration_updated);

    if (factory_calibration_updated) {
        factory_calibration_updated = false;
    }
#else /* CONFIG_ST_HAL_FACTORY_CALIBRATION */
    iNemoEngine_API_gbias_Initialization(false);
#endif /* CONFIG_ST_HAL_FACTORY_CALIBRATION */
#endif /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */

    return 0;
}

int Gyroscope::Enable(int handle, bool enable, bool lock_en_mutex)
{
#ifdef CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED
    int err;
    bool old_status;

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
        iNemoEngine_API_gbias_enable(enable);
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
#else /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */
    return HWSensorBaseWithPollrate::Enable(handle, enable, lock_en_mutex);
#endif /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */
}

int Gyroscope::SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mutex)
{
#ifdef CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED
    int err;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    err = HWSensorBaseWithPollrate::SetDelay(handle, period_ns, timeout, false);
    if (err < 0) {
        if (lock_en_mutex) {
            pthread_mutex_unlock(&enable_mutex);
        }

        return err;
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
#else /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */
    return HWSensorBaseWithPollrate::SetDelay(handle, period_ns, timeout, lock_en_mutex);
#endif /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */
}

void Gyroscope::ProcessData(SensorBaseData *data)
{
    float tmp_raw_data[SENSOR_DATA_3AXIS];
#ifdef CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED
    int err, nomaxdata = 10;
    SensorBaseData accel_data;
#endif /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */

    memcpy(tmp_raw_data, data->raw, SENSOR_DATA_3AXIS * sizeof(float));

    data->raw[0] = SENSOR_X_DATA(tmp_raw_data[0], tmp_raw_data[1], tmp_raw_data[2], CONFIG_ST_HAL_GYRO_ROT_MATRIX);
    data->raw[1] = SENSOR_Y_DATA(tmp_raw_data[0], tmp_raw_data[1], tmp_raw_data[2], CONFIG_ST_HAL_GYRO_ROT_MATRIX);
    data->raw[2] = SENSOR_Z_DATA(tmp_raw_data[0], tmp_raw_data[1], tmp_raw_data[2], CONFIG_ST_HAL_GYRO_ROT_MATRIX);

#ifdef CONFIG_ST_HAL_FACTORY_CALIBRATION
    data->raw[0] = (data->raw[0] - factory_offset[0]) * factory_scale[0];
    data->raw[1] = (data->raw[1] - factory_offset[1]) * factory_scale[1];
    data->raw[2] = (data->raw[2] - factory_offset[2]) * factory_scale[2];
#endif /* CONFIG_ST_HAL_FACTORY_CALIBRATION */

#ifdef CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED
    do {
        err = GetLatestValidDataFromDependency(SENSOR_DEPENDENCY_ID_0, &accel_data, data->timestamp);
        if (err < 0) {
            nomaxdata--;
            usleep(10);
            continue;
        }
    } while ((nomaxdata >= 0) && (err < 0));

    if (nomaxdata > 0) {
        if (gbias_last_pollrate != data->pollrate_ns) {
            gbias_last_pollrate = data->pollrate_ns;
            iNemoEngine_API_gbias_set_frequency(NS_TO_FREQUENCY(data->pollrate_ns));
        }

        iNemoEngine_API_gbias_Run(accel_data.raw, data->raw);
    }

    iNemoEngine_API_Get_gbias(data->offset);

    data->accuracy = SENSOR_STATUS_ACCURACY_HIGH;
#else /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */
    data->accuracy = SENSOR_STATUS_UNRELIABLE;
#endif /* CONFIG_ST_HAL_GYRO_GBIAS_ESTIMATION_ENABLED */

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

#if (CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED)
size_t Gyroscope::getSensorAdditionalInfoPayLoadFramesArray(additional_info_event_t **array_sensorAdditionalInfoPLFrames)
{
    additional_info_event_t Gyro_SAI_Placement_event, *p;
    additional_info_event_t *p_custom_Gyro_SAI_Placement_event =  NULL;

    // place for ODM/OEM to fill custom_Gyro_SAI_Placement_event

    if (!p_custom_Gyro_SAI_Placement_event) {
        Gyro_SAI_Placement_event = defaultSensorPlacement_additional_info_event;
        ALOGD("%s: using Sensor Additional Info Placement default", GetName());
    } else {
        Gyro_SAI_Placement_event = *p_custom_Gyro_SAI_Placement_event;
    }

    size_t frames = 1;

    p = (additional_info_event_t *)calloc(frames , sizeof(additional_info_event_t));
    if (!p) {
        ALOGE("%s: Failed to allocate memory.", GetName());
        return (size_t)-ENOMEM;
    }
    for (size_t i = 0; i < frames; i++) {
        memcpy(&p[i], &Gyro_SAI_Placement_event, sizeof(additional_info_event_t));
    }

    *array_sensorAdditionalInfoPLFrames = p;
    return sizeof(**array_sensorAdditionalInfoPLFrames)/sizeof(*array_sensorAdditionalInfoPLFrames[0]);
}
#endif /* CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED */

} // namespace core
} // namespace stm
