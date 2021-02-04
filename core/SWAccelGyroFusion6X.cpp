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

#include "SWAccelGyroFusion6X.h"

#ifdef CONFIG_INEMOENGINE_PRO
extern "C" {
    #include "iNemoEngineProAPI.h"
}
#endif /* CONFIG_INEMOENGINE_PRO */

namespace stm {
namespace core {

SWAccelGyroFusion6X::SWAccelGyroFusion6X(const char *name, int handle)
    : SWSensorBaseWithPollrate(name, handle,
                               AccelGyroFusion6XSensorType,
                               false, false, true, false)
{
    sensor_t_data.minRateHz = CONFIG_ST_HAL_MIN_FUSION_POLLRATE;

    sensor_t_data.resolution = ST_SENSOR_FUSION_RESOLUTION(1.0f);
    sensor_t_data.maxRange = 1.0f;

    dependencies_type_list.push_back(AccelSensorType);
    dependencies_type_list.push_back(GyroSensorType);
    id_sensor_trigger = SENSOR_DEPENDENCY_ID_1;
}

int SWAccelGyroFusion6X::CustomInit()
{
#ifdef CONFIG_INEMOENGINE_PRO
    iNemoEngine_API_Initialization(NULL, NULL);
#else /* CONFIG_INEMOENGINE_PRO */
    InvalidThisClass();
#endif /* CONFIG_INEMOENGINE_PRO */

    return 0;
}

int SWAccelGyroFusion6X::Enable(int handle, bool enable, bool lock_en_mutex)
{
    int err;
    bool old_status;
    bool old_status_no_handle;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    old_status = GetStatus(false);
    old_status_no_handle = GetStatusExcludeHandle(handle);

    err = SWSensorBaseWithPollrate::Enable(handle, enable, false);
    if (err < 0) {
        if (lock_en_mutex) {
            pthread_mutex_unlock(&enable_mutex);
        }

        return err;
    }

    if ((enable && !old_status) || (!enable && !old_status_no_handle)) {
        if (enable) {
            sensor_global_enable = utils.getTime();
        } else {
            sensor_global_disable = utils.getTime();
        }

#ifdef CONFIG_INEMOENGINE_PRO
        iNemoEngine_API_enable6X(enable);
#endif /* CONFIG_INEMOENGINE_PRO */
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
}

int SWAccelGyroFusion6X::SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mutex)
{
    int err;

    if ((period_ns > FREQUENCY_TO_NS(CONFIG_ST_HAL_MIN_FUSION_POLLRATE)) && (period_ns != INT64_MAX)) {
        period_ns = FREQUENCY_TO_NS(CONFIG_ST_HAL_MIN_FUSION_POLLRATE);
    }

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    err = SWSensorBaseWithPollrate::SetDelay(handle, period_ns, timeout, false);
    if (err < 0){
        if (lock_en_mutex) {
            pthread_mutex_unlock(&enable_mutex);
        }

        return err;
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
}

#ifdef CONFIG_INEMOENGINE_PRO
void SWAccelGyroFusion6X::ProcessData(SensorBaseData *data)
{
    unsigned int i;
    SensorBaseData accel_data;
    int err, nomaxdata = 10;
    iNemoSensorsData sdata;

    do {
        err = GetLatestValidDataFromDependency(SENSOR_DEPENDENCY_ID_0, &accel_data, data->timestamp);
        if (err < 0) {
            usleep(10);
            nomaxdata--;
            continue;
        }
    } while ((nomaxdata >= 0) && (err < 0));

    if (nomaxdata > 0) {
            memcpy(sdata.accel, accel_data.raw, sizeof(sdata.accel));
            memcpy(sdata.gyro, data->processed, sizeof(sdata.gyro));

            iNemoEngine_API_Run(data->timestamp - sensor_event.timestamp, &sdata);
    }

    sensor_event.timestamp = data->timestamp;
    outdata.timestamp = data->timestamp;
    outdata.flush_event_handle = data->flush_event_handle;
    outdata.accuracy = data->accuracy;
    outdata.pollrate_ns = data->pollrate_ns;

    for (i = 0; i < push_data.num; i++) {
        if (!push_data.sb[i]->ValidDataToPush(outdata.timestamp)) {
            continue;
        }

        switch (push_data.sb[i]->GetType()) {
        case STMSensorType::ROTATION_VECTOR:
            err = iNemoEngine_API_Get_6X_Quaternion(outdata.processed);
            if (err < 0)
                continue;

            break;

        case STMSensorType::GRAVITY:
            err = iNemoEngine_API_Get_Gravity(outdata.processed);
            if (err < 0)
                continue;

            break;

        case STMSensorType::LINEAR_ACCELERATION:
            err = iNemoEngine_API_Get_Linear_Acceleration(outdata.processed);
            if (err < 0)
                continue;

            break;

        default:
            return;
        }

        push_data.sb[i]->ReceiveDataFromDependency(sensor_t_data.handle, &outdata);
    }
}
#else /* CONFIG_INEMOENGINE_PRO */
void SWAccelGyroFusion6X::ProcessData(SensorBaseData *data)
{
    outdata.timestamp = data->timestamp;
    outdata.flushEventHandles = data->flushEventHandles;
    outdata.flushEventsNum = data->flushEventsNum;
    outdata.accuracy = data->accuracy;
    outdata.pollrate_ns = data->pollrate_ns;

    for (auto i = 0U; i < push_data.num; i++) {
        push_data.sb[i]->ReceiveDataFromDependency(sensor_t_data.handle, &outdata);
    }
}
#endif /* CONFIG_INEMOENGINE_PRO */

} // namespace core
} // namespace stm
