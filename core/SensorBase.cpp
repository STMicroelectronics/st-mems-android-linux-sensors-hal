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

#include <stdint.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "SensorBase.h"

namespace stm {
namespace core {

static IConsole &console { IConsole::getInstance() };

SensorBase::SensorBase(const char *name, int handle, const STMSensorType &type, int module)
    : sensor_t_data(type),
      threadsRunning(true),
      sensorsCallback(nullptr),
      moduleId(module)
{
    int i, err, pipe_fd[2];

    if (strlen(name) + 1 > SENSOR_BASE_ANDROID_NAME_MAX) {
        memcpy(android_name, name, SENSOR_BASE_ANDROID_NAME_MAX - 1);
        android_name[SENSOR_BASE_ANDROID_NAME_MAX - 1] = '\0';
    } else {
        memcpy(android_name, name, strlen(name) + 1);
    }

    valid_class = true;
    memset(&push_data, 0, sizeof(dependencies_t));
    memset(&dependencies, 0, sizeof(push_data_t));
    //memset(&sensor_t_data, 0, sizeof(struct sensor_t));
    // memset(&sensor_event, 0, sizeof(sensors_event_t));
    memset(sensors_pollrates, 0, ST_HAL_IIO_MAX_DEVICES * sizeof(int64_t));

    for (i = 0; i < ST_HAL_IIO_MAX_DEVICES; i++) {
        sensors_timeout[i] = INT64_MAX;
    }

    sensor_event.version = sizeof(sensors_event_t);
    sensor_event.sensor = handle;

    if (!type.isInternal()) {
        sensor_event.type = static_cast<SensorType>(type);
    }

    sensor_t_data.name = android_name;
    sensor_t_data.handle = handle;
    //sensor_t_data.type = type;
    sensor_t_data.vendor = "STMicroelectronics";

    last_data_timestamp = 0;
    enabled_sensors_mask = 0;
    current_real_pollrate = 0;
    sample_in_processing_timestamp = 0;
    current_min_pollrate = 0;
    current_min_timeout = INT64_MAX;
    sensor_global_enable = 0;
    sensor_global_disable = 1;
    sensor_my_enable = 0;
    sensor_my_disable = 1;
    decimator = 1;
    samples_counter = 0;

    injection_mode = SENSOR_INJECTION_NONE;

    write_pipe_fd = -EINVAL;
    read_pipe_fd = -EINVAL;

    pthread_mutex_init(&enable_mutex, NULL);
    pthread_mutex_init(&sample_in_processing_mutex, NULL);

    err = pipe(pipe_fd);
    if (err < 0) {
        console.error(GetName() + std::string(": Failed to create pipe file."));
        goto invalid_the_class;
    }

    fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK);

    write_pipe_fd = pipe_fd[1];
    read_pipe_fd = pipe_fd[0];

