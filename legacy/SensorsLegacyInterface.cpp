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

#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#include <IConsole.h>
#include "Convert.h"
#include "SensorsLegacyInterface.h"

SensorsLegacyInterface::SensorsLegacyInterface(void)
                       : sensorsCore(ISTMSensorsHAL::getInstance()),
                         console(IConsole::getInstance())
{
}

SensorsLegacyInterface::~SensorsLegacyInterface(void)
{
    for (auto fd : pipeFd) {
        if (fd > 0) {
            close(fd);
        }
    }
}

/**
 * initialize: initialize the interface
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsLegacyInterface::initialize(void)
{
    sensorsCore.initialize(*dynamic_cast<ISTMSensorsCallback *>(this));

    if (initializePipe()) {
        return -EINVAL;
    }

    return 0;
}

/**
 * getSensorsList: retrieve Android compatible sensors list
 * @sensorsList: Android framework pointer used to link the sensors list.
 *
 * Return value: number of sensors available.
 */
unsigned int SensorsLegacyInterface::getSensorsList(struct sensor_t const **sensorsList) const
{
    const std::vector<STMSensor> &list = sensorsCore.getSensorsList().getList();
    auto count = list.size();
    auto n {0U};

    if (count == 0) {
        return 0;
    }

    *sensorsList = (struct sensor_t *)malloc(count * sizeof(struct sensor_t));
    if (*sensorsList == nullptr) {
        return 0;
    }

    for (auto i = 0U; i < count; i++) {
        if (convertFromSTMSensor(list.at(i), (struct sensor_t *)&((*sensorsList)[n]))) {
            n++;
        }
    }

    if (n != count) {
        *sensorsList = (struct sensor_t *)realloc((void *)*sensorsList,
                                                  n * sizeof(struct sensor_t));
    }

    return n;
}

/**
 * activate: framework defined function,
 *           reference: hardware/libhardware/include/hardware/sensors.h
 */
int SensorsLegacyInterface::activate(struct sensors_poll_device_t *dev,
                                     int sensor_handle, int enabled)
{
    SensorsLegacyInterface *sensors = static_cast<SensorsLegacyInterface *>(dev->common.module->dso);

    return sensors->sensorsCore.activate(sensor_handle, enabled);
}

/**
 * setDelay: framework defined function,
 *           reference: hardware/libhardware/include/hardware/sensors.h
 */
int SensorsLegacyInterface::setDelay(struct sensors_poll_device_t *dev,
                                     int sensor_handle, int64_t sampling_period_ns)
{
    SensorsLegacyInterface *sensors = static_cast<SensorsLegacyInterface *>(dev->common.module->dso);

    return sensors->sensorsCore.setRate(sensor_handle, sampling_period_ns, 0);
}

/**
 * poll: framework defined function,
 *       reference: hardware/libhardware/include/hardware/sensors.h
 */
int SensorsLegacyInterface::poll(struct sensors_poll_device_t *dev,
                                 sensors_event_t *data, int count)
{
    SensorsLegacyInterface *sensors = static_cast<SensorsLegacyInterface *>(dev->common.module->dso);

    if (::poll(&sensors->sensorsDataPollFd, 1, -1) < 0) {
        return 0;
    }

    if (sensors->sensorsDataPollFd.revents & POLLIN) {
        auto len = read(sensors->sensorsDataPollFd.fd, data, count * sizeof(sensors_event_t));
        if (len == -1) {
            return 0;
        }

        return len / sizeof(sensors_event_t);
    }

    return 0;
}

/**
 * batch: framework defined function,
 *        reference: hardware/libhardware/include/hardware/sensors.h
 */
int SensorsLegacyInterface::batch(struct sensors_poll_device_1 *dev,
                                  int sensor_handle, int flags,
                                  int64_t sampling_period_ns,
                                  int64_t max_report_latency_ns)
{
    SensorsLegacyInterface *sensors = static_cast<SensorsLegacyInterface *>(dev->common.module->dso);
    (void) flags;

    return sensors->sensorsCore.setRate(sensor_handle, sampling_period_ns, max_report_latency_ns);
}

/**
 * flush: framework defined function,
 *        reference: hardware/libhardware/include/hardware/sensors.h
 */
int SensorsLegacyInterface::flush(struct sensors_poll_device_1 *dev,
                                  int sensor_handle)
{
    SensorsLegacyInterface *sensors = static_cast<SensorsLegacyInterface *>(dev->common.module->dso);

    return sensors->sensorsCore.flushData(sensor_handle);
}

/**
 * injectSensorData: framework defined function,
 *                   reference: hardware/libhardware/include/hardware/sensors.h
 */
int SensorsLegacyInterface::injectSensorData(struct sensors_poll_device_1 *dev,
                                             const sensors_event_t *data)
{
    (void) dev;
    (void) data;

    return -EPERM;
}

/**
 * registerDirectChannel: framework defined function,
 *                        reference: hardware/libhardware/include/hardware/sensors.h
 */
int SensorsLegacyInterface::registerDirectChannel(struct sensors_poll_device_1 *dev,
                                                  const struct sensors_direct_mem_t *mem,
                                                  int channel_handle)
{
    (void) dev;
    (void) mem;
    (void) channel_handle;

    return -EPERM;
}

/**
 * configDirectChannel: framework defined function,
 *                      reference: hardware/libhardware/include/hardware/sensors.h
 */
int SensorsLegacyInterface::configDirectChannel(struct sensors_poll_device_1 *dev,
                                                int sensor_handle, int channel_handle,
                                                const struct sensors_direct_cfg_t *config)
{
    (void) dev;
    (void) sensor_handle;
    (void) channel_handle;
    (void) config;

    return -EPERM;
}

/**
 * onNewSensorsData: receive data from STMSensorsHAL,
 *                   reference: ISTMSensorsCallbackData class
 */
void SensorsLegacyInterface::onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData)
{
    for (auto sdata : sensorsData) {
        sensors_event_t event;

        if (!convertFromSTMSensorType(sdata.getSensorType(), event.type)) {
            console.error("sensor event unknown, discarding...");
            continue;
        }

        event.version = 1;
        event.sensor = sdata.getSensorHandle();
        event.timestamp = sdata.getTimestamp();
        convertFromSTMSensorData(sdata, &event);

        if (sizeof(sensors_event_t) > PIPE_BUF) {
            console.error("cannot write a sensor event atomically!");
            return;
        }

        if (write(pipeFd[1], &event, sizeof(sensors_event_t)) <= 0) {
            console.error("failed to write data to pipe");
        }
    }
}

/**
 * initializePipe: initialize pipe used to send data to framework thread
 *
 * Return value: 0 on success, else a negative error code.
 */
int SensorsLegacyInterface::initializePipe(void)
{
    if (pipe(pipeFd)) {
        return -errno;
    }

    if (fcntl(pipeFd[0], F_SETFL, O_NONBLOCK) == -1) {
        return -errno;
    }

    sensorsDataPollFd.events = POLLIN;
    sensorsDataPollFd.fd = pipeFd[0];

    return 0;
}
