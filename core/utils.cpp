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

#include <iostream>
#include <regex>
#include <cfloat>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <IConsole.h>
#include "utils.h"

/* IIO SYSFS interface */
const char *device_iio_dir = "/sys/bus/iio/devices/";

namespace stm {
namespace core {

#define SELFTEST_POSITIVE_RESULT                "pass"
#define SELFTEST_NEGATIVE_RESULT                "fail"

static const char *device_iio_sfa_filename = "sampling_frequency_available";
static const char *device_iio_sf_filename = "sampling_frequency";
static const char *device_iio_hw_fifo_enabled = "hwfifo_enabled";
static const char *device_iio_hw_fifo_length = "hwfifo_watermark_max";
static const char *device_iio_hw_fifo_watermark = "hwfifo_watermark";
static const char *device_iio_max_delivery_rate_filename = "max_delivery_rate";
static const char *device_iio_hw_fifo_flush = "hwfifo_flush";
static const char *device_iio_buffer_enable = "buffer/enable";
static const char *device_iio_event_dir = "%s/events";
static const char *device_iio_buffer_length = "buffer/length";
static const char *device_iio_device_name = "iio:device";
static const char *device_iio_injection_mode_enable = "injection_mode";
static const char *device_iio_injection_sensors_filename = "injection_sensors";
static const char *device_iio_current_timestamp_clock = "current_timestamp_clock";
static const char *device_iio_scan_elements_en = "_en";
static const char *device_iio_selftest_available_filename = "selftest_available";
static const char *device_iio_selftest_filename = "selftest";
static const char *device_iio_module_id_filename = "module_id";

static IConsole &console = IConsole::getInstance();

int device_iio_utils::sysfs_opendir(const char *name, DIR **dp)
{
    struct stat sb;
    DIR *tmp;

    /*
     * Check if path exists, if a component of path does not exist,
     * or path is an empty string return ENOENT
     * If path is not accessible return EACCES
     */
    if (stat(name, &sb) == -1)
        return -errno;

    /* Check if directory */
    if ((sb.st_mode & S_IFMT) != S_IFDIR)
        return -EINVAL;

    /* Open sysfs directory */
    tmp = opendir(name);
    if (tmp == NULL)
        return -errno;

    *dp = tmp;

    return 0;
}

int device_iio_utils::sysfs_write_int(char *file, int val)
{
    int ret = 0;
    FILE *fp;

    fp = fopen(file, "w");
    if (NULL == fp)
        return -errno;

    ret = fprintf(fp, "%d", val);
    fclose(fp);

    return ret < 0 ? ret : 0;
}

int device_iio_utils::sysfs_write_uint(char *file, unsigned int val)
{
    int ret = 0;
    FILE *fp;

    fp = fopen(file, "r+");
    if (NULL == fp)
        return -errno;

    ret = fprintf(fp, "%u", val);
    fclose(fp);

    return ret < 0 ? ret : 0;
}

int device_iio_utils::sysfs_write_float(char *file, float val)
{
    int ret = 0;
    FILE *fp;

    fp = fopen(file, "w");
    if (NULL == fp)
        return -errno;

    ret = fprintf(fp, "%.*f", DBL_DIG - 1, val);
    fclose(fp);

    return ret < 0 ? ret : 0;
}

int device_iio_utils::sysfs_write_str(char *file, char *str)
{
    int ret = 0;
    FILE *fp;

    fp = fopen(file, "w");
    if (NULL == fp)
        return -errno;

    ret = fprintf(fp, "%s", str);
    fclose(fp);

    return ret < 0 ? ret : 0;
}

int device_iio_utils::sysfs_read_int(char *file, int *val)
{
    FILE *fp;
    int ret;

    fp = fopen(file, "r");
    if (NULL == fp)
        return -errno;

    ret = fscanf(fp, "%d\n", val);
    fclose(fp);

    return ret;
}

int device_iio_utils::sysfs_read_uint(char *file, unsigned int *val)
{
    FILE *fp;
    int ret;

    fp = fopen(file, "r");
    if (NULL == fp)
        return -errno;

    ret = fscanf(fp, "%u\n", val);
    fclose(fp);

    return ret;
}

int device_iio_utils::sysfs_read_float(char *file, float *val)
{
    FILE *fp;
    int ret;

    fp = fopen(file, "r");
    if (NULL == fp)
        return -errno;

    ret = fscanf(fp, "%f", val);
    fclose(fp);

    return ret;
}

int device_iio_utils::sysfs_read_str(char *file, char *str, int len)
{
    FILE *fp;
    int err = 0;

    fp = fopen(file, "r");
    if (fp == NULL)
        return -errno;

    if (fgets(str, len, fp) == NULL)
        err = -errno;

    fclose(fp);

    return err;
}

int device_iio_utils::check_file(const char *filename)
{
    struct stat info;

    return stat(filename, &info);
}

int device_iio_utils::get_device_by_name(const char *name)
{
    struct dirent *ent;
    int number;
    FILE *deviceFile;
    DIR *dp;
    char dname[DEVICE_IIO_MAX_NAME_LENGTH];
    char dfilename[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    int ret;

    ret = sysfs_opendir(device_iio_dir, &dp);
    if (ret)
        return ret;

    while ((ent = readdir(dp)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        size_t prefix_len = strlen(device_iio_device_name);
        if ((strncmp(ent->d_name, device_iio_device_name, prefix_len) == 0) &&
            (strlen(ent->d_name) > prefix_len)) {
            if (sscanf(ent->d_name + prefix_len, "%d", &number) != 1)
                continue;

            ret = snprintf(dfilename, sizeof(dfilename), "%s%s%d/name",
                           device_iio_dir, device_iio_device_name, number);
            if (ret < 0)
                continue;

            deviceFile = fopen(dfilename, "r");
            if (!deviceFile)
                continue;

            if (fgets(dname, sizeof(dname), deviceFile)) {
                size_t len = strlen(dname);
                if (len > 0 && dname[len - 1] == '\n')
                    dname[len - 1] = '\0';

                if (strncmp(name, dname, DEVICE_IIO_MAX_NAME_LENGTH) == 0) {
                    fclose(deviceFile);
                    closedir(dp);

                    return number;
                }
            }
            fclose(deviceFile);
        }
    }

    closedir(dp);

    return -ENODEV;
}

int device_iio_utils::get_devices_name(struct device_iio_type_name devices[],
                                       unsigned int max_list)
{
    DIR *dp;
    FILE *nameFile;
    size_t iio_len;
    const struct dirent *ent;
    int err, number;
    unsigned int device_num = 0;
    char thisname[DEVICE_IIO_MAX_FILENAME_LEN];
    char *filename;
    size_t filename_len;

    err = sysfs_opendir(device_iio_dir, &dp);
    if (err)
        return err;

    iio_len = strlen("iio:device");
    while ((ent = readdir(dp)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        if (strncmp(ent->d_name, "iio:device", iio_len) == 0 &&
            strlen(ent->d_name) > iio_len) {
            if (sscanf(ent->d_name + iio_len, "%d", &number) != 1)
                continue;

            filename_len = strlen(device_iio_dir) + strlen(ent->d_name) +
                                  strlen("/name") + 2;
            filename = (char *)malloc(filename_len);
            if (filename == NULL) {
                closedir(dp);

                return -ENOMEM;
            }

            err = snprintf(filename, filename_len, "%s%s/name",
                           device_iio_dir, ent->d_name);
            if (err < 0) {
                free(filename);
                closedir(dp);

                return err;
            }

            nameFile = fopen(filename, "r");
            free(filename);
            if (!nameFile)
                continue;

            if (fgets(thisname, DEVICE_IIO_MAX_FILENAME_LEN, nameFile)) {
                size_t len = strlen(thisname);
                if (len > 0 && thisname[len - 1] == '\n')
                    thisname[len - 1] = '\0';

                strncpy(devices[device_num].name, thisname,
                        DEVICE_IIO_MAX_FILENAME_LEN - 1);
                devices[device_num].name[DEVICE_IIO_MAX_FILENAME_LEN - 1] = '\0';
                devices[device_num].num = number;
                device_num++;

                if (device_num >= max_list) {
                    fclose(nameFile);
                    break;
                }
            }
            fclose(nameFile);
        }
    }

    closedir(dp);

    return (int)device_num;
}

int device_iio_utils::scan_channel(const char *device_dir,
                                   struct device_iio_info_channel **data,
                                   int *counter)
{
    DIR *dp = NULL;
    int ret = 0, i, x, y;
    unsigned int temp;
    const struct dirent *ent;
    char dname[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    char dfilename[2 * DEVICE_IIO_MAX_FILENAME_LEN + 1];
    struct device_iio_info_channel *current, ch_ord;
    const char *tmp;

    *data = NULL;
    *counter = 0;

    memset(dname, 0, DEVICE_IIO_MAX_FILENAME_LEN + 1);
    memset(dfilename, 0, DEVICE_IIO_MAX_FILENAME_LEN + 1);
    ret = snprintf(dname, sizeof(dname), "%s/scan_elements", device_dir);
    if (ret < 0)
        return ret;

    ret = sysfs_opendir(dname, &dp);
    if (ret)
        return ret;

    /* Count how many channel info in scan_elements */
    for (ent = readdir(dp); ent; ent = readdir(dp))  {
        tmp = &ent->d_name[strlen(ent->d_name) -
              strlen(device_iio_scan_elements_en)];
        if (strncmp(tmp, device_iio_scan_elements_en,
                    strlen(device_iio_scan_elements_en)) == 0) {
            if (strlen(dname) + strlen(ent->d_name) + 1 >
                DEVICE_IIO_MAX_FILENAME_LEN) {
                ret = -ENOMEM;
                goto error_cleanup_array;
            }

            /* open all scan_element xxx_en files and enable it */
            ret = snprintf(dfilename, sizeof(dfilename), "%s/%s",
                           dname, ent->d_name);
            if (ret < 0)
                goto error_cleanup_array;

            ret = sysfs_write_uint(dfilename, ENABLE_CHANNEL);
            if (ret < 0) {
                    ret = -errno;
                    goto error_cleanup_array;
                }

            /* Check for scan enabled */
            ret = sysfs_read_uint(dfilename, &temp);
            if (ret > 0 && temp == 1) {
                /* Allocate new channel and populate */
                *data = (struct device_iio_info_channel *)realloc(*data,
                            ((*counter) + 1) * sizeof(device_iio_info_channel));
                if (!*data) {
                    ret = -ENOMEM;
                    goto error_cleanup_array;
                }

                current = &(*data)[*counter];
                current->enabled = temp;
                current->scale = 1.0f;
                current->offset = 0.0f;
                current->name = strndup(ent->d_name, strlen(ent->d_name) -
                                        strlen(device_iio_scan_elements_en));
                if (current->name == NULL) {
                    ret = -ENOMEM;
                    goto error_cleanup_array;
                }

                ret = snprintf(dfilename, sizeof(dfilename), "%s/%s_index",
                               dname, current->name);
                if (ret < 0)
                    goto error_cleanup_array;

                ret = sysfs_read_uint(dfilename, &current->index);
                if ((ret <= 0) && ((errno != ENOENT) && (errno != EACCES))) {
                    ret = -errno;
                    goto error_cleanup_array;
                }

                ret = snprintf(dfilename, sizeof(dfilename), "%s/%s_scale",
                               device_dir, current->name);
                if (ret < 0)
                    goto error_cleanup_array;

                ret = sysfs_read_float(dfilename, &current->scale);
                if ((ret <= 0) && ((errno != ENOENT) && (errno != EACCES))) {
                    ret = -errno;
                    goto error_cleanup_array;
                }

                ret = snprintf(dfilename, sizeof(dfilename), "%s/%s_offset",
                               device_dir, current->name);
                if (ret < 0)
                    goto error_cleanup_array;

                ret = sysfs_read_float(dfilename, &current->offset);
                if ((ret <= 0) && ((errno != ENOENT) && (errno != EACCES))) {
                     ret = -errno;
                    goto error_cleanup_array;
                }

                ret = get_type(current, device_dir, current->name, "in");
                current->location = 0;
                (*counter)++;
            }
        }
    }

    closedir(dp);

    /* reorder index in channel array */
    for (x = 0; x < (*counter); x++) {
        for (y = 0; y < ((*counter) - 1); y++) {
            if ((*data)[y].index > (*data)[y + 1].index) {
                ch_ord = (*data)[y + 1];
                (*data)[y + 1] = (*data)[y];
                (*data)[y] = ch_ord;
            }
        }
    }

    return 0;

error_cleanup_array:
    if (*data) {
        for (i = *counter - 1; i >= 0; i--) {
            if ((*data)[i].name)
                free((*data)[i].name);
        }

        free(*data);
    }

    closedir(dp);

    return ret;
}

int device_iio_utils::enable_events(const char *device_dir, bool enable)
{
    char event_el_dir[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    char filename[2 * DEVICE_IIO_MAX_FILENAME_LEN + 1];
    const struct dirent *ent;
    FILE *sysfsfp;
    int err = 0;
    DIR *dp;

    err = snprintf(event_el_dir, sizeof(event_el_dir),
                   device_iio_event_dir, device_dir);
    if (err < 0)
        return err;

    dp = opendir(event_el_dir);
    if (!dp) {
        err = (errno == ENOENT) ? 0 : -errno;
        return err;
    }

    while ((ent = readdir(dp)) != NULL) {
        size_t name_len = strlen(ent->d_name);
        const char *suffix = "_en";
        size_t suffix_len = strlen(suffix);

        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        if (name_len > suffix_len &&
            strcmp(ent->d_name + name_len - suffix_len, suffix) == 0) {
            err = snprintf(filename, sizeof(filename), "%s/%s",
                           event_el_dir, ent->d_name);
            if (err < 0)
                 goto error_close_dir;

            sysfsfp = fopen(filename, "r+");
            if (!sysfsfp) {
                if ((errno == ENOENT) || (errno == EACCES))
                    err = 0;
                else
                    err = -errno;

                goto error_close_dir;
            }

            /* enable sensor event */
            fprintf(sysfsfp, "%d", enable ? 1 : 0);
            fclose(sysfsfp);
        }
    }

error_close_dir:
    closedir(dp);

    return err;
}

int device_iio_utils::enable_sensor(const char *device_dir, bool enable)
{
    char enable_file[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    int err;

    err = check_file(device_dir);
    if (err == 0) {
        err = snprintf(enable_file, sizeof(enable_file), "%s/%s",
                       device_dir, device_iio_buffer_enable);
        if (err < 0)
            return err;

        err = sysfs_write_int(enable_file, enable ? 1 : 0);
        if (err)
            return err;
    } else if (err != -ENOENT) {
        /* permission error */
        return err;
    }

    /* avoid manage enable events from sensor */
    return 0;
    //return enable_events(device_dir, enable);
}

int device_iio_utils::get_sampling_frequency_available(const char *device_dir,
                                          struct device_iio_sampling_freqs *sfa)
{
    char sf_filename[DEVICE_IIO_MAX_FILENAME_LEN];
    char *pch, *saveptr;
    char line[100];
    int err = 0;

    sfa->length = 0;

    err = snprintf(sf_filename, sizeof(sf_filename), "%s/%s",
                   device_dir, device_iio_sfa_filename);
    if (err < 0)
        return err;

    err = sysfs_read_str(sf_filename, line, sizeof(line));
    if (err < 0)
        return err;

    for (pch = strtok_r(line, " ,", &saveptr);
         pch != NULL && sfa->length < DEVICE_IIO_MAX_SAMP_FREQ_AVAILABLE;
         pch = strtok_r(NULL, " ,", &saveptr)) {
        sfa->freq[sfa->length] = atof(pch);
        sfa->length++;
    }

    return 0;
}

int device_iio_utils::get_hw_fifo_length(const char *device_dir)
{
    int ret;
    int len = 0;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];

    /* read <iio:devicex>/hwfifo_watermark_max */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_hw_fifo_length);
    if (ret < 0)
        return 0;

    ret = sysfs_read_int(tmp_filename, &len);
    if (ret < 0 || len <= 0)
        return 0;

     /* write "len * 2" -> <iio:devicex>/buffer/length */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_buffer_length);
    if (ret < 0)
        return len;

    ret = sysfs_write_int(tmp_filename, 2 * len);
    if (ret < 0)
        return len;

    /* write "1" -> <iio:devicex>/hwfifo_enabled */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_hw_fifo_enabled);
    if (ret < 0)
        return len;

     /* used for compatibility with old iio API */
    ret = check_file(tmp_filename);
    if (ret < 0 && errno == ENOENT)
        return len;

    ret = sysfs_write_int(tmp_filename, 1);
    if (ret < 0)
         console.error(std::string("Failed to enable hw fifo: ") +
                       tmp_filename);

    return len;
}

int device_iio_utils::set_sampling_frequency(char *device_dir,
                                             unsigned int frequency)
{
    int ret;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];

    /* write "frequency" -> <iio:devicex>/sampling_frequency */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_sf_filename);
    if (ret < 0)
        return ret;

    /* it's ok if file not exists */
    ret = check_file(tmp_filename);
    if (ret < 0 && errno == ENOENT)
        return 0;

    return sysfs_write_int(tmp_filename, frequency);
}

int device_iio_utils::set_max_delivery_rate(const char *device_dir,
                                            unsigned int delay)
{
    int ret;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];