    return;

invalid_the_class:
    InvalidThisClass();
}

SensorBase::~SensorBase()
{
    threadsRunning = false;

    close(write_pipe_fd);
    close(read_pipe_fd);

    if (dataThread  && dataThread->joinable())
        dataThread->join();

    if (eventsThread && eventsThread->joinable())
        eventsThread->join();
}

DependencyID SensorBase::GetDependencyIDFromHandle(int handle)
{
    return handle_remapping_ID[handle];
}

void SensorBase::SetDependencyIDOfHandle(int handle, DependencyID id)
{
    handle_remapping_ID[handle] = id;
}

void SensorBase::InvalidThisClass()
{
    valid_class = false;
}

bool SensorBase::IsValidClass()
{
    return valid_class;
}

int SensorBase::GetHandle()
{
    return sensor_t_data.handle;
}

STMSensorType SensorBase::GetType()
{
    return sensor_t_data.type;
}

int SensorBase::GetMaxFifoLenght()
{
    return sensor_t_data.fifoMaxEventCount;
}

int SensorBase::GetFdPipeToRead()
{
    return read_pipe_fd;
}

void SensorBase::SetBitEnableMask(int handle)
{
    enabled_sensors_mask |= (1ULL << handle);
}

void SensorBase::ResetBitEnableMask(int handle)
{
    enabled_sensors_mask &= ~(1ULL << handle);
}

int SensorBase::AddNewPollrate(int64_t timestamp, int64_t pollrate)
{
    return odr_stack.writeElement(timestamp, pollrate);
}

int SensorBase::CheckLatestNewPollrate(int64_t *timestamp, int64_t *pollrate)
{
    *timestamp = odr_stack.readLastElement(pollrate);
    if (*timestamp < 0) {
        return -EINVAL;
    }

    return 0;
}

void SensorBase::DeleteLatestNewPollrate()
{
    odr_stack.removeLastElement();
}

bool SensorBase::ValidDataToPush(int64_t timestamp)
{
    if (sensor_my_enable > sensor_my_disable) {
        if (timestamp > sensor_my_enable) {
            return true;
        }
    } else {
        if ((timestamp > sensor_my_enable) && (timestamp < sensor_my_disable)) {
            return true;
        }
    }

    return false;
}

bool SensorBase::GetDependencyMaxRange(STMSensorType type, float *maxRange)
{
    bool found;
    unsigned int i;
    float maxRange_priv = 0;

    if (sensor_t_data.type == type) {
        *maxRange = sensor_t_data.maxRange;
        return true;
    }

    for (i = 0; i < dependencies.num; i++) {
        found = dependencies.sb[i]->GetDependencyMaxRange(type, &maxRange_priv);
        if (found) {
            *maxRange = maxRange_priv;
            return true;
        }
    }

    return false;
}

char* SensorBase::GetName()
{
    return (char *)sensor_t_data.name;
}

selftest_status SensorBase::ExecuteSelfTest()
{
    return NOT_AVAILABLE;
}

int SensorBase::Enable(int handle, bool enable, bool lock_en_mutex)
{
    int err = 0;
    unsigned int i = 0;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    if ((handle == sensor_t_data.handle) && (enable == GetStatusOfHandle(handle))) {
        goto enable_unlock_mutex;
    }

    if ((enable && !GetStatus(false)) || (!enable && !GetStatusExcludeHandle(handle))) {
        if (enable) {
            SetBitEnableMask(handle);
            flush_stack.resetBuffer();
            lastDecimatedPollrate = 0;
        } else {
            err = SetDelay(handle, 0, INT64_MAX, false);
            if (err < 0) {
                goto enable_unlock_mutex;
            }

            ResetBitEnableMask(handle);
        }

        for (i = 0; i < dependencies.num; i++) {
            err = dependencies.sb[i]->Enable(sensor_t_data.handle, enable, true);
            if (err < 0) {
                goto restore_enable_dependencies;
            }
        }

        if (enable) {
            console.debug(GetName() + std::string(": power-on"));
        }  else {
            console.debug(GetName() + std::string(": power-off"));
        }
    } else {
        if (enable) {
            SetBitEnableMask(handle);
        }  else {
            err = SetDelay(handle, 0, INT64_MAX, false);
            if (err < 0) {
                goto enable_unlock_mutex;
            }

            ResetBitEnableMask(handle);
        }
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;

restore_enable_dependencies:
    while (i > 0) {
        i--;
        dependencies.sb[i]->Enable(sensor_t_data.handle, !enable, true);
    }

    if (enable) {
        ResetBitEnableMask(handle);
    }  else {
        SetBitEnableMask(handle);
    }
 enable_unlock_mutex:
    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return err;
}

bool SensorBase::GetStatusExcludeHandle(int handle)
{
    return (enabled_sensors_mask & ~(1ULL << handle)) > 0 ? true : false;
}

bool SensorBase::GetStatusOfHandle(int handle)
{
    return (enabled_sensors_mask & (1ULL << handle)) > 0 ? true : false;
}

bool SensorBase::GetStatusOfHandle(int handle, bool lock_en_mutex)
{
    bool status;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    status = (enabled_sensors_mask & (1ULL << handle)) > 0 ? true : false;

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return status;
}

bool SensorBase::GetStatus(bool lock_en_mutex)
{
    bool status;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    status = enabled_sensors_mask > 0 ? true : false;

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return status;
}

int SensorBase::SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mutex)
{
    int err, i;
    int64_t restore_min_timeout, restore_min_period_ms;

    if ((timeout > 0) && (timeout < INT64_MAX) && (sensor_t_data.fifoMaxEventCount == 0)) {
        return -EINVAL;
    }

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    restore_min_timeout = sensors_timeout[handle];
    restore_min_period_ms = sensors_pollrates[handle];

    sensors_pollrates[handle] = period_ns;
    sensors_timeout[handle] = timeout;

    for (i = 0; i < (int)dependencies.num; i++) {
        err = dependencies.sb[i]->SetDelay(sensor_t_data.handle, GetMinPeriod(false), GetMinTimeout(false), true);
        if (err < 0) {
            goto restore_delay_dependencies;
        }
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return 0;

 restore_delay_dependencies:
    sensors_pollrates[handle] = restore_min_period_ms;
    sensors_timeout[handle] = restore_min_timeout;

    for (i--; i >= 0; i--) {
        dependencies.sb[i]->SetDelay(sensor_t_data.handle, GetMinPeriod(false), GetMinTimeout(false), true);
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return err;
}

const std::vector<STMSensorType>& SensorBase::GetDepenciesTypeList(void) const
{
    return dependencies_type_list;
}

int SensorBase::AllocateBufferForDependencyData(DependencyID id, unsigned int max_fifo_len)
{
    circular_buffer_data[id] = new CircularBuffer(max_fifo_len < 2 ? 10 : 10 * max_fifo_len);
    if (!circular_buffer_data[id]) {
        console.error(GetName() + std::string(": Failed to allocate circular buffer data."));
        return -ENOMEM;
    }

    return 0;
}

void SensorBase::DeAllocateBufferForDependencyData(DependencyID id)
{
    delete circular_buffer_data[id];
}

int SensorBase::AddSensorToDataPush(SensorBase *t)
{
    if (push_data.num >= SENSOR_DEPENDENCY_ID_MAX) {
        console.error(android_name + std::string(": Failed to add dependency data, too many sensors to push data."));
        return -ENOMEM;
    }

    push_data.sb[push_data.num] = t;
    push_data.num++;

    return 0;
}

void SensorBase::RemoveSensorToDataPush(SensorBase *t)
{
    unsigned int i;

    for (i = 0; i < push_data.num; i++) {
        if (t == push_data.sb[i]) {
            break;
        }
    }
    if (i == push_data.num) {
        return;
    }

    for (; i < push_data.num - 1; i++) {
        push_data.sb[i] = push_data.sb[i + 1];
    }

    push_data.num--;
}

int SensorBase::AddSensorDependency(SensorBase *p)
{
    int err;
    unsigned int dependency_id;

    if (dependencies.num >= SENSOR_DEPENDENCY_ID_MAX) {
        console.error(android_name + std::string(": Failed to add dependency, too many dependencies."));
        return -ENOMEM;
    }

    dependency_id = dependencies.num;
    SetDependencyIDOfHandle(p->GetHandle(), (DependencyID)dependency_id);

    err = p->AddSensorToDataPush(this);
    if (err < 0) {
        return err;
    }

    struct sensor_t dependecy_data = p->GetSensor_tData();
    sensor_t_data.power += dependecy_data.power;

    dependencies.sb[dependency_id] = p;
    dependencies.num++;

    return dependency_id;
}

void SensorBase::RemoveSensorDependency(SensorBase *p)
{
    unsigned int i;

    for (i = 0; i < dependencies.num; i++) {
        if (p == dependencies.sb[i]) {
            break;
        }
    }
    if (i == dependencies.num) {
        return;
    }

    p->RemoveSensorToDataPush(this);

    for (; i < dependencies.num - 1; i++) {
        dependencies.sb[i] = dependencies.sb[i + 1];
    }

    dependencies.num--;
}

int SensorBase::startThreads(void)
{
    if (hasDataChannels()) {
        dataThread = std::make_unique<std::thread>(ThreadDataWork, this, std::ref(threadsRunning));
    }

    if (hasEventChannels()) {
        eventsThread = std::make_unique<std::thread>(ThreadEventsWork, this, std::ref(threadsRunning));
    }

    return 0;
}

void SensorBase::stopThreads(void)
{

}

struct sensor_t SensorBase::GetSensor_tData(void)
{
   struct sensor_t data(sensor_t_data);

   return data;
}

void SensorBase::WriteOdrChangeEventToPipe(int64_t timestamp, int64_t pollrate)
{
    sensors_event_t odr_change_event_data;

    odr_change_event_data.sensor = sensor_t_data.handle;
    odr_change_event_data.timestamp = timestamp;
    odr_change_event_data.type = SensorType::ODR_SWITCH_INFO;
    odr_change_event_data.data.dataLen = 1;
    odr_change_event_data.data.data2[0] = pollrate;

    auto err = write(write_pipe_fd, &odr_change_event_data, sizeof(sensors_event_t));
    if (err <= 0) {
        console.error(android_name + std::string(": Failed to write odr change event data to pipe."));
    }
}

void SensorBase::WriteFlushEventToPipe()
{
    int err;
    sensors_event_t flush_event_data;

    // memset(&flush_event_data, 0, sizeof(sensors_event_t));

    flush_event_data.sensor = sensor_t_data.handle;
    flush_event_data.timestamp = 0;
    // flush_event_data.data_new[0] =
    // flush_event_data.meta_data.sensor = sensor_t_data.handle;
    //flush_event_data.meta_data.what = META_DATA_FLUSH_COMPLETE;
    flush_event_data.type = SensorType::META_DATA;
    //flush_event_data.version = META_DATA_VERSION;

    console.debug(GetName() + std::string(": write flush event to pipe"));

    err = write(write_pipe_fd, &flush_event_data, sizeof(sensors_event_t));
    if (err <= 0) {
        console.error(android_name + std::string(": Failed to write flush event data to pipe."));
    }
}

void SensorBase::WriteDataToPipe(int64_t __attribute__((unused))hw_pollrate)
{
    int err;

    if (ValidDataToPush(sensor_event.timestamp)) {
        if (sensor_event.timestamp > last_data_timestamp) {
            err = write(write_pipe_fd, &sensor_event, sizeof(sensors_event_t));
            if (err <= 0) {
                console.error(android_name + std::string(": Failed to write sensor data to pipe."));
                return;
            }

            last_data_timestamp = sensor_event.timestamp;
        }
    }
}

void SensorBase::ProcessData(SensorBaseData *data)
{
    unsigned int i;

    for (int i = 0; i < data->flushEventsNum; ++i) {
        if (data->flushEventHandles[i] == sensor_t_data.handle) {
            WriteFlushEventToPipe();
        }
    }

    for (i = 0; i < push_data.num; i++) {
        push_data.sb[i]->ReceiveDataFromDependency(sensor_t_data.handle, data);
    }
}

void SensorBase::ReceiveDataFromDependency(int handle, SensorBaseData *data)
{
    bool fill_buffer = false;

    if (sensor_global_enable > sensor_global_disable) {
        if (data->timestamp > sensor_global_enable) {
            fill_buffer = true;
        }
    } else {
        if ((data->timestamp > sensor_global_enable) && (data->timestamp < sensor_global_disable)) {
            fill_buffer = true;
        }
    }

    if (fill_buffer) {
        circular_buffer_data[GetDependencyIDFromHandle(handle)]->writeElement(data);
    }
}

int SensorBase::GetLatestValidDataFromDependency(int dependency_id, SensorBaseData *data, int64_t timesync)
{
    return circular_buffer_data[dependency_id]->readSyncElement(data, timesync);
}

int64_t SensorBase::GetMinTimeout(bool lock_en_mutex)
{
    int i;
    int64_t min = INT64_MAX;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    for (i = 0; i < ST_HAL_IIO_MAX_DEVICES; i++) {
        if ((sensors_timeout[i] < min) && (sensors_timeout[i] < INT64_MAX)) {
            min = sensors_timeout[i];
        }
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return min;
}

int64_t SensorBase::GetMinPeriod(bool lock_en_mutex)
{
    int i;
    int64_t min = INT64_MAX;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    for (i = 0; i < ST_HAL_IIO_MAX_DEVICES; i++) {
        if ((sensors_pollrates[i] < min) && (sensors_pollrates[i] > 0)) {
            min = sensors_pollrates[i];
        }
    }

    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return min == INT64_MAX ? 0 : min;
}

void *SensorBase::ThreadDataWork(void *context, std::atomic<bool>& threadsRunning)
{
    SensorBase *mypointer = (SensorBase *)context;

    mypointer->ThreadDataTask(threadsRunning);

    return mypointer;
}

void SensorBase::ThreadDataTask(std::atomic<bool>& threadsRunning)
{
    (void)threadsRunning;
}

void *SensorBase::ThreadEventsWork(void *context, std::atomic<bool>& threadsRunning)
{
    SensorBase *mypointer = (SensorBase *)context;

    mypointer->ThreadEventsTask(threadsRunning);

    return mypointer;
}

void SensorBase::ThreadEventsTask(std::atomic<bool>& threadsRunning)
{
    (void)threadsRunning;
}

int SensorBase::InjectionMode(bool __attribute__((unused))enable)
{
    return 0;
}

int SensorBase::InjectSensorData(const sensors_event_t __attribute__((unused))*data)
{
    return -EINVAL;
}

bool SensorBase::hasEventChannels()
{
    return false;
}

bool SensorBase::hasDataChannels()
{
    return false;
}

void SensorBase::setCallbacks(const ISTMSensorsCallback &sensorsCallback)
{
    this->sensorsCallback = (ISTMSensorsCallback *)&sensorsCallback;
}

int SensorBase::getHandleOfMyTrigger(void) const
{
    return -1;
}

} // namespace core
} // namespace stm
