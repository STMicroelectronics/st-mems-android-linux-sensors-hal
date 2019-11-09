/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019 STMicroelectronics
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

#include <memory>
#include <cerrno>

#include "androidVersion.h"
#include "SensorsLegacyInterface.h"
#include <IConsole.h>

static const IConsole &console { IConsole::getInstance() };

static int st_hal_dev_close(struct hw_device_t *dev)
{
    //SensorsLegacyInterface *sensors = static_cast<SensorsLegacyInterface *>(dev->module->dso);
    (void) dev;

    return 0;
}

static int st_hal_open_sensors(const struct hw_module_t *module,
                               const char *id,
                               struct hw_device_t **device)
{
    const std::string sensorsModuleId { SENSORS_HARDWARE_POLL };
    std::unique_ptr<sensors_poll_device_1> pollDevice;
    std::unique_ptr<SensorsLegacyInterface> sensors;

    if (sensorsModuleId.compare(id)) {
        console.error("not a valid module ID");
        return -EINVAL;
    }

    pollDevice = std::make_unique<sensors_poll_device_1>();
    if (pollDevice == nullptr) {
        return -ENOMEM;
    }

    sensors = std::make_unique<SensorsLegacyInterface>();
    if (sensors == nullptr) {
        return -ENOMEM;
    }

    pollDevice->common.tag = HARDWARE_DEVICE_TAG;
    pollDevice->common.version = SENSORS_DEVICE_API_VERSION_1_4;
    pollDevice->common.module = const_cast<hw_module_t *>(module);
    pollDevice->common.close = st_hal_dev_close;
    pollDevice->common.module->dso = sensors.get();
    pollDevice->activate = sensors->activate;
    pollDevice->setDelay = sensors->setDelay;
    pollDevice->poll = sensors->poll;
    pollDevice->batch = sensors->batch;
    pollDevice->flush = sensors->flush;
    pollDevice->inject_sensor_data = sensors->injectSensorData;

#if ANDROID_VERSION_CODE >= ANDROID_VERSION(8, 0, 0)
    pollDevice->register_direct_channel = sensors->registerDirectChannel;
    pollDevice->config_direct_report = sensors->configDirectChannel;
#endif /* ANDROID_VERSION_CODE */

    *device = &pollDevice->common;

    return sensors->initialize();
}

static int st_hal_get_sensors_list(struct sensors_module_t *module,
                                   struct sensor_t const **list)
{
    SensorsLegacyInterface *sensors = static_cast<SensorsLegacyInterface *>(module->common.dso);

    return sensors->getSensorsList(list);
}

static int st_hal_set_operation_mode(unsigned int mode)
{
    (void) mode;

    return -EINVAL;
}

static struct hw_module_methods_t st_hal_sensors_module_methods = {
    .open = st_hal_open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = SENSORS_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "STMicroelectronics Sensors Module",
        .author = "STMicroelectronics",
        .methods = &st_hal_sensors_module_methods,
        .dso = NULL,
        .reserved = { },
    },
    .get_sensors_list = st_hal_get_sensors_list,
    .set_operation_mode = st_hal_set_operation_mode,
};
