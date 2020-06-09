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

#include "Accelerometer.h"

#ifdef CONFIG_ST_HAL_ACCEL_CALIB_ENABLED
#define CALIBRATION_FREQUENCY                 25
#define CALIBRATION_PERIOD_MS                 (1000.0f / CALIBRATION_FREQUENCY)

extern "C" {
    #include "STAccCalibration_API.h"
}
#endif /* CONFIG_ST_HAL_ACCEL_CALIB_ENABLED */

namespace stm {
namespace core {

Accelerometer::Accelerometer(HWSensorBaseCommonData *data,
                             const char *name,
                             struct device_iio_sampling_freqs *sfa,
                             int handle,
                             unsigned int hw_fifo_len,
                             float power_consumption, bool wakeup)
    : HWSensorBaseWithPollrate(data, name, sfa, handle,
                               AccelSensorType,
                               hw_fifo_len, power_consumption)
{
    (void)wakeup;

    sensor_t_data.resolution = data->channels[0].scale;
    sensor_t_data.maxRange = sensor_t_data.resolution * (std::pow(2, data->channels[0].bits_used - 1) - 1);
    sensor_event.data.dataLen = 3;

#if (CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED)
    supportsSensorAdditionalInfo = true;
#endif /* CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED */
}

int Accelerometer::Enable(int handle, bool enable, bool lock_en_mutex)
{
#ifdef CONFIG_ST_HAL_ACCEL_CALIB_ENABLED
    int err;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    err = HWSensorBaseWithPollrate::Enable(handle, enable, false);
    if (err < 0) {
        if (lock_en_mutex) {
            pthread_mutex_unlock(&enable_mutex);
        }

        return err;
    }

    if (enable) {
        ST_AccCalibration_API_Init(CALIBRATION_PERIOD_MS);
    } else {
        ST_AccCalibration_API_DeInit(CALIBRATION_PERIOD_MS);
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
#else /* CONFIG_ST_HAL_ACCEL_CALIB_ENABLED */
    return HWSensorBaseWithPollrate::Enable(handle, enable, lock_en_mutex);
#endif /* CONFIG_ST_HAL_ACCEL_CALIB_ENABLED */
}

void Accelerometer::ProcessData(SensorBaseData *data)
{
    float tmp_raw_data[SENSOR_DATA_3AXIS];
#ifdef CONFIG_ST_HAL_ACCEL_CALIB_ENABLED
    STAccCalibration_Input acc_cal_input;
    STAccCalibration_Output acc_cal_output;
#endif /* CONFIG_ST_HAL_ACCEL_CALIB_ENABLED */

    memcpy(tmp_raw_data, data->raw, SENSOR_DATA_3AXIS * sizeof(float));

    data->raw[0] = SENSOR_X_DATA(tmp_raw_data[0], tmp_raw_data[1], tmp_raw_data[2], CONFIG_ST_HAL_ACCEL_ROT_MATRIX);
    data->raw[1] = SENSOR_Y_DATA(tmp_raw_data[0], tmp_raw_data[1], tmp_raw_data[2], CONFIG_ST_HAL_ACCEL_ROT_MATRIX);
    data->raw[2] = SENSOR_Z_DATA(tmp_raw_data[0], tmp_raw_data[1], tmp_raw_data[2], CONFIG_ST_HAL_ACCEL_ROT_MATRIX);

#ifdef CONFIG_ST_HAL_FACTORY_CALIBRATION
    data->raw[0] = (data->raw[0] - factory_offset[0]) * factory_scale[0];
    data->raw[1] = (data->raw[1] - factory_offset[1]) * factory_scale[1];
    data->raw[2] = (data->raw[2] - factory_offset[2]) * factory_scale[2];
    data->accuracy = SENSOR_STATUS_ACCURACY_HIGH;
#else /* CONFIG_ST_HAL_FACTORY_CALIBRATION */
    data->accuracy = SENSOR_STATUS_UNRELIABLE;
#endif /* CONFIG_ST_HAL_FACTORY_CALIBRATION */

#ifdef CONFIG_ST_HAL_ACCEL_CALIB_ENABLED
    acc_cal_input.timestamp = data->timestamp;
    acc_cal_input.acc_raw[0] = data->raw[0];
    acc_cal_input.acc_raw[1] = data->raw[1];
    acc_cal_input.acc_raw[2] = data->raw[2];

    ST_AccCalibration_API_Run(&acc_cal_output, &acc_cal_input);

    data->processed[0] = acc_cal_output.acc_cal[0];
    data->processed[1] = acc_cal_output.acc_cal[1];
    data->processed[2] = acc_cal_output.acc_cal[2];
    data->offset[0] = acc_cal_output.offset[0];
    data->offset[1] = acc_cal_output.offset[1];
    data->offset[2] = acc_cal_output.offset[2];
    data->accuracy = acc_cal_output.accuracy;
#else /* CONFIG_ST_HAL_ACCEL_CALIB_ENABLED */
    data->processed[0] = data->raw[0];
    data->processed[1] = data->raw[1];
    data->processed[2] = data->raw[2];

    data->accuracy = SENSOR_STATUS_UNRELIABLE;
#endif /* CONFIG_ST_HAL_ACCEL_CALIB_ENABLED */

    sensor_event.data.data2[0] = data->processed[0];
    sensor_event.data.data2[1] = data->processed[1];
    sensor_event.data.data2[2] = data->processed[2];
    // sensor_event.acceleration.status = data->accuracy;
    sensor_event.timestamp = data->timestamp;

    HWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    HWSensorBaseWithPollrate::ProcessData(data);
}

#if (CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED)
size_t Accelerometer::getSensorAdditionalInfoPayLoadFramesArray(additional_info_event_t **array_sensorAdditionalInfoPLFrames)
{
    additional_info_event_t XL_SAI_Placement_event, *p;
    additional_info_event_t *p_custom_XL_SAI_Placement_event = NULL;

    // place for ODM/OEM to fill custom_XL_SAI_Placement_event

    if (!p_custom_XL_SAI_Placement_event) {
        XL_SAI_Placement_event = defaultSensorPlacement_additional_info_event;
        ALOGD("%s: using Sensor Additional Info Placement default", GetName());
    } else {
        XL_SAI_Placement_event = *p_custom_XL_SAI_Placement_event;
    }

    size_t frames = 1;

    p = (additional_info_event_t *)calloc(frames , sizeof(additional_info_event_t));
    if (!p) {
        ALOGE("%s: Failed to allocate memory.", GetName());
        return (size_t)-ENOMEM;
    }
    for (size_t i = 0; i < frames; i++)
        memcpy(&p[i], &XL_SAI_Placement_event, sizeof(additional_info_event_t));

    *array_sensorAdditionalInfoPLFrames = p;
    return sizeof(**array_sensorAdditionalInfoPLFrames)/sizeof(*array_sensorAdditionalInfoPLFrames[0]);
}
#endif /* CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED */

} // namespace core
} // namespace stm
