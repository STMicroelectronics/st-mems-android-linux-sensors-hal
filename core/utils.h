/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019-2020 STMicroelectronics
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

#include <stdint.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/types.h>

extern const char *device_iio_dir;

namespace stm {
namespace core {

#define DEVICE_IIO_MAX_FILENAME_LEN             512
#define DEVICE_IIO_MAX_NAME_LENGTH              16
#define DEVICE_IIO_MAX_SAMPLINGFREQ_LENGTH      32
#define DEVICE_IIO_MAX_SAMP_FREQ_AVAILABLE      10
#define DEVICE_IIO_SCALE_AVAILABLE              10

#include <local/sensorhal_iio_types.h>

#define DISABLE_CHANNEL                         0
#define ENABLE_CHANNEL                          1

struct device_iio_events {
    uint64_t event_id;
    int64_t event_timestamp;
};

struct device_iio_scales {
    float scales[DEVICE_IIO_SCALE_AVAILABLE] = {1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};
    unsigned int length;
};

struct device_iio_sampling_freqs {
    float freq[DEVICE_IIO_MAX_SAMP_FREQ_AVAILABLE];
    unsigned int length;
};

enum class IIOChannelType {
    TIMESTAMP,
    HW_TIMESTAMP,
    UNKNOWN
};

struct device_iio_info_channel {
    char *name;
    char *type_name;
    unsigned int index;
    unsigned int enabled;
    float scale = 1;
    float offset = 0;
    unsigned int bytes;
    unsigned int bits_used;
    unsigned int shift;
    unsigned long long int mask;
    unsigned int be;
    unsigned int sign;
    unsigned int location;
    IIOChannelType type;
};

struct device_iio_type_name {
    unsigned int num;
    char name[DEVICE_IIO_MAX_FILENAME_LEN];
};

class device_iio_utils {
private:
    static int sysfs_opendir(const char *name, DIR **dp);
    static int sysfs_write_int(char *file, int val);
    static int sysfs_write_uint(char *file, unsigned int val);
    static int sysfs_write_str(char *file, char *str);
    static int sysfs_write_float(char *file, float val);
    static int sysfs_read_int(char *file, int *val);
    static int sysfs_read_uint(char *file, unsigned int *val);
    static int sysfs_read_str(char *file, char *str, int len);
    static int sysfs_read_float(char *file, float *val);
    static int check_file(const char *filename);
    static int enable_events(const char *device_dir, bool enable);
    static int sysfs_enable_channels(const char *device_dir, bool enable);

public:
    static int get_device_by_name(const char *name);
    static int enable_sensor(const char *device_dir, bool enable);
    static int get_sampling_frequency_available(const char *device_dir, struct device_iio_sampling_freqs *sfa);
    static int get_hw_fifo_length(const char *device_dir);
    static int set_sampling_frequency(char *device_dir, unsigned int frequency);
    static int set_max_delivery_rate(const char *device_dir, unsigned int delay);
    static int set_hw_fifo_watermark(char *device_dir, unsigned int watermark);
    static int hw_fifo_flush(char *device_dir);
    static int set_scale(const char *device_dir, float value, device_iio_chan_type_t device_type);
    static int get_scale(const char *device_dir, float *value, device_iio_chan_type_t device_type);
    static int get_type(struct device_iio_info_channel *channel,
                        const char *device_dir,
                        const char *name,
                        const char *post);
    static int get_available_scales(const char *device_dir,
                                    struct device_iio_scales *sa,
                                    device_iio_chan_type_t device_type);
    static int get_devices_name(struct device_iio_type_name devices[],
                                unsigned int max_list);
    static int scan_channel(const char *device_dir,
                            struct device_iio_info_channel **ci_array,
                            int *counter);
    static int support_injection_mode(const char *device_dir);
    static int set_injection_mode(const char *device_dir, bool enable);
    static int inject_data(const char *device_dir, unsigned char *data,
                           int len, device_iio_chan_type_t device_type);
    static int set_clock_type(const char *device_dir, const char *type);
    static int get_selftest_available(const char *device_dir, char list[][20]);
    static int execute_selftest(const char *device_dir, char *mode);
    static int get_module_id(const char *device_dir);
};

} // namespace core
} // namespace stm
