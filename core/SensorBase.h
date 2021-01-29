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

#pragma once

#include <inttypes.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <float.h>
#include <stdlib.h>
#include <vector>
#include <atomic>
#include <thread>

#include <IConsole.h>
#include <STMSensorType.h>
#include "temp_struct_porting.h"
#include "common_data.h"
#include <CircularBuffer.h>
#include <FlushBufferStack.h>
#include <FlushRequested.h>
#include <ChangeODRTimestampStack.h>

#ifdef CONFIG_ST_HAL_HAS_SELFTEST_FUNCTIONS
#include <SelfTest.h>
#endif /* CONFIG_ST_HAL_HAS_SELFTEST_FUNCTIONS */

namespace stm {
namespace core {

#define SENSOR_DATA_1AXIS                       (1)
#define SENSOR_DATA_3AXIS                       (3)
#define SENSOR_DATA_4AXIS                       (4)
#define SENSOR_DATA_4AXIS_ACCUR                 (5)

#define SENSOR_BASE_ANDROID_NAME_MAX            (40)

#define NS_TO_MS(x)                             (x / 1E6)
#define NS_TO_FREQUENCY(x)                      (1E9 / x)
#define FREQUENCY_TO_NS(x)                      (1E9 / x)
#define FREQUENCY_TO_US(x)                      (1E6 / x)


class SensorBase;

typedef enum DependencyID {
    SENSOR_DEPENDENCY_ID_0 = 0,
    SENSOR_DEPENDENCY_ID_1,
    SENSOR_DEPENDENCY_ID_2,
    SENSOR_DEPENDENCY_ID_3,
    SENSOR_DEPENDENCY_ID_4,
    SENSOR_DEPENDENCY_ID_5,
    SENSOR_DEPENDENCY_ID_MAX
} DependencyID;

typedef struct push_data {
    bool is_trigger;
    unsigned int num;
    SensorBase *sb[SENSOR_DEPENDENCY_ID_MAX];
} push_data_t;

typedef struct dependencies {
    unsigned int num;
    SensorBase *sb[SENSOR_DEPENDENCY_ID_MAX];
} dependencies_t;

typedef enum InjectionModeID {
    SENSOR_INJECTION_NONE = 0,
    SENSOR_INJECTOR,
    SENSOR_INJECTED,
} InjectionModeID;

/*
 * class SensorBase
 */
class SensorBase {
private:
    bool valid_class;

    int64_t enabled_sensors_mask;

    ChangeODRTimestampStack odr_stack;
    DependencyID handle_remapping_ID[ST_HAL_IIO_MAX_DEVICES];

    int AddSensorToDataPush(SensorBase *t);
    void RemoveSensorToDataPush(SensorBase *t);

    void SetDependencyIDOfHandle(int handle, DependencyID id);

protected:
    char android_name[SENSOR_BASE_ANDROID_NAME_MAX];

    int write_pipe_fd, read_pipe_fd;
    std::vector<STMSensorType> dependencies_type_list;

    pthread_mutex_t sample_in_processing_mutex;
    volatile int64_t sample_in_processing_timestamp;

    InjectionModeID injection_mode;

    int64_t current_real_pollrate;
    int64_t current_min_pollrate;
    int64_t current_min_timeout;
    int64_t last_data_timestamp;
    int64_t sensors_timeout[ST_HAL_IIO_MAX_DEVICES];
    int64_t sensors_pollrates[ST_HAL_IIO_MAX_DEVICES];
    volatile int64_t sensor_global_enable;
    volatile int64_t sensor_global_disable;
    volatile int64_t sensor_my_enable;
    volatile int64_t sensor_my_disable;
    uint8_t decimator;
    uint8_t samples_counter;

    push_data_t push_data;
    dependencies_t dependencies;

    pthread_mutex_t enable_mutex;

    FlushBufferStack flush_stack;

    sensors_event_t sensor_event;
    struct sensor_t sensor_t_data;

    CircularBuffer *circular_buffer_data[SENSOR_DEPENDENCY_ID_MAX];

    std::unique_ptr<std::thread> dataThread;
    std::unique_ptr<std::thread> eventsThread;

    void InvalidThisClass();
    bool GetStatusExcludeHandle(int handle);
    bool GetStatusOfHandle(int handle);
    bool GetStatusOfHandle(int handle, bool lock_en_mutex);
    int64_t GetMinTimeout(bool lock_en_mutex);
    int64_t GetMinPeriod(bool lock_en_mutex);
    DependencyID GetDependencyIDFromHandle(int handle);

    int AllocateBufferForDependencyData(DependencyID id, unsigned int max_fifo_len);
    void DeAllocateBufferForDependencyData(DependencyID id);

    void SetBitEnableMask(int handle);
    void ResetBitEnableMask(int handle);

    int AddNewPollrate(int64_t timestamp, int64_t pollrate);
    int CheckLatestNewPollrate(int64_t *timestamp, int64_t *pollrate);
    void DeleteLatestNewPollrate();

public:
    SensorBase(const char *name, int handle, const STMSensorType &type);
    virtual ~SensorBase();

    bool IsValidClass();

    virtual int CustomInit();

    STMSensorType GetType();
    char* GetName();
    int GetHandle();
    int GetFdPipeToRead();
    int GetMaxFifoLenght();
    struct sensor_t GetSensor_tData(void);
    const std::vector<STMSensorType>& GetDepenciesTypeList(void) const;
    virtual bool ValidDataToPush(int64_t timestamp);
    bool GetDependencyMaxRange(STMSensorType type, float *maxRange);

    virtual int AddSensorDependency(SensorBase *p);
    virtual void RemoveSensorDependency(SensorBase *p);

    virtual int startThreads(void);
    virtual void stopThreads(void);

#ifdef CONFIG_ST_HAL_HAS_SELFTEST_FUNCTIONS
    virtual selftest_status ExecuteSelfTest();
#endif /* CONFIG_ST_HAL_HAS_SELFTEST_FUNCTIONS */

    virtual int Enable(int handle, bool enable, bool lock_en_mutex);
    bool GetStatus(bool lock_en_mutex);
    void SetEnableTimestamp(int handle, bool enable, int64_t timestamp);

    virtual int SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mutex);

    virtual int FlushData(int handle, bool lock_en_mute);
    virtual void ProcessFlushData(int handle, int64_t timestamp);

    void WriteOdrChangeEventToPipe(int64_t timestamp, int64_t pollrate);
    void WriteFlushEventToPipe();
    virtual void WriteDataToPipe(int64_t hw_pollrate);

#if (CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED)
    void WriteSensorAdditionalInfoFrameToPipe(additional_info_event_t *p_additional_info_event);
#endif /* CONFIG_ST_HAL_ADDITIONAL_INFO_ENABLED */

    virtual void ProcessData(SensorBaseData *data);
    virtual void ReceiveDataFromDependency(int handle, SensorBaseData *data);
    virtual int GetLatestValidDataFromDependency(int dependency_id, SensorBaseData *data, int64_t timesync);

    static void *ThreadDataWork(void *context);
    virtual void ThreadDataTask();

    static void *ThreadEventsWork(void *context);
    virtual void ThreadEventsTask();

    virtual int InjectionMode(bool enable);
    virtual int InjectSensorData(const sensors_event_t *data);

    virtual bool hasEventChannels();
    virtual bool hasDataChannels();
};

} // namespace core
} //namespace stm