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
#include <string.h>
#include <unistd.h>

#include "SWSensorBase.h"

namespace stm {
namespace core {

#define SW_SENSOR_BASE_DEELAY_PROCESSING_DATA            (500000000LL)

static IConsole &console { IConsole::getInstance() };

SWSensorBase::SWSensorBase(const char *name, int handle, STMSensorType sensor_type,
                           bool use_dependency_resolution, bool use_dependency_range,
                           bool use_dependency_delay, bool use_dependency_name)
    : SensorBase(name, handle, sensor_type)
{
    int err, pipe_fd[2];

    dependency_resolution = use_dependency_resolution;
    dependency_range = use_dependency_range;
    dependency_delay = use_dependency_delay;
    dependency_name = use_dependency_name;

    err = pipe(pipe_fd);
    if (err < 0) {
        console.error(std::string(GetName()) + ": Failed to create pipe file.");
        goto invalid_this_class;
    }

    fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK);

    trigger_write_pipe_fd = pipe_fd[1];
    trigger_read_pipe_fd = pipe_fd[0];

    android_pollfd.events = POLLIN;
    android_pollfd.fd = trigger_read_pipe_fd;

    return;

invalid_this_class:
    InvalidThisClass();
}

SWSensorBase::~SWSensorBase()
{
    close(trigger_write_pipe_fd);
    close(trigger_read_pipe_fd);
}

int SWSensorBase::Enable(int handle, bool enable, bool lock_en_mutex)
{
    int err;
    bool old_status, old_status_no_handle;
    int64_t timestampEnable;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    old_status = GetStatus(false);
    old_status_no_handle = GetStatusExcludeHandle(handle);
    timestampEnable = utils.getTime();

    err = SensorBase::Enable(handle, enable, false);
    if (err < 0) {
        goto unlock_mutex;
    }

    if ((enable && !old_status) || (!enable && !old_status_no_handle)) {
        if (enable) {
            sensor_global_enable = timestampEnable;
        } else {
            sensor_global_disable = utils.getTime();
        }
    }

    if (sensor_t_data.handle == handle) {
        if (enable) {
            sensor_my_enable = timestampEnable;
        } else {
            sensor_my_disable = utils.getTime();
        }
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;

unlock_mutex:
    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return err;
}

int SWSensorBase::AddSensorDependency(SensorBase *p)
{
    int err;
    DependencyID dependency_ID;

    err = SensorBase::AddSensorDependency(p);
    if (err < 0) {
        return err;
    }

    dependency_ID = (DependencyID)err;

    if (sensor_t_data.fifoMaxEventCount == 0) {
        sensor_t_data.fifoMaxEventCount = p->GetMaxFifoLenght();
    } else {
        if (p->GetMaxFifoLenght() < (int)sensor_t_data.fifoMaxEventCount) {
            sensor_t_data.fifoMaxEventCount = p->GetMaxFifoLenght();
        }
    }

    struct sensor_t dependecy_data = p->GetSensor_tData();

    if (dependency_resolution) {
        sensor_t_data.resolution = dependecy_data.resolution;
    }

    if (dependency_range) {
        sensor_t_data.maxRange = dependecy_data.maxRange;
    }

    if (dependency_delay) {
        if (dependency_ID == id_sensor_trigger) {
            sensor_t_data.maxRateHz = dependecy_data.maxRateHz;

            if ((sensor_t_data.minRateHz == 0) || (sensor_t_data.minRateHz > dependecy_data.minRateHz)) {
                sensor_t_data.minRateHz = dependecy_data.minRateHz;
            }
        }
    }

    if (dependency_name) {
        memcpy((char *)sensor_t_data.name, dependecy_data.name, strlen(dependecy_data.name) + 1);
    }

    err = AllocateBufferForDependencyData(dependency_ID, p->GetMaxFifoLenght());
    if (err < 0) {
        return err;
    }

    return 0;
}

void SWSensorBase::RemoveSensorDependency(SensorBase *p)
{
    DeAllocateBufferForDependencyData(GetDependencyIDFromHandle(p->GetHandle()));
    SensorBase::RemoveSensorDependency(p);
}

int SWSensorBase::FlushData(int handle, bool lock_en_mutex)
{
    int err;
    unsigned int i;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    if (GetStatus(false)) {
        if (GetMinTimeout(false) > 0) {
            for (i = 0; i < dependencies.num; i++) {
                err = dependencies.sb[i]->FlushData(handle, true);
                if (err < 0) {
                    goto unlock_mutex;
                }
            }
        } else {
            ProcessFlushData(handle, 0);
        }
    } else {
        goto unlock_mutex;
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;

unlock_mutex:
    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return -EINVAL;
}

bool SWSensorBase::ValidDataToPush(int64_t timestamp)
{
    if (sensor_my_enable > sensor_my_disable) {
        if (timestamp > sensor_my_disable) {
            return true;
        }
    } else {
        if ((timestamp > sensor_my_enable) && (timestamp < sensor_my_disable)) {
            return true;
        }
    }

    return false;
}

void SWSensorBase::ProcessFlushData(int handle, int64_t timestamp)
{
    unsigned int i;
    int err;

    pthread_mutex_lock(&sample_in_processing_mutex);

    if (sensor_t_data.type.isInternal()) {
        for (i = 0; i < push_data.num; i++) {
            push_data.sb[i]->ProcessFlushData(handle, timestamp);
        }
    } else {
        if (ValidDataToPush(timestamp)) {
            if (timestamp > sample_in_processing_timestamp) {
                err = flush_stack.writeElement(handle, timestamp);
                if (err < 0) {
                    console.error(std::string(GetName()) + ": Failed to write Flush event into stack.");
                }
            } else {
                if (handle == sensor_t_data.handle) {
                    WriteFlushEventToPipe();
                } else {
                    for (i = 0; i < push_data.num; i++) {
                        push_data.sb[i]->ProcessFlushData(handle, timestamp);
                    }
                }
            }
        }
    }

    pthread_mutex_unlock(&sample_in_processing_mutex);
}

void SWSensorBase::ReceiveDataFromDependency(int handle, SensorBaseData *data)
{
    int err;
    bool valid_data = false;

    if (sensor_global_enable > sensor_global_disable) {
        if (data->timestamp > sensor_global_enable) {
            valid_data = true;
        }
    } else {
        if ((data->timestamp > sensor_global_enable) && (data->timestamp < sensor_global_disable)) {
            valid_data = true;
        }
    }

    if (valid_data) {
        if (id_sensor_trigger == GetDependencyIDFromHandle(handle)) {
            err = write(trigger_write_pipe_fd, data, sizeof(SensorBaseData));
            if (err <= 0) {
                console.error(std::string(android_name) + ": Failed to write trigger data to pipe.");
            }
        } else {
            SensorBase::ReceiveDataFromDependency(handle, data);
        }
    }
}

void SWSensorBase::ThreadDataTask()
{
    int err, flush_handle;
    unsigned int i, fifo_len;
    int64_t timestamp_flush;

    if (sensor_t_data.fifoMaxEventCount > 0) {
        fifo_len = 2 * sensor_t_data.fifoMaxEventCount;
    }  else {
        fifo_len = 2;
    }

    sensors_tmp_data = (SensorBaseData *)malloc(fifo_len * sizeof(SensorBaseData));
    if (!sensors_tmp_data) {
        console.error(std::string(GetName()) + ": Failed to allocate sensor data buffer.");
        return;
    }

    while (1) {
        err = poll(&android_pollfd, 1, -1);
        if (err < 0)
            continue;

        if (android_pollfd.revents & POLLIN) {
            err = read(android_pollfd.fd, sensors_tmp_data, fifo_len * sizeof(SensorBaseData));
            if (err <= 0) {
                console.error(std::string(GetName()) + ": Failed to read data from pipe.");
                continue;
            }

            for (i = 0; i < err / sizeof(SensorBaseData); i++) {
                pthread_mutex_lock(&sample_in_processing_mutex);
                sample_in_processing_timestamp = sensors_tmp_data[i].timestamp;
                pthread_mutex_unlock(&sample_in_processing_mutex);

                bool retry = false;

                do {
                    flush_handle = flush_stack.readLastElement(&timestamp_flush);
                    if ((flush_handle >= 0) && (timestamp_flush <= sensors_tmp_data[i].timestamp)) {
                        sensors_tmp_data[i].flushEventHandles[sensors_tmp_data[i].flushEventsNum++] = flush_handle;
                        flush_stack.removeLastElement();
                        retry = true;
                    } else {
                        retry = false;
                    }
                } while (retry  && sensors_tmp_data[i].flushEventsNum <= (int)sensors_tmp_data[i].flushEventHandles.size());

                this->ProcessData(&sensors_tmp_data[i]);
            }
        }
    }
}

SWSensorBaseWithPollrate::SWSensorBaseWithPollrate(const char *name, int handle, STMSensorType sensor_type,
                                                   bool use_dependency_resolution,
                                                   bool use_dependency_range, bool use_dependency_delay,
                                                   bool use_dependency_name)
    : SWSensorBase(name, handle, sensor_type,
                   use_dependency_resolution, use_dependency_range,
                   use_dependency_delay, use_dependency_name)
{

}

SWSensorBaseWithPollrate::~SWSensorBaseWithPollrate()
{

}

int SWSensorBaseWithPollrate::SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mutex)
{
    int err;
    int64_t min_pollrate_ns, min_timeout_ns;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    if ((sensors_pollrates[handle] == period_ns) && (sensors_timeout[handle] == timeout)) {
        err = 0;
        goto mutex_unlock;
    }

    if ((period_ns > 0) && (timeout < INT64_MAX)) {
        if (period_ns > (((int64_t)FREQUENCY_TO_US(sensor_t_data.minRateHz)) * 1000)) {
            period_ns = FREQUENCY_TO_US(sensor_t_data.minRateHz) * 1000;
        }

        if ((period_ns < (((int64_t)FREQUENCY_TO_US(sensor_t_data.maxRateHz)) * 1000)) && (period_ns > 0)) {
            period_ns = FREQUENCY_TO_US(sensor_t_data.maxRateHz) * 1000;
        }

        if (timeout < SW_SENSOR_BASE_DEELAY_PROCESSING_DATA) {
            timeout = 0;
        } else {
            timeout -= SW_SENSOR_BASE_DEELAY_PROCESSING_DATA;
        }
    }

    err = SensorBase::SetDelay(handle, period_ns, timeout, false);
    if (err < 0) {
        goto mutex_unlock;
    }

    min_pollrate_ns = GetMinPeriod(false);
    if (min_pollrate_ns == 0) {
        err = 0;
        current_min_pollrate = 0;
        current_min_timeout = INT64_MAX;
        goto mutex_unlock;
    }

    min_timeout_ns = GetMinTimeout(false);

    if ((current_min_pollrate != min_pollrate_ns) || (current_min_timeout != min_timeout_ns)) {
        current_min_pollrate = min_pollrate_ns;
        current_min_timeout = min_timeout_ns;

        if (handle == sensor_t_data.handle) {
            AddNewPollrate(utils.getTime(), period_ns);
        }

        const std::string msg = ": changed pollrate to " +
            std::to_string(NS_TO_FREQUENCY((float)(uint64_t)min_pollrate_ns)) + "Hz, timeout=" +
            std::to_string((uint64_t)NS_TO_MS((uint64_t)min_timeout_ns)) + "ms";

        console.debug(GetName() +  msg);
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;

mutex_unlock:
    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return err;
}

int SWSensorBaseWithPollrate::FlushData(int handle, bool lock_en_mutex)
{
    int err;
    unsigned int i;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    if (GetStatus(false)) {
        if (GetMinTimeout(false) > 0) {
            for (i = 0; i < dependencies.num; i++) {
                err = dependencies.sb[i]->FlushData(handle, true);
                if (err < 0) {
                    goto unlock_mutex;
                }
            }
        } else {
            ProcessFlushData(handle, utils.getTime());
        }
    } else {
        goto unlock_mutex;
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;

unlock_mutex:
    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return -EINVAL;
}

void SWSensorBaseWithPollrate::WriteDataToPipe(int64_t hw_pollrate)
{
    int64_t timestamp_change = 0, new_pollrate = 0;
    bool odr_changed = false;
    float temp;
    int err;

    err = CheckLatestNewPollrate(&timestamp_change, &new_pollrate);
    if ((err >= 0) && (sensor_event.timestamp > timestamp_change)) {
        current_real_pollrate = new_pollrate;
        DeleteLatestNewPollrate();
        odr_changed = true;
    }

    temp = (float)current_real_pollrate / hw_pollrate;
    decimator = (int)(temp + (temp / 20));
    samples_counter++;

    if (decimator == 0) {
        decimator = 1;
    }

    int64_t decimatedPollrate = decimator * hw_pollrate;
    if (decimatedPollrate != lastDecimatedPollrate) {
        WriteOdrChangeEventToPipe(sensor_event.timestamp, decimatedPollrate);
    }
    lastDecimatedPollrate = decimatedPollrate;

    if (((samples_counter % decimator) == 0) || odr_changed) {
        err = write(write_pipe_fd, &sensor_event, sizeof(sensors_event_t));
        if (err <= 0) {
            console.error(std::string(android_name) + ": Failed to write sensor data to pipe.");
            samples_counter--;
            return;
        }

        samples_counter = 0;
        last_data_timestamp = sensor_event.timestamp;
    }
}

} // namespace core
} // namespace stm
