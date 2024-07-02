/*
 * STMicroelectronics SensorHAL IIO sensor types
 *
 * MEMS Software Solutions Team
 *
 * Copyright 2023 STMicroelectronics Inc.
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

#ifndef __SENSORHAL_IIO_TYPE__
#define __SENSORHAL_IIO_TYPE__

#include <local/stm_iio_types.h>

#ifdef __STM_IIO_CUSTOM_TYPE__

#include <local/types.h>

/* mapping sensor hal channel type to IIO driver */
typedef enum {
    /* IIO standard types */
    DEVICE_IIO_ACC = IIO_ACCEL,
    DEVICE_IIO_GYRO = IIO_ANGL_VEL,
    DEVICE_IIO_MAGN = IIO_MAGN,
    DEVICE_IIO_TEMP = IIO_TEMP,
    DEVICE_IIO_PRESSURE = IIO_PRESSURE,
    DEVICE_IIO_HUMIDITYRELATIVE = IIO_HUMIDITYRELATIVE,
    DEVICE_IIO_STEP_DETECTOR = IIO_STEPS,

    /* IIO custom types */
    DEVICE_IIO_SIGN_MOTION = STM_IIO_SIGN_MOTION,
    DEVICE_IIO_STEP_COUNTER = STM_IIO_STEP_COUNTER,
} device_iio_chan_type_t;

/* mapping sensor hal event type/dir to IIO driver */
#define DEVICE_IIO_EV_DIR_FIFO_DATA	STM_IIO_EV_DIR_FIFO_DATA
#define DEVICE_IIO_EV_TYPE_FIFO_FLUSH	STM_IIO_EV_TYPE_FIFO_FLUSH
#define DEVICE_IIO_EV_TYPE_TIME_SYNC	STM_IIO_EV_TYPE_TIME_SYNC

#else /* __STM_IIO_CUSTOM_TYPE__ */

/* mapping sensor hal channel type to IIO driver */
typedef enum {
    /* IIO standard types */
    DEVICE_IIO_ACC = 3,
    DEVICE_IIO_GYRO = 4,
    DEVICE_IIO_MAGN = 5,
    DEVICE_IIO_TEMP = 9,
    DEVICE_IIO_PRESSURE = 17,
    DEVICE_IIO_HUMIDITYRELATIVE = 18,
    DEVICE_IIO_STEP_DETECTOR = 20,

    /* IIO custom types */
    DEVICE_IIO_SIGN_MOTION = 29,
    DEVICE_IIO_STEP_COUNTER = 31,
} device_iio_chan_type_t;

/* mapping sensor hal event type/dir to IIO driver */
#define DEVICE_IIO_EV_DIR_FIFO_DATA	0x05
#define DEVICE_IIO_EV_TYPE_FIFO_FLUSH	0x06
#define DEVICE_IIO_EV_TYPE_TIME_SYNC	0x07

#endif /* __STM_IIO_CUSTOM_TYPE__ */

#endif /* __SENSORHAL_IIO_TYPE__ */