    /* write "delay" -> <iio:devicex>/max_delivery_rate */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_max_delivery_rate_filename);
    if (ret < 0)
        return ret;

    /* it's ok if file not exists */
    ret = check_file(tmp_filename);
    if (ret < 0 && errno == ENOENT)
        return 0;

    return sysfs_write_int(tmp_filename, delay);
}

int device_iio_utils::set_hw_fifo_watermark(char *device_dir,
                                            unsigned int watermark)
{
    int ret;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];

    /* write "watermark" -> <iio:devicex>/hwfifo_watermark */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_hw_fifo_watermark);
    if (ret < 0)
        return ret;

    /* it's ok if file not exists */
    ret = check_file(tmp_filename);
    if (ret < 0 && errno == ENOENT)
        return 0;

    return sysfs_write_int(tmp_filename, watermark);
}

int device_iio_utils::hw_fifo_flush(char *device_dir)
{
    int ret;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];

    /* write "1" -> <iio:devicex>/hwfifo_flush */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_hw_fifo_flush);
    if (ret < 0)
        return ret;

    /* it's ok if file not exists */
    ret = check_file(tmp_filename);
    if (ret < 0 && errno == ENOENT)
        return 0;

    return sysfs_write_int(tmp_filename, 1);
}

int device_iio_utils::set_scale(const char *device_dir, float value,
                                device_iio_chan_type_t device_type)
{
    int ret;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];
    const char *scale_filename;

    switch (device_type) {
    case DEVICE_IIO_ACC:
        scale_filename = "in_accel_x_scale";
        break;
    case DEVICE_IIO_GYRO:
        scale_filename = "in_anglvel_x_scale";
        break;
    case DEVICE_IIO_MAGN:
        scale_filename = "in_magn_x_scale";
        break;
    case DEVICE_IIO_PRESSURE:
        scale_filename = "in_press_scale";
        break;
    case DEVICE_IIO_TEMP:
        scale_filename = "in_temp_scale";
        break;
    default:
        return -EINVAL;
    }

    /* write scale -> <iio:devicex>/in_<device_type>_x_scale */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, scale_filename);
    if (ret < 0)
        return ret;

    return sysfs_write_float(tmp_filename, value);
}

