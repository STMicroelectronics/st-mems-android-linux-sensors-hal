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
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <endian.h>
#include <unistd.h>
#include <memory>
#include <array>
#include <algorithm>

#include <IConsole.h>
#include "SensorsGraph.h"
#include <PropertiesManager.h>

#include "SensorHAL.h"
#include "Accelerometer.h"
#include "Magnetometer.h"
#include "Gyroscope.h"
#include "StepDetector.h"
#include "StepCounter.h"
#include "SignificantMotion.h"
#include "TiltSensor.h"
#include "SWMagnetometerUncalibrated.h"
#include "SWGyroscopeUncalibrated.h"
#include "SWAccelerometerUncalibrated.h"
#include "Pressure.h"
#include "RHumidity.h"
#include "Temp.h"
#include "SWAccelGyroFusion6X.h"
#include "SWGameRotationVector.h"
#include "SWAccelMagnGyroFusion9X.h"
#include "SWRotationVector.h"
#include "SWOrientation.h"
#include "SWGravity.h"
#include "SWLinearAccel.h"
#include "WristTiltGesture.h"
#include "Gesture.h"
#include "DeviceOrientation.h"
#include "utils.h"
#include "SensorsSupported.h"
#include "sensors_legacy.h"

namespace stm {
namespace core {

static IConsole &console = IConsole::getInstance();

/*
 * STSensorHAL_iio_devices_data: informations related to the IIO devices, used during open-sensor function
 * @iio_sysfs_path: IIO device sysfs path.
 * @device_name: IIO device name.
 * @android_name: name showed in Android OS.
 * @dev_id: iio:device device id.
 * @sensor_type: Android sensor type.
 * @wake_up_sensor: is a wake-up sensor.
 * @num_channels: number of channels.
 * @channels: channels data.
 * @sa: scale factors available.
 * @hw_fifo_len: hw FIFO length.
 * @power_consumption: sensor power consumption in mA.
 * @sfa: sampling frequency available.
 */
struct STSensorHAL_iio_devices_data {
    std::string iio_sysfs_path;
    std::string deviceName;
    std::string androidName;

    unsigned int dev_id;
    SensorType sensor_type;
    int moduleId;

    bool wake_up_sensor;

    int num_channels;
    struct device_iio_info_channel *channels;
    struct device_iio_scales sa;

    unsigned int hw_fifo_len;
    float power_consumption;

    struct device_iio_sampling_freqs sfa;
} typedef STSensorHAL_iio_devices_data;



// /**************** Tilt sensors ****************/
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_0, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "Test Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_1, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM6DS3 Tilt Sensor", 240E-3f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_14, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LIS2DS12 Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_15, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LIS2DG Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_20, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM6DSM Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_21, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM6DS33 Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_22, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM6DS3H Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_23, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM303AH Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_30, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM6DSO Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_31, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM6DSL Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_20, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "ISM330DLC Tilt Sensor", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_46, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM6DSR Tilt Sensor", 0.0f),
//         ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_48, TILT_NAME_SUFFIX_IIO), STMSensorType::TILT_DETECTOR, DEVICE_IIO_TILT, "LSM6DSO32 Tilt Sensor", 0.0f),

// /**************** Wrist Tilt Gesture ****************/
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_22, WRIST_GESTURE_NAME_SUFFIX_IIO), STMSensorType::WRIST_TILT_GESTURE, DEVICE_IIO_WRIST_TILT_GESTURE, "LSM6DS3H Wrist Tilt Gesture", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_30, WRIST_GESTURE_NAME_SUFFIX_IIO), STMSensorType::WRIST_TILT_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSO Wrist Tilt Gesture", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_46, WRIST_GESTURE_NAME_SUFFIX_IIO), STMSensorType::WRIST_TILT_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSR Wrist Tilt Gesture", 0.0f),
//         ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_48, WRIST_GESTURE_NAME_SUFFIX_IIO), STMSensorType::WRIST_TILT_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSO32 Wrist Tilt Gesture", 0.0f),

// /**************** Glance Gesture ****************/
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_30, GLANCE_GESTURE_NAME_SUFFIX_IIO), STMSensorType::GLANCE_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSO Glance Gesture", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_46, GLANCE_GESTURE_NAME_SUFFIX_IIO), STMSensorType::GLANCE_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSR Glance Gesture", 0.0f),
//         ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_48, GLANCE_GESTURE_NAME_SUFFIX_IIO), STMSensorType::GLANCE_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSO32 Glance Gesture", 0.0f),

