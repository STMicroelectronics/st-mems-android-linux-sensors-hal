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

#include <queue>
#include <poll.h>
#include <mutex>

#include "SensorBase.h"
#include <IUtils.h>
#include <IConsole.h>
#include <STMTimesync.h>
#include <PropertiesManager.h>

extern "C" {
    #include "utils.h"
}

namespace stm {
namespace core {

#define HW_SENSOR_BASE_DEFAULT_IIO_BUFFER_LEN    (2)
#define HW_SENSOR_BASE_IIO_SYSFS_PATH_MAX        (200)
#define HW_SENSOR_BASE_IIO_DEVICE_NAME_MAX       (30)
#define HW_SENSOR_BASE_MAX_CHANNELS              (8)

struct HWSensorBaseCommonData {
    char device_iio_sysfs_path[HW_SENSOR_BASE_IIO_SYSFS_PATH_MAX];
    char device_name[HW_SENSOR_BASE_IIO_DEVICE_NAME_MAX];
    unsigned int device_iio_dev_num;

    int num_channels;
    struct device_iio_info_channel channels[HW_SENSOR_BASE_MAX_CHANNELS];

    struct device_iio_scales sa;
} typedef HWSensorBaseCommonData;

struct selftest_data {
    unsigned int available;
    char mode[5][20];
};

class HWSensorBase;
class HWSensorBaseWithPollrate;

/*
 * class HWSensorBase
 */
class HWSensorBase : public SensorBase {
protected:
    ssize_t scan_size;
    struct pollfd pollfd_iio[2];
    std::queue<int> flushRequested;
    std::mutex flushRequesteLock;
    HWSensorBaseCommonData common_data;
    ChangeODRTimestampStack odr_switch;

    struct selftest_data selftest;
    uint8_t *injection_data;
    bool has_event_channels;

    int WriteBufferLenght(unsigned int buf_len);

    IUtils &utils { IUtils::getInstance() };
    PropertiesManager& propertiesManager { PropertiesManager::getInstance() };

    std::mutex timesyncLock;
    STMTimesync timesync;

    struct syncEventHolder {
        uint32_t val;
        int64_t timestamp;
    } lastMSB, lastLSB;

public:
    HWSensorBase(HWSensorBaseCommonData *data,
                 const char *name,
                 int handle, const STMSensorType &sensor_type,
                 unsigned int hw_fifo_len,
                 float power_consumption,
                 int module);
    virtual ~HWSensorBase();

    virtual selftest_status ExecuteSelfTest() override;
    void GetSelfTestAvailable();

    virtual int Enable(int handle, bool enable, bool lock_en_mute) override;
    virtual int SetDelay(int handle, int64_t period_ns, int64_t timeout, bool lock_en_mute) override;

    virtual int AddSensorDependency(SensorBase *p) override;
    virtual void RemoveSensorDependency(SensorBase *p) override;

    virtual void ProcessData(SensorBaseData *data) override;
    virtual void ProcessEvent(struct device_iio_events *event_data);
    virtual int flushRequest(int handle, bool lock_en_mute) override;
    virtual void ProcessFlushData(int handle, int64_t timestamp) override;
    void processSyncEvent(struct device_iio_events *event_data);
    virtual void ThreadDataTask(std::atomic<bool>& threadsRunning) override;
    virtual void ThreadEventsTask(std::atomic<bool>& threadsRunning) override;

    virtual int InjectionMode(bool enable) override;
    virtual int InjectSensorData(const sensors_event_t *data) override;

    bool hasEventChannels() override { return has_event_channels; }
    bool hasDataChannels() override { return common_data.num_channels > 0; }
};

/*
 * class HWSensorBaseWithPollrate
 */
class HWSensorBaseWithPollrate : public HWSensorBase {
private:
    struct device_iio_sampling_freqs sampling_frequency_available;
    bool xSupported;
    bool ySupported;
    bool zSupported;
    float hw_sampling_frequency;

public:
    HWSensorBaseWithPollrate(HWSensorBaseCommonData *data, const char *name,
                             struct device_iio_sampling_freqs *sfa, int handle,
                             const STMSensorType &sensor_type,
                             unsigned int hw_fifo_len, float power_consumption,
                             int module);
    HWSensorBaseWithPollrate(HWSensorBaseCommonData *data, const char *name,
                             struct device_iio_sampling_freqs *sfa, int handle,
                             const STMSensorType &sensor_type,
                             unsigned int hw_fifo_len, float power_consumption,
                             int module,
                             bool x_is_supp, bool y_is_supp, bool z_is_supp);
    virtual ~HWSensorBaseWithPollrate();

    virtual int SetDelay(int handle, int64_t period_ns, int64_t timeout,
                         bool lock_en_mute) override;
    virtual int flushRequest(int handle, bool lock_en_mute) override;
    virtual void WriteDataToPipe(int64_t hw_pollrate) override;
    bool isXSupported(void);
    bool isYSupported(void);
    bool isZSupported(void);
    int copyAxesData(std::array<float, 3> &axesdata, SensorBaseData *data);
    virtual int getSupportedAxes(void);
    float getHWSamplingRate(void);
};

} // namespace core
} // namespace stm
