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

#include <string.h>
#include <poll.h>

#include "SensorBase.h"
#include "IUtils.h"

namespace stm {
namespace core {

#define ST_SENSOR_FUSION_RESOLUTION(maxRange)           (maxRange / (1 << 24))
#define ST_SW_SENSOR_BASE_MAX_FLUSH_EVENTS              (10)

class SWSensorBase;

/*
 * class SWSensorBase
 */
class SWSensorBase : public SensorBase {
protected:
    bool dependency_resolution;
    bool dependency_range;
    bool dependency_delay;
    bool dependency_name;
    int triggerHandle;

    DependencyID id_sensor_trigger;
    int trigger_write_pipe_fd, trigger_read_pipe_fd;
    struct pollfd android_pollfd;

    SensorBaseData *sensors_tmp_data;
    IUtils &utils { IUtils::getInstance() };

    virtual bool ValidDataToPush(int64_t timestamp) override;

public:
SWSensorBase(const char *name, int handle, STMSensorType sensor_type,
             bool use_dependency_resolution, bool use_dependency_range,
             bool use_dependency_delay, bool use_dependency_name, int module);
    virtual ~SWSensorBase();

    virtual int Enable(int handle, bool enable, bool lock_en_mutex) override;

    virtual int AddSensorDependency(SensorBase *p) override;
    virtual void RemoveSensorDependency(SensorBase *p) override;

    virtual void ReceiveDataFromDependency(int handle, SensorBaseData *data) override;

    virtual int flushRequest(int handle, bool lock_en_mutex) override;
    virtual void ProcessFlushData(int handle, int64_t timestamp) override;

    virtual void ThreadDataTask(std::atomic<bool>& threadsRunning) override;

    bool hasDataChannels() override { return true; }

    int getHandleOfMyTrigger(void) const override;
};


/*
 * class SWSensorBaseWithPollrate
 */
class SWSensorBaseWithPollrate : public SWSensorBase {
public:
    SWSensorBaseWithPollrate(const char *name, int handle, STMSensorType sensor_type,
                             bool use_dependency_resolution, bool use_dependency_range,
                             bool use_dependency_delay, bool use_dependency_name, int module);
    virtual ~SWSensorBaseWithPollrate();

    virtual int SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mutex) override;
    virtual int flushRequest(int handle, bool lock_en_mutex) override;
    virtual void WriteDataToPipe(int64_t hw_pollrate) override;
};

} // namespace core
} // namespace stm