// /**************** WakeUp Gesture ****************/
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_30, ST_HAL_WAKEUP_SUFFIX_IIO), STMSensorType::WAKE_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSO WakeUp Gesture", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_27, ST_HAL_WAKEUP_SUFFIX_IIO), STMSensorType::WAKE_GESTURE, DEVICE_IIO_GESTURE, "LIS2MDL WakeUp Gesture", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_46, ST_HAL_WAKEUP_SUFFIX_IIO), STMSensorType::WAKE_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSR WakeUp Gesture", 0.0f),
//         ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_48, ST_HAL_WAKEUP_SUFFIX_IIO), STMSensorType::WAKE_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSO32 WakeUp Gesture", 0.0f),

// /**************** PickUp Gesture ****************/
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_30, ST_HAL_PICKUP_SUFFIX_IIO), STMSensorType::PICK_UP_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSO PickUp Gesture", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_46, ST_HAL_PICKUP_SUFFIX_IIO), STMSensorType::PICK_UP_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSR PickUp Gesture", 0.0f),
//         ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_48, ST_HAL_PICKUP_SUFFIX_IIO), STMSensorType::PICK_UP_GESTURE, DEVICE_IIO_GESTURE, "LSM6DSO32 PickUp Gesture", 0.0f),

// /**************** Motion Gesture ****************/
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_30, ST_HAL_MOTION_SUFFIX_IIO), STMSensorType::MOTION_DETECT, DEVICE_IIO_GESTURE, "LSM6DSO Motion Gesture", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_46, ST_HAL_MOTION_SUFFIX_IIO), STMSensorType::MOTION_DETECT, DEVICE_IIO_GESTURE, "LSM6DSR Motion Gesture", 0.0f),
//         ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_48, ST_HAL_MOTION_SUFFIX_IIO), STMSensorType::MOTION_DETECT, DEVICE_IIO_GESTURE, "LSM6DSO32 Motion Gesture", 0.0f),

// /**************** No-Motion Gesture ****************/
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_30, ST_HAL_NO_MOTION_SUFFIX_IIO), STMSensorType::STATIONARY_DETECT, DEVICE_IIO_GESTURE, "LSM6DSO No-Motion Gesture", 0.0f),
// 	ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_46, ST_HAL_NO_MOTION_SUFFIX_IIO), STMSensorType::STATIONARY_DETECT, DEVICE_IIO_GESTURE, "LSM6DSR No-Motion Gesture", 0.0f),
//         ST_sensors_supported_t(CONCATENATE_STRING(ST_SENSORS_LIST_48, ST_HAL_NO_MOTION_SUFFIX_IIO), STMSensorType::STATIONARY_DETECT, DEVICE_IIO_GESTURE, "LSM6DSO32 No-Motion Gesture", 0.0f),

static int st_hal_set_operation_mode(unsigned int mode);

/*
 * st_hal_create_virtual_class_sensor - Istance virtual sensor class
 * @sensor_type: Android sensor type.
 * @handle: android handle number.
 *
 * Return value: sensor class pointer on success, NULL pointer on fail.
 */
static std::shared_ptr<SensorBase> st_hal_create_virtual_class_sensor(const STMSensorType &sensor_type,
                                                                      int handle,
                                                                      int moduleId)
{
    std::shared_ptr<SensorBase> sensor = nullptr;

    if (sensor_type.isInternal()) {
        if (sensor_type == AccelGyroFusion6XSensorType) {
            sensor = std::make_shared<SWAccelGyroFusion6X>("Accel-Gyro Fusion 6X", handle, moduleId);
        } else if (sensor_type == AccelMagnGyroFusion9XSensorType) {
            sensor = std::make_shared<SWAccelMagnGyroFusion9X>("Accel-Magn-Gyro Fusion 9X", handle, moduleId);
        }
    } else {
        if (sensor_type == AccelUncalibSensorType) {
            sensor = std::make_shared<SWAccelerometerUncalibrated>("Accelerometer Uncalibrated Sensor", handle, moduleId);
        } else if (sensor_type == MagnUncalibSensorType) {
            sensor = std::make_shared<SWMagnetometerUncalibrated>("Magnetometer Uncalibrated Sensor", handle, moduleId);
        } else if (sensor_type == GyroUncalibSensorType) {
            sensor = std::make_shared<SWGyroscopeUncalibrated>("Gyroscope Uncalibrated Sensor", handle, moduleId);
        } else if (sensor_type == GameRotationVecSensorType) {
            sensor = std::make_shared<SWGameRotationVector>("iNemoEngine Game Rotation Vector Sensor", handle, moduleId);
        } else if (sensor_type == RotationVecSensorType) {
            sensor = std::make_shared<SWRotationVector>("iNemoEngine Rotation Vector Sensor", handle, moduleId);
        } else if (sensor_type == OrientationSensorType) {
            sensor = std::make_shared<SWOrientation>("iNemoEngine Orientation Sensor", handle, moduleId);
        } else if (sensor_type == GravitySensorType) {
            sensor = std::make_shared<SWGravity>("iNemoEngine Gravity Sensor", handle, moduleId);
        } else if (sensor_type == LinearAccelSensorType) {
            sensor = std::make_shared<SWLinearAccel>("iNemoEngine Linear Acceleration Sensor", handle, moduleId);
        }
    }

    if (sensor == nullptr) {
        return nullptr;
    }

    return sensor->IsValidClass() ? sensor : nullptr;
}