int device_iio_utils::get_scale(const char *device_dir, float *value,
                                device_iio_chan_type_t device_type)
{
    int ret;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];
    const char *scale_filename;

    switch (device_type) {
    case DEVICE_IIO_ACC:
        scale_filename = "in_accel_x_scale";
        break;
    case DEVICE_IIO_GYRO:
        scale_filename = "in_anglvel_x_scale";
        break;
    case DEVICE_IIO_MAGN:
        scale_filename = "in_magn_x_scale";
        break;
    case DEVICE_IIO_PRESSURE:
        scale_filename = "in_press_scale";
        break;
    case DEVICE_IIO_TEMP:
        scale_filename = "in_temp_scale";
        break;
    default:
        return -EINVAL;
    }

    /* read <iio:devicex>/in_<device_type>_x_scale */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, scale_filename);
    if (ret < 0)
        return ret;

    return sysfs_read_float(tmp_filename, value);
}

static bool isTimestampChannel(const char *name)
{
    std::regex e("(.*)(timestamp)(.*)");

    if (std::regex_match(name, e))
        return true;

    return false;
}

static bool isHwTimestampChannel(const char *name)
{
    std::regex e("(.*)(count)(.*)");

    if (std::regex_match(name, e))
        return true;

    return false;
}

int device_iio_utils::get_type(struct device_iio_info_channel *channel,
                               const char *device_dir, const char *name,
                               const char *pre_name)
{
    DIR *dp = NULL;
    int ret = 0;
    FILE *sysfsfp;
    unsigned int padint;
    const struct dirent *ent;
    char signchar, endianchar;
    char dir[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    char type_name[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    char name_pre_name[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    char filename[2 * DEVICE_IIO_MAX_FILENAME_LEN + 1];

    /* Check string len */
    if (strlen(device_dir) + strlen("/scan_elements") >= sizeof(dir))
        return -ENOMEM;
    if (strlen(name) + strlen("_type") >= sizeof(type_name))
        return -ENOMEM;
    if (strlen(pre_name) + strlen("_type") >= sizeof(name_pre_name))
        return -ENOMEM;

    if (isTimestampChannel(name))
        channel->type = IIOChannelType::TIMESTAMP;
    else if (isHwTimestampChannel(name))
        channel->type = IIOChannelType::HW_TIMESTAMP;
    else
        channel->type = IIOChannelType::UNKNOWN;

    ret = snprintf(dir, sizeof(dir), "%s/scan_elements", device_dir);
    if (ret < 0)
        return ret;

    ret = snprintf(type_name, sizeof(type_name), "%s_type", name);
    if (ret < 0)
        return ret;

    ret = snprintf(name_pre_name, sizeof(name_pre_name), "%s_type", pre_name);
    if (ret < 0)
        return ret;

    ret = sysfs_opendir(dir, &dp);
    if (ret)
        return ret;

    while ((ent = readdir(dp)) != NULL) {
        if ((strcmp(type_name, ent->d_name) == 0) ||
            (strcmp(name_pre_name, ent->d_name) == 0)) {
            if (snprintf(filename, sizeof(filename), "%s/%s", dir,
                         ent->d_name) >= (int)sizeof(filename))
                continue;

            sysfsfp = fopen(filename, "r");
            if (sysfsfp == NULL)
                continue;

            /* scan format like "le:s16/16>>0" */
            ret = fscanf(sysfsfp, "%ce:%c%u/%u>>%u",
                         &endianchar,
                         &signchar,
                         &channel->bits_used,
                         &padint,
                         &channel->shift);
            fclose(sysfsfp);

            if (ret < 0)
                continue;

            channel->be = (endianchar == 'b');
            channel->sign = (signchar == 's');
            channel->bytes = (padint >> 3);

            if (channel->bits_used == 64)
                channel->mask = ~0ULL;
            else
                channel->mask = (1ULL << channel->bits_used) - 1;
        }
    }

    closedir(dp);

    return 0;
}

int device_iio_utils::get_available_scales(const char *device_dir,
                                           struct device_iio_scales *sa,
                                           device_iio_chan_type_t device_type)
{
    int err = 0;
    FILE *fp = NULL;
    const char *avl_name;
    char tmp_name[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    char line[DEVICE_IIO_MAX_FILENAME_LEN + 1];
    char *pch, *saveptr;

    sa->length = 0;

    /* check on all supported sensor type */
    switch (device_type) {
    case DEVICE_IIO_ACC:
        avl_name = "in_accel_scale_available";
        break;
    case DEVICE_IIO_GYRO:
        avl_name = "in_anglvel_scale_available";
        break;
    case DEVICE_IIO_TEMP:
        avl_name = "in_temp_scale_available";
        break;
    case DEVICE_IIO_MAGN:
        avl_name = "in_magn_scale_available";
        break;
    case DEVICE_IIO_PRESSURE:
        avl_name = "in_press_scale_available";
        break;
    case DEVICE_IIO_HUMIDITYRELATIVE:
        avl_name = "in_humidityrelative_scale_available";
        break;
    default:
        return -EINVAL;
    }

    /* check string len */
    if (strlen(device_dir) + strlen(avl_name) + 1 > sizeof(tmp_name) - 1)
        return -ENOMEM;

    err = snprintf(tmp_name, sizeof(tmp_name), "%s/%s", device_dir, avl_name);
    if (err < 0)
        return err;

    /* if scale not available not report error */
    fp = fopen(tmp_name, "r");
    if (fp == NULL)
        return 0;

    if (!fgets(line, sizeof(line), fp)) {
        err = -EINVAL;
        fclose(fp);
        return err;
    }

    for (pch = strtok_r(line, " ", &saveptr);
         pch != NULL && sa->length < DEVICE_IIO_SCALE_AVAILABLE;
         pch = strtok_r(NULL, " ", &saveptr)) {
        sa->scales[sa->length] = atof(pch);
        sa->length++;
    }

    fclose(fp);

    return 0;
}

/* Sensor data injection mode */
int device_iio_utils::support_injection_mode(const char *device_dir)
{
    int ret;
    int value;
    char injectors[30];
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN + 1];

    /* Check string len */
    if (strlen(device_dir) +
        strlen(device_iio_injection_mode_enable) + 1 > sizeof(tmp_filename) - 1)
        return -1;

    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_injection_mode_enable);
    if (ret < 0)
        return ret;

    ret = sysfs_read_int(tmp_filename, &value);
    if (ret < 0) {
        ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                       device_dir, device_iio_injection_sensors_filename);
        if (ret < 0)
            return ret;

        ret = sysfs_read_str(tmp_filename, injectors, sizeof(injectors));
        if (ret < 0)
            return ret;

        return 1;
    }

    return 0;
}

int device_iio_utils::set_injection_mode(const char *device_dir, bool enable)
{
    int ret;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];

    /* write "enable" -> <iio:devicex>/injection_mode */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_injection_mode_enable);
    if (ret < 0)
        return ret;

    return sysfs_write_int(tmp_filename, enable ? 1 : 0);
}

