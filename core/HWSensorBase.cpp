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

#include <endian.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <climits>

#include "HWSensorBase.h"

#include <PropertiesManager.h>

namespace stm {
namespace core {

#define HW_SENSOR_BASE_DEELAY_TRANSFER_DATA            (500000000LL)

/**
 * size_from_channelarray() - Calculate the storage size of a scan
 * @channels: the channel info array.
 * @num_channels: number of channels.
 **/
static int size_from_channelarray(struct device_iio_info_channel *channels,
                                  int num_channels)
{
    int bytes = 0, i;

    for (i = 0; i < num_channels; i++) {
        channels[i].location = 0;

        if (channels[i].bytes == 0) {
            continue;
        }

        if (bytes % channels[i].bytes == 0) {
            channels[i].location = bytes;
        } else {
            channels[i].location = bytes - (bytes % channels[i].bytes) + channels[i].bytes;
        }

        bytes = channels[i].location + channels[i].bytes;
    }

    return bytes;
}

/**
 * process_2byte_received() - Return channel data from 2 byte
 * @input: 2 byte of data received from buffer channel.
 * @info: information about channel structure.
 **/
static float process_2byte_received(int input, struct device_iio_info_channel *info)
{
    float res;
    int16_t val;

    if (info->be) {
        input = be16toh((uint16_t)input);
    } else {
        input = le16toh((uint16_t)input);
    }

    val = input >> info->shift;

    if (info->sign) {
        val &= (1 << info->bits_used) - 1;
        val = (int16_t)(val << (16 - info->bits_used)) >> (16 - info->bits_used);
        res = (float)val;
    } else {
        val &= (1 << info->bits_used) - 1;
        res = (float)((uint16_t)val);
    }

    return ((res + info->offset) * info->scale);
}

/**
 * process_3byte_received() - Return channel data from 3 byte
 * @input: 3 byte of data received from buffer channel.
 * @info: information about channel structure.
 **/
static float process_3byte_received(int input, struct device_iio_info_channel *info)
{
    float res;
    int32_t val;

    if (info->be) {
        input = be32toh((uint32_t)input);
    } else {
        input = le32toh((uint32_t)input);
    }

    val = input >> info->shift;
    if (info->sign) {
        val &= (1 << info->bits_used) - 1;
        val = (int32_t)(val << (24 - info->bits_used)) >> (24 - info->bits_used);
        res = (float)val;
    } else {
        val &= (1 << info->bits_used) - 1;
        res = (float)((uint32_t)val);
    }

    return ((res + info->offset) * info->scale);
}

/**
 * process_scan() - This functions use channels device information to build data
 * @hw_sensor: pointer to current hardware sensor.
 * @data: sensor data of all channels read from buffer.
 * @channels: information about channel structure.
 * @num_channels: number of channels of the sensor.
 **/
static int ProcessScanData(uint8_t *data, struct device_iio_info_channel *channels,
                           int num_channels,
                           SensorBaseData *sensor_out_data)
{
    int k;

    for (k = 0; k < num_channels; k++) {
        sensor_out_data->offset[k] = 0;

        switch (channels[k].bytes) {
        case 1:
            sensor_out_data->raw[k] = *(uint8_t *)(data + channels[k].location);
            break;
        case 2:
            sensor_out_data->raw[k] = process_2byte_received(*(uint16_t *)
                                                             (data + channels[k].location), &channels[k]);
            break;
        case 3:
            sensor_out_data->raw[k] = process_3byte_received(*(uint32_t *)
                                                             (data + channels[k].location), &channels[k]);
            break;
        case 4:
            uint32_t val;

            if (channels[k].be) {
                val = be32toh(*(uint32_t *)(data + channels[k].location));
            } else {
                val = le32toh(*(uint32_t *)(data + channels[k].location));
            }

            val >>= channels[k].shift;
            val &= channels[k].mask;

            if (channels[k].sign) {
                sensor_out_data->raw[k] = ((float)(int32_t)val + channels[k].offset) * channels[k].scale;
            } else {
                sensor_out_data->raw[k] = ((float)val + channels[k].offset) * channels[k].scale;
            }

            break;
        case 8:
            if (channels[k].sign) {
                int64_t val = *(int64_t *)(data + channels[k].location);
                if ((val >> channels[k].bits_used) & 1) {
                    val = (val & channels[k].mask) | ~channels[k].mask;
                }

                if (channels[k].type == IIOChannelType::TIMESTAMP) {
                    sensor_out_data->timestamp = val;
                } else if (channels[k].type == IIOChannelType::HW_TIMESTAMP) {
                    sensor_out_data->hwTimestamp = val;
                    sensor_out_data->hasHwTimestamp = true;
                } else {
                    IConsole &console = IConsole::getInstance();
                    console.warning("cannot process 64bit channel");
                }
            }

            break;
        default:
            return -EINVAL;
        }
    }

    return num_channels;
}

static int ProcessInjectionData(float *data,
                                struct device_iio_info_channel *channels,
                                int num_channels,
                                uint8_t *out_data,
                                int64_t timestamp)
{
    int k;

    for (k = 0; k < num_channels; k++) {
        switch (channels[k].bytes) {
        case 1:
            *(uint8_t *)(out_data + channels[k].location) = data[k];
            break;

        case 2:
            *(uint16_t *)(out_data + channels[k].location) = (int16_t)(data[k] / channels[k].scale);
            break;

        case 8:
            *(int64_t *)(out_data + channels[k].location) = timestamp;
            break;

        default:
            return -EINVAL;
        }
    }

    return num_channels;
}

HWSensorBase::HWSensorBase(HWSensorBaseCommonData *data, const char *name,
                           int handle, const STMSensorType &sensor_type, unsigned int hw_fifo_len,
                           float power_consumption)
    : SensorBase(name, handle, sensor_type)
{
    int err;
    char *buffer_path;

    if (HAL_ENABLE_TIMESYNC != 0) {
        timesync.init(32);
    }
    lastLSB.timestamp = 0;
    lastMSB.timestamp = 0;

    memcpy(&common_data, data, sizeof(common_data));

    sensor_t_data.power = power_consumption;
    sensor_t_data.fifoMaxEventCount = hw_fifo_len;

    selftest.available = 0;

    scan_size = size_from_channelarray(common_data.channels, common_data.num_channels);

    err = asprintf(&buffer_path, "/dev/iio:device%d", data->device_iio_dev_num);
    if (err <= 0) {
        console.error(GetName() + std::string(": Failed to allocate iio device path string."));
        goto invalid_this_class;
    }

    pollfd_iio[0].fd = open(buffer_path, O_RDONLY | O_NONBLOCK);
    if (pollfd_iio[0].fd < 0) {
        console.error(GetName() + std::string(": Failed to open iio char device (") + buffer_path + ").");
        goto free_buffer_path;
    }

    pollfd_iio[0].events = POLLIN;

    if (!ioctl(pollfd_iio[0].fd, _IOR('i', 0x90, int), &pollfd_iio[1].fd)) {
        pollfd_iio[1].events = POLLIN;
        has_event_channels = true;
    } else {
        has_event_channels= false;
    }

    err = device_iio_utils::support_injection_mode(common_data.device_iio_sysfs_path);
    switch (err) {
    case 0:
        console.debug(GetName() + std::string(": injection mode available, sensor is an injector"));
        //sensor_t_data.flags |= DATA_INJECTION_MASK;
        injection_mode = SENSOR_INJECTOR;
        break;

    case 1:
        console.debug(GetName() + std::string(": injection mode available, sensor is injected"));
        //sensor_t_data.flags |= DATA_INJECTION_MASK;
        injection_mode = SENSOR_INJECTED;
        break;

    default:
        console.debug(GetName() + std::string(": injection mode not available"));
        //sensor_t_data.flags &= ~DATA_INJECTION_MASK;
        break;
    }

    free(buffer_path);

    return;

free_buffer_path:
    free(buffer_path);
invalid_this_class:
    InvalidThisClass();
}

HWSensorBase::~HWSensorBase()
{
    if (!IsValidClass()) {
        return;
    }

    close(pollfd_iio[0].fd);
    close(pollfd_iio[1].fd);
}

void HWSensorBase::GetSelfTestAvailable()
{
    int err;

    err = device_iio_utils::get_selftest_available(common_data.device_iio_sysfs_path, selftest.mode);
    if (err < 0) {
        return;
    }

    selftest.available = err;
}

selftest_status HWSensorBase::ExecuteSelfTest()
{
    int status;

    if (selftest.available == 0) {
        return NOT_AVAILABLE;
    }

    status = device_iio_utils::execute_selftest(common_data.device_iio_sysfs_path,
                                                &selftest.mode[0][0]);
    if (status < 0) {
        console.error(GetName() + std::string("failed to execute selftest procedure (errorno: ") +
                      std::to_string(status) + std::string(")"));
        return GENERIC_ERROR;
    }

    if (status == 0) {
        return FAILURE;
    }

    return PASS;
}

int HWSensorBase::WriteBufferLenght(unsigned int buf_len)
{
    int err;
    unsigned int hw_buf_fifo_len;

    if (buf_len == 0) {
        hw_buf_fifo_len = 1;
    } else {
        hw_buf_fifo_len = buf_len;
    }

    err = device_iio_utils::set_hw_fifo_watermark(common_data.device_iio_sysfs_path,
                                                  hw_buf_fifo_len);
    if (err < 0) {
        console.error(GetName() + std::string(": Failed to write hw fifo watermark."));
        return err;
    }

    return 0;
}

int HWSensorBase::Enable(int handle, bool enable, bool lock_en_mutex)
{
    int err = 0;
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
        err = device_iio_utils::enable_sensor(common_data.device_iio_sysfs_path,
                                              GetStatus(false));
        if (err < 0) {
            console.error(GetName() + std::string(": Failed to enable iio sensor device."));
            goto restore_status_enable;
        }

        if (enable) {
            if (HAL_ENABLE_TIMESYNC != 0) {
                std::lock_guard<std::mutex> lock(timesyncLock);
                timesync.reset();
            }
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

restore_status_enable:
    SensorBase::Enable(handle, !enable, false);
unlock_mutex:
    if (lock_en_mutex) {
        pthread_mutex_unlock(&enable_mutex);
    }

    return err;
}

int HWSensorBase::SetDelay(int __attribute__((unused))handle,
                           int64_t __attribute__((unused))period_ns,
                           int64_t timeout, bool lock_en_mutex)
{
    unsigned int buf_len;
    int err;

    if (timeout < INT64_MAX) {
        if ((sensor_t_data.fifoMaxEventCount == 0) && (timeout > 0)) {
            return -EINVAL;
        }
    } else {
        return 0;
    }

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    if (sensor_t_data.fifoMaxEventCount > 0) {
        buf_len = timeout / FREQUENCY_TO_NS(1);
        if (buf_len > sensor_t_data.fifoMaxEventCount) {
            buf_len = sensor_t_data.fifoMaxEventCount;
        }

        err = WriteBufferLenght(buf_len);
        if (err < 0) {
            goto mutex_unlock;
        }
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

int HWSensorBase::AddSensorDependency(SensorBase *p)
{
    int dependency_id, err;

    dependency_id = SensorBase::AddSensorDependency(p);
    if (dependency_id < 0) {
        return dependency_id;
    }

    err = AllocateBufferForDependencyData((DependencyID)dependency_id,
                                          p->GetMaxFifoLenght());
    if (err < 0) {
        return err;
    }

    return 0;
}

void HWSensorBase::RemoveSensorDependency(SensorBase *p)
{
    DeAllocateBufferForDependencyData(GetDependencyIDFromHandle(p->GetHandle()));
    SensorBase::RemoveSensorDependency(p);
}

void HWSensorBase::ProcessEvent(struct device_iio_events *event_data)
{
    uint8_t event_type, event_dir;

    event_type = ((event_data->event_id >> 56) & 0xFF);
    event_dir = ((event_data->event_id >> 48) & 0x7F);

    if ((event_type == DEVICE_IIO_EV_TYPE_FIFO_FLUSH) ||
        (event_dir == DEVICE_IIO_EV_DIR_FIFO_DATA)) {
        ProcessFlushData(sensor_t_data.handle, event_data->event_timestamp);
    } else if (event_type == DEVICE_IIO_EV_TYPE_TIME_SYNC) {
        if (HAL_ENABLE_TIMESYNC != 0) {
            processSyncEvent(event_data);
        } else {
            console.warning("received timesync event but feature not enabled!");
        }
    }
}

void HWSensorBase::processSyncEvent(struct device_iio_events *event_data)
{
    uint8_t event_dir = ((event_data->event_id >> 48) & 0x7F);

    if (event_dir == 0) {
        lastLSB.timestamp = event_data->event_timestamp;
        lastLSB.val = le32toh(event_data->event_id & 0xFFFFFFFF);
    } else {
        lastMSB.timestamp = event_data->event_timestamp;
        lastMSB.val = le32toh(event_data->event_id & 0xFFFFFFFF);
    }

    if (lastLSB.timestamp == lastMSB.timestamp) {
        int64_t hwTimestamp = ((uint64_t)lastMSB.val << 32) | (uint64_t)lastLSB.val;

        std::lock_guard<std::mutex> lock(timesyncLock);
        timesync.add(event_data->event_timestamp, hwTimestamp);
    }
}

void HWSensorBase::ProcessData(SensorBaseData *data)
{
    SensorBase::ProcessData(data);
}

int HWSensorBase::flushRequest(int handle, bool lock_en_mutex)
{
    int err;
    unsigned int i;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    if (GetStatus(false)) {
        {
            std::lock_guard<std::mutex> lock(flushRequesteLock);
            flushRequested.push(handle);
        }

        for (i = 0; i < dependencies.num; i++) {
            dependencies.sb[i]->flushRequest(sensor_t_data.handle, true);
        }

        if (sensor_t_data.fifoMaxEventCount) {
            err = device_iio_utils::hw_fifo_flush(common_data.device_iio_sysfs_path);
            if (err < 0) {
                // TODO should remove the flush requests just added
                console.error(GetName() + std::string(": Failed to flush hw fifo."));
                goto unlock_mutex;
            }
        } else {
            ProcessFlushData(sensor_t_data.handle, 0);
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

void HWSensorBase::ProcessFlushData(int __attribute__((unused))handle, int64_t timestamp)
{
    int err, flush_handle;

    std::lock_guard<std::mutex> lock(flushRequesteLock);
    if (flushRequested.empty()) {
        console.debug(GetName() + std::string(": no flush requests were made"));
        return;
    }
    flush_handle = flushRequested.front();
    flushRequested.pop();

    pthread_mutex_lock(&sample_in_processing_mutex);

    if (flush_handle == sensor_t_data.handle) {
        if (timestamp > sample_in_processing_timestamp) {
            err = flush_stack.writeElement(flush_handle, timestamp);
            if (err < 0) {
                console.error(GetName() + std::string(": Failed to write Flush event into stack."));
            }
        } else {
            WriteFlushEventToPipe();
        }
    } else {
        for (auto i = 0U; i < push_data.num; i++) {
            if (sensor_t_data.handle == push_data.sb[i]->getHandleOfMyTrigger()) {
                push_data.sb[i]->ProcessFlushData(flush_handle, timestamp);
            }
        }
    }

    pthread_mutex_unlock(&sample_in_processing_mutex);
}

void HWSensorBase::ThreadDataTask(std::atomic<bool>& threadsRunning)
{
    uint8_t *data;
    unsigned int hw_fifo_len;
    SensorBaseData sensor_data;
    int err, i, read_size, flush_handle;
    int64_t timestamp_flush, timestamp_odr_switch, new_pollrate = 0;
    int64_t old_pollrate = 0;

    if (sensor_t_data.fifoMaxEventCount > 0) {
        hw_fifo_len = sensor_t_data.fifoMaxEventCount;
    } else {
        hw_fifo_len = 1;
    }

    data = (uint8_t *)malloc(hw_fifo_len * scan_size * HW_SENSOR_BASE_DEFAULT_IIO_BUFFER_LEN * sizeof(uint8_t));
    if (!data) {
        console.error(GetName() + std::string(": Failed to allocate sensor data buffer (" +
                                              std::to_string(hw_fifo_len) + " " +
                                              std::to_string(scan_size) + ")."));
        return;
    }

    while (threadsRunning.load()) {
        err = poll(&pollfd_iio[0], 1, 200);
        if (err <= 0) {
            continue;
        }

        if (pollfd_iio[0].revents & POLLIN) {
            read_size = read(pollfd_iio[0].fd, data, hw_fifo_len * scan_size * HW_SENSOR_BASE_DEFAULT_IIO_BUFFER_LEN);
            if (read_size <= 0) {
                console.error(GetName() + std::string(": Failed to read data from iio char device."));
                continue;
            }

            for (i = 0; i < (read_size / scan_size); i++) {
                err = ProcessScanData(data + (i * scan_size), common_data.channels, common_data.num_channels, &sensor_data);
                if (err < 0) {
                    continue;
                }

                if ((HAL_ENABLE_TIMESYNC != 0) && (sensor_data.hasHwTimestamp)) {
                    std::lock_guard<std::mutex> lock(timesyncLock);
                    if (!timesync.estimate(sensor_data.hwTimestamp, sensor_data.timestamp)) {
                        sensor_data.timestamp = 0;
                    } else {
                        int64_t now = utils.getTime();
                        if (sensor_data.timestamp > utils.getTime()) {
                            sensor_data.timestamp = now;
                        }
                    }

                    pthread_mutex_lock(&sample_in_processing_mutex);
                    sample_in_processing_timestamp = sensor_data.hwTimestamp;
                    pthread_mutex_unlock(&sample_in_processing_mutex);
                } else {
                    pthread_mutex_lock(&sample_in_processing_mutex);
                    sample_in_processing_timestamp = sensor_data.timestamp;
                    pthread_mutex_unlock(&sample_in_processing_mutex);
                }

                timestamp_odr_switch = odr_switch.readLastElement(&new_pollrate);
                if ((timestamp_odr_switch >= 0) && (sensor_data.timestamp > timestamp_odr_switch)) {
                    sensor_data.pollrate_ns = new_pollrate;
                    old_pollrate = new_pollrate;
                    odr_switch.removeLastElement();
                } else {
                    sensor_data.pollrate_ns = old_pollrate;
                }

                sensor_data.flushEventsNum = 0;
                bool tryAgain = false;

                do {
                    flush_handle = flush_stack.readLastElement(&timestamp_flush);
                    int64_t timestampToCompare = sensor_data.timestamp;
                    if ((HAL_ENABLE_TIMESYNC != 0) && sensor_data.hasHwTimestamp) {
                        timestampToCompare = sensor_data.hwTimestamp;
                    }
                    if ((flush_handle >= 0) && (timestamp_flush <= timestampToCompare)) {
                        if (sensor_data.flushEventsNum < (int)sensor_data.flushEventHandles.size()) {
                            sensor_data.flushEventHandles[sensor_data.flushEventsNum++] = flush_handle;
                        }
                        flush_stack.removeLastElement();
                        tryAgain = true;
                    } else {
                        tryAgain = false;
                    }
                } while (tryAgain);

                if (sensor_data.timestamp) {
                    ProcessData(&sensor_data);
                } else {
                    for (int i = 0; i < sensor_data.flushEventsNum; ++i) {
                        if (sensor_data.flushEventHandles[i] == sensor_t_data.handle) {
                                WriteFlushEventToPipe();
                        } else {
                            for (auto j = 0U; j < push_data.num; j++) {
                                if (sensor_t_data.handle == push_data.sb[j]->getHandleOfMyTrigger()) {
                                    push_data.sb[j]->ProcessFlushData(sensor_data.flushEventHandles[i], 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void HWSensorBase::ThreadEventsTask(std::atomic<bool>& threadsRunning)
{
    struct device_iio_events event_data[10];
    int err, i, read_size;

    while (threadsRunning.load()) {
        err = poll(&pollfd_iio[1], 1, 200);
        if (err <= 0) {
            continue;
        }

        if (pollfd_iio[1].revents & POLLIN) {
            read_size = read(pollfd_iio[1].fd, event_data, 10 * sizeof(struct device_iio_events));
            if (read_size <= 0) {
                console.error(GetName() + std::string(": Failed to read event data from iio char device."));
                continue;
            }

            for (i = 0; i < (int)(read_size / sizeof(struct device_iio_events)); i++) {
                ProcessEvent(&event_data[i]);
            }
        }
    }
}

int HWSensorBase::InjectionMode(bool enable)
{
    int err;

    switch (injection_mode) {
    case SENSOR_INJECTION_NONE:
        break;

    case SENSOR_INJECTOR:
        if (enable) {
            injection_data = (uint8_t *)malloc(sizeof(uint8_t) * scan_size);
            if (!injection_data) {
                return -ENOMEM;
            }
        } else {
            free(injection_data);
        }

        err = device_iio_utils::set_injection_mode(common_data.device_iio_sysfs_path, enable);
        if (err < 0) {
            console.error(GetName() + std::string(": Failed to switch injection mode."));
            free(injection_data);
            return err;
        }
        break;

    case SENSOR_INJECTED:
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

int HWSensorBase::InjectSensorData(const sensors_event_t *data)
{
    int err;
    device_iio_chan_type_t device_iio_sensor_type;

    err = ProcessInjectionData((float *)data->data.data2, common_data.channels,
                               common_data.num_channels, injection_data,
                               data->timestamp);
    if (err < 0) {
        return err;
    }

    if (sensor_t_data.type == AccelSensorType) {
        device_iio_sensor_type = DEVICE_IIO_ACC;
    } else if (sensor_t_data.type == MagnSensorType) {
        device_iio_sensor_type = DEVICE_IIO_MAGN;
    } else if (sensor_t_data.type == GyroSensorType) {
        device_iio_sensor_type = DEVICE_IIO_GYRO;
    } else {
        return -EINVAL;
    }

    return device_iio_utils::inject_data(common_data.device_iio_sysfs_path,
                                         injection_data,
                                         scan_size,
                                         device_iio_sensor_type);
}

HWSensorBaseWithPollrate::HWSensorBaseWithPollrate(HWSensorBaseCommonData *data,
                                                   const char *name, struct device_iio_sampling_freqs *sfa,
                                                   int handle, const STMSensorType &sensor_type, unsigned int hw_fifo_len,
                                                   float power_consumption)
    : HWSensorBase(data, name, handle, sensor_type, hw_fifo_len, power_consumption)
{
    float min_sampling_frequency = UINT_MAX, max_sampling_frequency = 0;
    unsigned int i;

    memcpy(&sampling_frequency_available, sfa, sizeof(sampling_frequency_available));

    PropertiesManager& properties = PropertiesManager::getInstance();
    int maxOdr = properties.getMaxOdr();

    for (i = 0; i < sfa->length; i++) {
        if ((max_sampling_frequency < sfa->freq[i]) && (sfa->freq[i] <= maxOdr)) {
            max_sampling_frequency = sfa->freq[i];
        }

        if (min_sampling_frequency > sfa->freq[i]) {
            min_sampling_frequency = sfa->freq[i];
        }
    }

    sensor_t_data.maxRateHz = max_sampling_frequency;
    sensor_t_data.minRateHz = min_sampling_frequency;
}

HWSensorBaseWithPollrate::~HWSensorBaseWithPollrate()
{

}

int HWSensorBaseWithPollrate::SetDelay(int handle, int64_t period_ns,
                                       int64_t timeout, bool lock_en_mutex)
{
    int err, i;
    bool message = false;
    unsigned int sampling_frequency, buf_len;
    int64_t min_pollrate_ns, min_timeout_ns = 0, timestamp;

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

    sampling_frequency = NS_TO_FREQUENCY(min_pollrate_ns);
    for (i = 0; i < (int)sampling_frequency_available.length; i++) {
        if (sampling_frequency_available.freq[i] >= sampling_frequency) {
            break;
        }
    }
    if (i == (int)sampling_frequency_available.length) {
        i--;
    }

    if (current_min_pollrate != min_pollrate_ns) {
        err = device_iio_utils::set_sampling_frequency(common_data.device_iio_sysfs_path,
                                                       sampling_frequency_available.freq[i]);
        if (err < 0) {
            console.error(GetName() + std::string(": Failed to write sampling frequency to iio device."));
            goto mutex_unlock;
        }

        timestamp = utils.getTime();

        err = odr_switch.writeElement(timestamp, FREQUENCY_TO_NS(sampling_frequency_available.freq[i]));
        if (err < 0) {
            console.error(GetName() + std::string(": Failed to write new odr on stack."));
        }

        if (handle == sensor_t_data.handle) {
            AddNewPollrate(timestamp, period_ns);
        }

        current_min_pollrate = min_pollrate_ns;
        message = true;
    } else {
        if (handle == sensor_t_data.handle) {
            AddNewPollrate(utils.getTime(), period_ns);
        }
    }

    if (sensor_t_data.fifoMaxEventCount > 0) {
        min_timeout_ns = GetMinTimeout(false);
        if (min_timeout_ns < HW_SENSOR_BASE_DEELAY_TRANSFER_DATA) {
            min_timeout_ns = 0;
        }  else {
            min_timeout_ns -= HW_SENSOR_BASE_DEELAY_TRANSFER_DATA;
        }

        if (current_min_timeout != min_timeout_ns) {
            buf_len = min_timeout_ns / FREQUENCY_TO_NS(sampling_frequency_available.freq[i]);
            if (buf_len > sensor_t_data.fifoMaxEventCount) {
                buf_len = sensor_t_data.fifoMaxEventCount;
            }

            err = WriteBufferLenght(buf_len);
            if (err < 0) {
                goto mutex_unlock;
            }

            current_min_timeout = min_timeout_ns;
            message = true;
        }
    }

    if (message) {
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

int HWSensorBaseWithPollrate::flushRequest(int handle, bool lock_en_mutex)
{
    int err;
    unsigned int i;

    if (lock_en_mutex) {
        pthread_mutex_lock(&enable_mutex);
    }

    if (GetStatus(false)) {
        {
            std::lock_guard<std::mutex> lock(flushRequesteLock);
            flushRequested.push(handle);
        }

        for (i = 0; i < dependencies.num; i++) {
            dependencies.sb[i]->flushRequest(sensor_t_data.handle, true);
        }

        if (sensor_t_data.fifoMaxEventCount) {
            err = device_iio_utils::hw_fifo_flush(common_data.device_iio_sysfs_path);
            if (err < 0) {
                // TODO should remove the flush requests just added
                console.error(GetName() + std::string(": Failed to flush hw fifo."));
                goto unlock_mutex;
            }
        } else {
            ProcessFlushData(sensor_t_data.handle, 0);
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

void HWSensorBaseWithPollrate::WriteDataToPipe(int64_t hw_pollrate)
{
    int err;
    float temp;
    bool odr_changed = false;
    int64_t timestamp_change = 0, new_pollrate = 0;

    err = CheckLatestNewPollrate(&timestamp_change, &new_pollrate);
    if ((err >= 0) && (sensor_event.timestamp > timestamp_change)) {
        current_real_pollrate = new_pollrate;
        DeleteLatestNewPollrate();
        odr_changed = true;
    }

    if (ValidDataToPush(sensor_event.timestamp)) {
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
                console.error(GetName() + std::string(": Failed to write sensor data to pipe."));
                samples_counter--;
                return;
            }

            samples_counter = 0;
            last_data_timestamp = sensor_event.timestamp;
        }
    }
}

} // namespace core
} // namespace stm