/*
 * st_hal_create_class_sensor() - Istance hardware sensor class
 * @data: iio:device data.
 * @handle: Android handle number.
 *
 * Return value: sensor class pointer on success, NULL pointer on fail.
 */
static std::shared_ptr<SensorBase> st_hal_create_class_sensor(STSensorHAL_iio_devices_data *data,
                                                              int sensorId,
                                                              int moduleId)
{
    struct HWSensorBaseCommonData class_data;
    std::shared_ptr<SensorBase> sensor = nullptr;

    if ((strlen(data->iio_sysfs_path.c_str()) + 1 > HW_SENSOR_BASE_IIO_SYSFS_PATH_MAX) ||
        (strlen(data->deviceName.c_str()) + 1 > HW_SENSOR_BASE_IIO_DEVICE_NAME_MAX) ||
        (data->num_channels > HW_SENSOR_BASE_MAX_CHANNELS)) {
        return nullptr;
    }

    memcpy(class_data.device_name, data->deviceName.c_str(), strlen(data->deviceName.c_str()) + 1);
    memcpy(class_data.device_iio_sysfs_path, data->iio_sysfs_path.c_str(), strlen(data->iio_sysfs_path.c_str()) + 1);
    memcpy(&class_data.sa, &data->sa, sizeof(class_data.sa));
    memcpy(class_data.channels, data->channels, data->num_channels * sizeof(class_data.channels[0]));

    class_data.device_iio_dev_num = data->dev_id;
    class_data.num_channels = data->num_channels;

    if (data->sensor_type == AccelSensorType) {
        sensor = std::make_shared<Accelerometer>(&class_data,
                                                 data->androidName.c_str(), &data->sfa,
                                                 sensorId, data->hw_fifo_len,
                                                 data->power_consumption,
                                                 data->wake_up_sensor,
                                                 moduleId);
    } else if (data->sensor_type == MagnSensorType) {
        sensor = std::make_shared<Magnetometer>(&class_data,
                                                data->androidName.c_str(), &data->sfa,
                                                sensorId, data->hw_fifo_len,
                                                data->power_consumption,
                                                data->wake_up_sensor,
                                                moduleId);
    } else if (data->sensor_type == GyroSensorType) {
        sensor = std::make_shared<Gyroscope>(&class_data,
                                             data->androidName.c_str(), &data->sfa,
                                             sensorId, data->hw_fifo_len,
                                             data->power_consumption,
                                             data->wake_up_sensor,
                                             moduleId);
    } else if (data->sensor_type == StepDetectorSensorType) {
        sensor = std::make_shared<StepDetector>(&class_data,
                                                data->androidName.c_str(),
                                                sensorId, data->hw_fifo_len,
                                                data->power_consumption,
                                                data->wake_up_sensor,
                                                moduleId);
    } else if (data->sensor_type == StepCounterSensorType) {
        sensor = std::make_shared<StepCounter>(&class_data,
                                               data->androidName.c_str(),
                                               sensorId, data->hw_fifo_len,
                                               data->power_consumption,
                                               data->wake_up_sensor,
                                               moduleId);
    } else if (data->sensor_type == SignMotionSensorType) {
        sensor = std::make_shared<SignMotion>(&class_data,
                                              data->androidName.c_str(),
                                              sensorId,
                                              data->power_consumption,
                                              moduleId);
    } else if (data->sensor_type == TiltDetectorSensorType) {
        sensor = std::make_shared<TiltSensor>(&class_data,
                                              data->androidName.c_str(),
                                              sensorId, data->hw_fifo_len,
                                              data->power_consumption,
                                              moduleId);
    } else if (data->sensor_type == PressureSensorType) {
        sensor = std::make_shared<Pressure>(&class_data,
                                            data->androidName.c_str(), &data->sfa,
                                            sensorId, data->hw_fifo_len,
                                            data->power_consumption,
                                            data->wake_up_sensor,
                                            moduleId);
    } else if ((data->sensor_type == WristTiltGestureSensorType) ||
               (data->sensor_type == GlaceGestureSensorType) ||
               (data->sensor_type == WakeGestureSensorType) ||
               (data->sensor_type == PickupGestureSensorType)||
               (data->sensor_type == MotionDetectSensorType) ||
               (data->sensor_type == StationaryDetectSensorType)) {
        sensor = std::make_shared<Gesture>(&class_data,
                                           data->androidName.c_str(),
                                           sensorId, data->hw_fifo_len,
                                           data->power_consumption,
                                           moduleId);
    } else if (data->sensor_type == DeviceOrientationSensorType) {
        sensor = std::make_shared<DeviceOrientation>(&class_data,
                                                     data->androidName.c_str(),
                                                     &data->sfa, sensorId,
                                                     data->hw_fifo_len,
                                                     data->power_consumption,
                                                     data->wake_up_sensor,
                                                     moduleId);
    } else if (data->sensor_type == HumiditySensorType) {
        sensor = std::make_shared<RHumidity>(&class_data,
                                             data->androidName.c_str(), &data->sfa,
                                             sensorId, data->hw_fifo_len,
                                             data->power_consumption,
                                             data->wake_up_sensor,
                                             moduleId);
    } else if (data->sensor_type == AmbTemperatureSensorType) {
        sensor = std::make_shared<Temp>(&class_data,
                                        data->androidName.c_str(), &data->sfa,
                                        sensorId, data->hw_fifo_len,
                                        data->power_consumption,
                                        data->wake_up_sensor,
                                        moduleId);
    } else {
        return nullptr;
    }

    if (sensor->IsValidClass()) {
        ((HWSensorBase*)sensor.get())->GetSelfTestAvailable();
    }

    return sensor->IsValidClass() ? sensor : nullptr;
}