int device_iio_utils::inject_data(const char *device_dir, unsigned char *data,
                                  int len, device_iio_chan_type_t device_type)
{
    const char *injection_filename;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];
    FILE *sysfsfp;
    int ret = 0;

    switch (device_type) {
    case DEVICE_IIO_ACC:
        injection_filename = "in_accel_injection_raw";
        break;
    case DEVICE_IIO_GYRO:
        injection_filename = "in_anglvel_injection_raw";
        break;
    case DEVICE_IIO_MAGN:
        injection_filename = "in_magn_injection_raw";
        break;
    case DEVICE_IIO_PRESSURE:
        injection_filename = "in_press_injection_raw";
        break;
    case DEVICE_IIO_TEMP:
        injection_filename = "in_temp_injection_raw";
        break;
    default:
        return -EINVAL;
    }

    /* Check string len */
    if (strlen(device_dir) +
        strlen(injection_filename) + 2 > sizeof(tmp_filename))
        return -ENOMEM;

    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, injection_filename);
    if (ret < 0)
        return ret;

    sysfsfp = fopen(tmp_filename, "w");
    if (sysfsfp == NULL)
        return -errno;

    ret = fwrite(data, 1, len, sysfsfp);
    fclose(sysfsfp);

    return (ret == len) ? 0 : -EIO;
}

