/*
 * STMicroelectronics Accel-Magn Fusion 6X Sensor Class
 *
 * Copyright 2015-2016 STMicroelectronics Inc.
 * Author: Denis Ciocca - <denis.ciocca@st.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#define __STDC_LIMIT_MACROS
#define __STDINT_LIMITS

#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>

#include "SWAccelMagnFusion6X.h"

namespace stm {
namespace core {

SWAccelMagnFusion6X::SWAccelMagnFusion6X(const char *name, int handle, int module)
    : SWSensorBaseWithPollrate(name, handle, AccelMagnFusion6XSensorType,
                               false, false, true, false, module),
      geomagFusion(STMGeomagFusion::getInstance())
{
    sensor_t_data.minRateHz = CONFIG_ST_HAL_MIN_FUSION_POLLRATE;

    sensor_t_data.resolution = ST_SENSOR_FUSION_RESOLUTION(1.0f);
    sensor_t_data.maxRange = 1.0f;

    CustomInit();
    dependencies_type_list.push_back(AccelSensorType);
    dependencies_type_list.push_back(MagnSensorType);
    id_sensor_trigger = SENSOR_DEPENDENCY_ID_0;
}

int SWAccelMagnFusion6X::CustomInit()
{
    if (HAL_ENABLE_GEOMAG_FUSION == 0) {
        InvalidThisClass();
    }

    return 0;
}

int SWAccelMagnFusion6X::libsInit(void)
{
    std::string libVersionMsg { "geomag fusion library: " };
    int err = 0;

    if (HAL_ENABLE_GEOMAG_FUSION != 0) {
        err = geomagFusion.init(100);
        libVersionMsg += geomagFusion.getLibVersion();
    } else {
        libVersionMsg += std::string("not enabled!");
    }

    console.info(libVersionMsg);

    return err;
}

int SWAccelMagnFusion6X::Enable(int handle, bool enable, bool lock_en_mutex)
{
    int err;
    bool old_status;
    bool old_status_no_handle;

    if (lock_en_mutex)
        pthread_mutex_lock(&enable_mutex);

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
        if (enable)
            sensor_global_enable = utils.getTime();
        else
            sensor_global_disable = utils.getTime();
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
}

int SWAccelMagnFusion6X::SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mutex)
{
    int err;

    if ((period_ns > FREQUENCY_TO_NS(CONFIG_ST_HAL_MIN_FUSION_POLLRATE)) && (period_ns != INT64_MAX))
        period_ns = FREQUENCY_TO_NS(CONFIG_ST_HAL_MIN_FUSION_POLLRATE);

    if (lock_en_mutex)
        pthread_mutex_lock(&enable_mutex);

    err = SWSensorBaseWithPollrate::SetDelay(handle, period_ns, timeout, false);
    if (err < 0) {
        if (lock_en_mutex) {
            pthread_mutex_lock(&enable_mutex);
        }

        return err;
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;
}

void SWAccelMagnFusion6X::ProcessData(SensorBaseData *data)
{

    if (HAL_ENABLE_GEOMAG_FUSION != 0) {
        unsigned int i;
        int err, nomaxdata = 10;
        SensorBaseData magn_data;

        do {
            err = GetLatestValidDataFromDependency(SENSOR_DEPENDENCY_ID_1,
                                                   &magn_data,
                                                   data->timestamp);
            if (err < 0) {
                usleep(10);
                nomaxdata--;
                continue;
            }
        } while ((nomaxdata >= 0) && (err < 0));

        if (nomaxdata > 0) {
            int64_t delta_ms;
            std::array<float, 3> accelData;
            std::array<float, 3> magnData;

            memcpy(magnData.data(), magn_data.raw, 3 * sizeof(float));
            memcpy(accelData.data(), data->processed, 3 * sizeof(float));

            delta_ms = (data->timestamp - sensor_event.timestamp) / 1000000;
            geomagFusion.run(accelData, magnData, delta_ms);
        }

        sensor_event.timestamp = data->timestamp;
        outdata.timestamp = data->timestamp;
        outdata.flushEventHandles = data->flushEventHandles;
        outdata.flushEventsNum = data->flushEventsNum;
        outdata.accuracy = data->accuracy;
        outdata.pollrate_ns = data->pollrate_ns;

        for (i = 0; i < push_data.num; i++) {
            if (!push_data.sb[i]->ValidDataToPush(outdata.timestamp))
                continue;

            switch ((SensorType)push_data.sb[i]->GetType()) {
            case SensorType::ROTATION_VECTOR:
            case SensorType::GEOMAGNETIC_ROTATION_VECTOR:
                std::array<float, 4> quaternion;

                err = geomagFusion.getQuaternion(quaternion);
                if (err < 0)
                    continue;

                memcpy(outdata.processed, quaternion.data(), 4 * sizeof(float));

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