/*
 * st_hal_set_fullscale() - Change fullscale of iio device sensor
 * @iio_sysfs_path: iio device driver sysfs path.
 * @sensor_type: Android sensor type.
 * @sa: scale available structure.
 * @channels: iio channels informations.
 * @num_channels: number of iio channels.
 *
 * Return value: 0 on success, negative number on fail.
 */
static int st_hal_set_fullscale(const char *iio_sysfs_path,
                                STMSensorType sensor_type,
                                struct device_iio_scales *sa,
                                struct device_iio_info_channel *channels,
                                int num_channels)
{
    device_iio_chan_type_t iio_sensor_type;
    double max_number;
    int i, max_value;

    PropertiesManager& properties = PropertiesManager::getInstance();
    max_value = properties.getMaxRangeOfMeasurements(sensor_type);

    if (sensor_type == AccelSensorType) {
        iio_sensor_type = DEVICE_IIO_ACC;
    } else if (sensor_type == MagnSensorType) {
        iio_sensor_type = DEVICE_IIO_MAGN;
        if (max_value > 0) {
            max_value /= 100;
        }
    } else if (sensor_type == GyroSensorType) {
        iio_sensor_type = DEVICE_IIO_GYRO;
    } else if (sensor_type == AmbTemperatureSensorType) {
        /* temperature sensors generally do not support change full scale */
        return 0;
    } else {
        return -EINVAL;
    }

    if (channels[0].sign) {
        max_number = pow(2, channels[0].bits_used - 1) - 1;
    } else {
        max_number = pow(2, channels[0].bits_used) - 1;
    }

    for (i = 0; i < (int)sa->length; i++) {
        if ((sa->scales[i] * max_number) >= max_value) {
            break;
        }
    }
    if (i == (int)sa->length) {
        i = sa->length - 1;
    }

    int err = device_iio_utils::set_scale(iio_sysfs_path, sa->scales[i], iio_sensor_type);
    if (err < 0) {
        return err;
    }

    for (auto c = 0; c < num_channels - 1; c++) {
        channels[c].scale = sa->scales[i];
    }

    return 0;
}

static bool isSensorSupported(const std::string &driverName,
                              const struct SensorsSupported **sensorSupported)
{
    for (auto &sensor : sensorsSupportedList) {
        if (sensor.driver_name.compare(0, driverName.size(), driverName) == 0) {
            *sensorSupported = &sensor;
            return true;
        }
    }

    console.debug(driverName + ": IIO device not supported by sensorHAL");

    return false;
}