int device_iio_utils::set_clock_type(const char *device_dir, const char *type)
{
    int ret;
    char tmp_filename[DEVICE_IIO_MAX_FILENAME_LEN];

    /* write "enable" -> <iio:devicex>/injection_mode */
    ret = snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s",
                   device_dir, device_iio_current_timestamp_clock);
    if (ret < 0)
        return ret;

    return sysfs_write_str(tmp_filename, (char *)type);
}

int device_iio_utils::get_selftest_available(const char *device_dir,
                                             char list[][20])
{
    FILE *fp;
    int elements = 0;
    char *pch, *saveptr;
    char line[200];
    char sf_filename[DEVICE_IIO_MAX_FILENAME_LEN];
    int ret;

    ret = snprintf(sf_filename, sizeof(sf_filename), "%s/%s",
                   device_dir, device_iio_selftest_available_filename);
    if (ret < 0)
        return ret;

    fp = fopen(sf_filename, "r");
    if (fp == NULL)
        return -errno;

    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return -EINVAL;
    }

    for (pch = strtok_r(line, " ,.", &saveptr);
         pch != NULL && elements < 20;
         pch = strtok_r(NULL, " ,.", &saveptr)) {
        strncpy(list[elements], pch, 19);
        list[elements][19] = '\0';
        elements++;
    }

    fclose(fp);

    return elements;
}

