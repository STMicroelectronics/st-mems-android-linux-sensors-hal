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

namespace stm {
namespace core {

SWAccelGyroFusion6X::SWAccelGyroFusion6X(const char *name, int handle, int module)
    : SWSensorBaseWithPollrate(name, handle,
                               AccelGyroFusion6XSensorType,
                               false, false, true, false, module),
      sensorsFusion(STMSensorsFusion6Axis::getInstance())
{
    sensor_t_data.minRateHz = CONFIG_ST_HAL_MIN_FUSION_POLLRATE;

    sensor_t_data.resolution = ST_SENSOR_FUSION_RESOLUTION(1.0f);
    sensor_t_data.maxRange = 1.0f;

    CustomInit();
    dependencies_type_list.push_back(AccelSensorType);
    dependencies_type_list.push_back(GyroSensorType);
    id_sensor_trigger = SENSOR_DEPENDENCY_ID_1;
}

int SWAccelGyroFusion6X::CustomInit()
{
    if (HAL_ENABLE_SENSORS_FUSION == 0) {
        InvalidThisClass();
    }

    return 0;
}

int SWAccelGyroFusion6X::libsInit(void)
{
    std::string libVersionMsg { "sensors fusion (6X) library: " };
    int err = 0;

    if (HAL_ENABLE_SENSORS_FUSION != 0) {
        err = sensorsFusion.init();
        libVersionMsg += sensorsFusion.getLibVersion();
    } else {
        libVersionMsg += std::string("not enabled!");
    }

    console.info(libVersionMsg);

    return err;
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
            if (HAL_ENABLE_SENSORS_FUSION != 0) {
                sensorsFusion.reset(NULL);
            }

            sensor_event.timestamp = 0;
            sensor_global_enable = utils.getTime();
        } else {
            sensor_global_disable = utils.getTime();
        }
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

void SWAccelGyroFusion6X::ProcessData(SensorBaseData *data)
{
    if (HAL_ENABLE_SENSORS_FUSION != 0) {
        unsigned int i;
        SensorBaseData accel_data;
        int err, nomaxdata = 10;

        do {
            err = GetLatestValidDataFromDependency(SENSOR_DEPENDENCY_ID_0, &accel_data, data->timestamp);
            if (err < 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                nomaxdata--;
                continue;
            }
        } while ((nomaxdata >= 0) && (err < 0));

        if (nomaxdata > 0) {
            std::array<float, 3> accelData;
            std::array<float, 3> gyroData;

            memcpy(accelData.data(), accel_data.raw, 3 * sizeof(float));
            memcpy(gyroData.data(), data->processed, 3 * sizeof(float));

            sensorsFusion.run(accelData, gyroData, data->timestamp);
        }

        sensor_event.timestamp = data->timestamp;
        outdata.timestamp = data->timestamp;
        outdata.flushEventHandles = data->flushEventHandles;
        outdata.flushEventsNum = data->flushEventsNum;
        outdata.accuracy = data->accuracy;
        outdata.pollrate_ns = data->pollrate_ns;

        for (i = 0; i < push_data.num; i++) {
            if (!push_data.sb[i]->ValidDataToPush(outdata.timestamp)) {
                continue;
            }

            switch ((SensorType)push_data.sb[i]->GetType()) {
            case SensorType::GAME_ROTATION_VECTOR:
                std::array<float, 4> quaternion;

                err = sensorsFusion.getQuaternion(quaternion);
                if (err < 0)
                    continue;

                memcpy(outdata.processed, quaternion.data(), 4 * sizeof(float));
                break;

            case SensorType::GRAVITY:
                std::array<float, 3> gravity;

                err = sensorsFusion.getGravity(gravity);
                if (err < 0)
                    continue;

                memcpy(outdata.processed, gravity.data(), 3 * sizeof(float));
                break;

            case SensorType::LINEAR_ACCELERATION:
                std::array<float, 3> linearAccel;

                err = sensorsFusion.getLinearAccel(linearAccel);
                if (err < 0)
                    continue;

                memcpy(outdata.processed, linearAccel.data(), 3 * sizeof(float));
                break;

            default:
                return;
            }

            push_data.sb[i]->ReceiveDataFromDependency(sensor_t_data.handle, &outdata);
        }
    }
}

} // namespace core
} // namespace stm