static int loadIIODevices(std::vector<STSensorHAL_iio_devices_data> &iioDeviceDataList)
{
    struct device_iio_type_name iio_devices[ST_HAL_IIO_MAX_DEVICES];
    struct STSensorHAL_iio_devices_data data;
    int err;

    auto len =  device_iio_utils::get_devices_name(iio_devices, ST_HAL_IIO_MAX_DEVICES);
    if (len <= 0) {
        return len;
    }

    console.debug(std::string("found ") + std::to_string(len) + " IIO devices available under /sys/bus/iio/devices");

    for (auto i = 0; i < len; i++) {
        const struct SensorsSupported *sensor;

        if (!isSensorSupported(iio_devices[i].name, &sensor)) {
            continue;
        }

        if (strcmp(&iio_devices[i].name[strlen(iio_devices[i].name) - strlen(ST_HAL_WAKEUP_SUFFIX_IIO)], ST_HAL_WAKEUP_SUFFIX_IIO) == 0) {
            data.wake_up_sensor = true;
        } else {
            data.wake_up_sensor = false;
        }

        data.iio_sysfs_path = std::string(device_iio_dir) + "iio:device" + std::to_string(iio_devices[i].num);
        data.power_consumption = sensor->power_consumption;

        err = device_iio_utils::scan_channel(data.iio_sysfs_path.c_str(), &data.channels, &data.num_channels);
        if (err < 0 && err != -ENOENT) {
            console.error(std::string(iio_devices[i].name) + ": failed to read IIO channels informations.");
            continue;
        }

        err = device_iio_utils::enable_sensor(data.iio_sysfs_path.c_str(), false);
        if (err < 0) {
            console.error(std::string(iio_devices[i].name) + ": failed to disable sensor.");
            goto st_hal_load_free_iio_channels;
        }

        device_iio_utils::set_clock_type(data.iio_sysfs_path.c_str(), "boottime");

        if ((sensor->android_sensor_type != StepDetectorSensorType) &&
            (sensor->android_sensor_type != StepCounterSensorType) &&
            (sensor->android_sensor_type != SignMotionSensorType) &&
            (sensor->android_sensor_type != TiltDetectorSensorType) &&
            (sensor->android_sensor_type != WristTiltGestureSensorType) &&
            (sensor->android_sensor_type != WakeGestureSensorType) &&
            (sensor->android_sensor_type != PickupGestureSensorType) &&
            (sensor->android_sensor_type != MotionDetectSensorType) &&
            (sensor->android_sensor_type != StationaryDetectSensorType) &&
            (sensor->android_sensor_type != DeviceOrientationSensorType) &&
            (sensor->android_sensor_type != GlaceGestureSensorType)) {
            err = device_iio_utils::get_sampling_frequency_available(data.iio_sysfs_path.c_str(), &data.sfa);
            if (err < 0) {
                console.error(std::string(iio_devices[i].name) + ": unable to get sampling frequency availability.");
                goto st_hal_load_free_iio_channels;
            }

            err = device_iio_utils::get_available_scales(data.iio_sysfs_path.c_str(), &data.sa, sensor->iio_sensor_type);
            if (err < 0)  {
                console.error(std::string(iio_devices[i].name) + ": unable to get scale availability.");
                goto st_hal_load_free_iio_channels;
            }

            if (data.sa.length > 0) {
                err = st_hal_set_fullscale(data.iio_sysfs_path.c_str(), sensor->android_sensor_type,
                                           &data.sa, data.channels, data.num_channels);
                if (err < 0) {
                    console.error(std::string(iio_devices[i].name) + ": unable to set full scale.");
                    goto st_hal_load_free_iio_channels;
                }
            }
        }

        data.deviceName = std::string(iio_devices[i].name);
        data.androidName = sensor->android_name;

        data.hw_fifo_len = device_iio_utils::get_hw_fifo_length(data.iio_sysfs_path.c_str());
        if (data.hw_fifo_len <= 0) {
            data.hw_fifo_len = 1;
        }

        data.sensor_type = sensor->android_sensor_type;
        data.dev_id = iio_devices[i].num;
        data.moduleId = device_iio_utils::get_module_id(data.iio_sysfs_path.c_str());

        iioDeviceDataList.push_back(data);

        continue;

st_hal_load_free_iio_channels:
        free(data.channels);
    }

    // if (index == 0) {
    // 	console.error("No IIO sensors found into /sys/bus/iio/devices/ folder.");
    // }

    return 0;
}