int device_iio_utils::execute_selftest(const char *device_dir, char *mode)
{
    int ret;
    char result[20];
    char sf_filename[DEVICE_IIO_MAX_FILENAME_LEN];

    ret = snprintf(sf_filename, sizeof(sf_filename), "%s/%s",
                   device_dir, device_iio_selftest_filename);
    if (ret < 0)
        return ret;

    ret = sysfs_write_str(sf_filename, mode);
    if (ret < 0)
        return ret;

    ret = sysfs_read_str(sf_filename, result, sizeof(result));
    if (ret < 0)
        return ret;

    if (strncmp(result, SELFTEST_POSITIVE_RESULT,
                strlen(SELFTEST_POSITIVE_RESULT)) == 0)
        return 1;

    if (strncmp(result, SELFTEST_NEGATIVE_RESULT,
                strlen(SELFTEST_NEGATIVE_RESULT)) == 0)
        return 0;

    return -EINVAL;
}

int device_iio_utils::get_module_id(const char *device_dir)
{
    char filename[DEVICE_IIO_MAX_FILENAME_LEN];
    int ret, id;

    ret = snprintf(filename, sizeof(filename), "%s/%s",
                   device_dir, device_iio_module_id_filename);
    if (ret < 0)
        return ret;

    ret = sysfs_read_int(filename, &id);
    if (ret < 0)
        return ret;

    return id;
}

} // namespace core
} // namespace stm