/*
 * st_hal_free_iio_devices_data() - Free iio devices data
 * @data: iio device data.
 * @num_devices: number of allocated devices.
 */
__attribute__((unused)) static void st_hal_free_iio_devices_data(STSensorHAL_iio_devices_data *data,
                                                                 unsigned int num_devices)
{
    unsigned int i;

    for (i = 0; i < num_devices; i++) {
        free(data[i].channels);
    }
}

/**
 * st_hal_dev_flush() - Flush sensor data
 * @dev: sensors device.
 * @handle: Android sensor handle.
 *
 * Return value: 0 on success, negative number on fail.
 */
int st_hal_dev_flush(void *data, uint32_t handle)
{
    STSensorHAL_data *hal_data = (STSensorHAL_data *)data;

    auto nodeId = hal_data->handleToNodeId_.find(handle);

    auto sensor = hal_data->graph[nodeId->second];
    if (sensor != nullptr) {
        return sensor->flushRequest(sensor->GetHandle(), true);
    }

    return -EINVAL;
}

/**
 * st_hal_dev_inject_sensor_data() - Sensor data injection
 * @dev: sensors device.
 * @data: sensor event data to be injected.
 *
 * Return value: 0 on success, negative number on fail.
 */
__attribute__((unused)) static int st_hal_dev_inject_sensor_data(struct sensors_poll_device_1 *dev,
                                                                 const sensors_event_t *data)
{
    STSensorHAL_data *hal_data = (STSensorHAL_data *)dev;

    auto nodeId = hal_data->handleToNodeId_.find(data->sensor);

    auto sensor = hal_data->graph[nodeId->second];
    if (sensor != nullptr) {
        return sensor->InjectSensorData(data);
    }

    return -EINVAL;
}

/**
 * st_hal_dev_batch() - Set sensor batch mode
 * @dev: sensors device structure.
 * @handle: Android sensor handle.
 * @flags: used for test the availability of batch mode.
 * @period_ns: time to batch (like setDelay(...)).
 * @timeout: 0 to disable batch mode.
 *
 * Return value: 0 on success, negative number on fail.
 */
int st_hal_dev_batch(void *data, int handle, int64_t period_ns, int64_t timeout)
{
    STSensorHAL_data *hal_data = (STSensorHAL_data *)data;

    auto nodeId = hal_data->handleToNodeId_.find(handle);

    auto sensor = hal_data->graph[nodeId->second];
    if (sensor != nullptr) {
        return sensor->SetDelay(sensor->GetHandle(), period_ns, timeout, true);
    }

    return -EINVAL;
}

static void indexRemapping(STSensorHAL_data *hal_data, struct sensors_event_t *sensorsData, int num)
{
    for (int i = 0; i < num; ++i) {
        auto itr = hal_data->sensorIdToHandle.find(sensorsData[i].sensor);
        sensorsData[i].sensor = itr->second;
    }
}

/**
 * st_hal_dev_poll() - Poll new sensors data
 * @dev: sensors device structure.
 * @data: data structure used to push data to the upper layer.
 * @count: maximum number of events in the same time.
 *
 * Return value: 0 on success, negative number on fail.
 */
int st_hal_dev_poll(void *data, sensors_event_t *sdata, int count)
{
    int err, read_size, remaining_event = count, event_read;
    STSensorHAL_data *hal_data = (STSensorHAL_data *)data;

    err = poll(hal_data->androidPollFd.data(), hal_data->androidPollFd.size(), 200);
    if (err <= 0) {
        return 0;
    }

    for (auto &fd : hal_data->androidPollFd) {
        if (fd.revents & POLLIN) {
            read_size = read(fd.fd, sdata, remaining_event * sizeof(sensors_event_t));
            if (read_size <= 0) {
                continue;
            }

            event_read = (read_size / sizeof(sensors_event_t));
            indexRemapping(hal_data, sdata, event_read);
            remaining_event -= event_read;
            sdata += event_read;

            if (remaining_event == 0) {
                return count;
            }
        } else {
            continue;
        }
    }

    return (count - remaining_event);
}

/**
 * st_hal_dev_activate() - Enable or Disable sensors
 * @dev: sensors device structure.
 * @handle: Android sensor handle.
 * @enable: enable/ disable flag.
 *
 * Return value: 0 on success, negative number on fail.
 */
int st_hal_dev_activate(void *data, uint32_t handle, bool enable)
{
    STSensorHAL_data *hal_data = (STSensorHAL_data *)data;

    auto nodeId = hal_data->handleToNodeId_.find(handle);

    auto sensor = hal_data->graph[nodeId->second];
    if (sensor != nullptr) {
        return sensor->Enable(sensor->GetHandle(), enable, true);
    }

    return -EINVAL;
}

void st_hal_close_sensors(void *pdata)
{
    STSensorHAL_data *hal_data = (STSensorHAL_data *)pdata;

    delete(hal_data);
}

/**
 * open_sensors() - Open sensor device
 * see Android documentation.
 *
 * Return value: 0 on success, negative number on fail.
 */
int st_hal_open_sensors(void **pdata, STMSensorsList &sensorsList)
{
    std::vector<STSensorHAL_iio_devices_data> iioDataList;
    unsigned int internalSensorId = 4;

    *pdata = new STSensorHAL_data();
    if (!*pdata) {
        *pdata = nullptr;
        return -ENOMEM;
    }

    STSensorHAL_data *hal_data = (STSensorHAL_data *)*pdata;

    int deviceFoundNum = loadIIODevices(iioDataList);
    if (deviceFoundNum < 0) {
        console.error("Failed to read IIO sensors");
        free(*pdata);
        *pdata = nullptr;
        return deviceFoundNum;
    }
    if (iioDataList.size() == 0) {
        console.error("No IIO sensors found!");
        free(*pdata);
        *pdata = nullptr;
        return -ENODEV;
    }

    std::unordered_set<int> modulesIdAvailable;

    for (auto i = 0U; i < iioDataList.size(); ++i) {
        for (auto j = i + 1; j < iioDataList.size(); ++j) {
            if ((iioDataList[i].moduleId == iioDataList[j].moduleId) &&
                (iioDataList[i].sensor_type == iioDataList[j].sensor_type)) {
                    console.warning("detected two sensors of the same type in the same module (may not work correctly)!");
            }
        }
    }

    std::sort(iioDataList.begin(), iioDataList.end(),
              [] (const auto& lhs, const auto& rhs) {
                  return lhs.moduleId > rhs.moduleId;
              });

    for (auto &iioDeviceData : iioDataList) {
        std::shared_ptr<SensorBase> sensor = st_hal_create_class_sensor(&iioDeviceData,
                                                                        internalSensorId,
                                                                        iioDeviceData.moduleId);
        if (sensor == nullptr) {
            console.error(iioDeviceData.deviceName + ": failed to create HW sensor class.");
            continue;
        }

        if (!sensor->libsInit() && sensor->IsValidClass()) {
            modulesIdAvailable.insert(iioDeviceData.moduleId);
            hal_data->graph.addNode(sensor->GetHandle(), sensor);
            internalSensorId++;
        }
    }
    if (hal_data->graph.empty()) {
        console.error("no hardware sensors!");
        free(*pdata);
        *pdata = nullptr;
        return -ENODEV;
    }

    for (auto& moduleId : modulesIdAvailable) {
        for (auto &virtualSensor : sensorsSWSupportedList) {
            std::shared_ptr<SensorBase> sensor = st_hal_create_virtual_class_sensor(virtualSensor.type,
                                                                                    internalSensorId,
                                                                                    moduleId);
            if (sensor == nullptr) {
                console.error(": failed to create SW sensor class.");
                continue;
            }

            if (!sensor->libsInit() && sensor->IsValidClass()) {
                hal_data->graph.addNode(sensor->GetHandle(), sensor);
                internalSensorId++;
            }
        }
    }

    std::vector<int> nodesToRemove;

    for (auto& sensor : hal_data->graph) {
        auto s = sensor.second.payload;
        auto &listDependenciesType = s->GetDepenciesTypeList();
        bool validDependencies = true;

        for (auto dependecyType : listDependenciesType) {
            bool valid = false;

            for (auto &sensorDependecy : hal_data->graph) {
                auto sDependency = sensorDependecy.second.payload;

                if ((sDependency->GetType() == dependecyType) &&
                    (sDependency->getModuleId() == s->getModuleId())) {
                    hal_data->graph.addEdge(s->GetHandle(), sDependency->GetHandle());
                    valid = true;
                    break;
                }
            }
            if (!valid) {
                validDependencies = false;
            }
        }
        if (!validDependencies) {
            nodesToRemove.push_back(s->GetHandle());
        }
    }

    for (auto &id : nodesToRemove) {
        hal_data->graph.removeNodeAnd(id);
    }

    auto sortedNodesToVisit = hal_data->graph.getTopologicalSortReverse();
    for (auto& dependencyNodeId : sortedNodesToVisit) {
        for (auto& nodeId : hal_data->graph.getArrivingNodesId(dependencyNodeId)) {
            hal_data->graph[nodeId]->AddSensorDependency(hal_data->graph[dependencyNodeId].get());
        }
    }

    for (auto &node : hal_data->graph) {
        struct sensor_t sensorData = node.second.payload->GetSensor_tData();
        if (sensorData.type.isInternal()) {
            continue;
        }

        auto sensor = std::make_unique<STMSensor>(sensorData.name,
                                                  sensorData.vendor,
                                                  1,
                                                  sensorData.type,
                                                  sensorData.maxRange,
                                                  sensorData.resolution,
                                                  sensorData.power,
                                                  sensorData.minRateHz,
                                                  sensorData.maxRateHz,
                                                  sensorData.fifoRsvdCount,
                                                  sensorData.fifoMaxEventCount,
                                                  false);

        if (!sensorsList.addSensor(*sensor)) {
            console.error("sensor added is not valid");
            continue;
        }

        hal_data->handleToNodeId_.insert(std::pair<uint32_t, int>(sensor->getHandle(), node.first));
        hal_data->sensorIdToHandle.insert(std::pair<int, uint32_t>(node.first, sensor->getHandle()));

        struct pollfd sensorPollFd;
        sensorPollFd.events = POLLIN;
        sensorPollFd.fd = node.second.payload->GetFdPipeToRead();

        hal_data->androidPollFd.push_back(sensorPollFd);
    }

    for (auto &node : hal_data->graph) {
        node.second.payload->startThreads();
    }

    hal_data->selfTest = std::make_shared<SelfTest>(hal_data);
    if (!hal_data->selfTest->IsValidClass()) {
        console.error("selftest functions cannot be loaded correctly");
    }

    console.debug(std::to_string(sensorsList.getList().size()) + " sensors available and ready");

    return 0;
}

static void st_hal_print_timesync_version()
{
    std::string libVersionMsg { "timesync library: " };

    if (HAL_ENABLE_TIMESYNC != 0) {
        libVersionMsg += STMTimesync::getLibVersion();
        console.info(libVersionMsg);
    } else {
        libVersionMsg += std::string("not enabled!");
        console.info(libVersionMsg);
    }
}

void st_hal_dev_set_callbacks(void *data, const ISTMSensorsCallback &sensorsCallback)
{
    STSensorHAL_data *hal_data = (STSensorHAL_data *)data;

    for (auto &node : hal_data->graph) {
        node.second.payload->setCallbacks(sensorsCallback);
    }

    st_hal_print_timesync_version();
}

void st_hal_dev_post_setup(void *data)
{
    STSensorHAL_data *hal_data = (STSensorHAL_data *)data;

    for (auto &node : hal_data->graph) {
        node.second.payload->postSetup();
    }
}

/**
 * st_hal_set_operation_mode() - Set HAL mode
 * @mode: HAL mode.
 *
 * Return value: 0 (supported), negative number if error.
 */
__attribute__((unused)) static int st_hal_set_operation_mode(unsigned int mode)
{
	(void) mode;
// 	int err;
// 	bool enable_injection = false;
// 	STSensorHAL_data *hal_data = (STSensorHAL_data *)&mode;

// 	switch (mode) {
// 	case SENSOR_HAL_NORMAL_MODE:
// 		enable_injection = false;
// 		break;

// 	case SENSOR_HAL_DATA_INJECTION_MODE:
// 		enable_injection = true;
// 		break;

// 	default:
// 		return -EPERM;
// 	}

// 	for (auto sensor : hal_data->sensorList.getList()) {
// 		// err = hal_data->sensor_classes[sensor.getHandle()]->InjectionMode(enable_injection);
// 		if (err < 0) {
// 			console.error("Failed to set HAL operation mode.");
// 			goto rollback_operation_mode;
// 		}
// 	}

// 	return 0;

// rollback_operation_mode:
// 	for (auto sensor : hal_data->sensorList.getList()) {
// 		// err = hal_data->sensor_classes[sensor.getHandle()]->InjectionMode(!enable_injection);
// 		if (err < 0) {
// 			goto rollback_operation_mode;
// 		}
// 	}

	return -EINVAL;
}

} // namespace core
} // namespace stm
