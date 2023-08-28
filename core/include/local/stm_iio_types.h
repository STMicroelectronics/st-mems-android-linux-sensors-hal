/*
 * STMicroelectronics IIO default types
 *
 * MEMS Software Solutions Team
 *
 * Copyright 2023 STMicroelectronics Inc.
 */

#ifndef __STM_IIO_DEFAULT_TYPE__
#define __STM_IIO_DEFAULT_TYPE__

/*
 * IIO default types doesn't introduce any custom define:
 * currently defining default standard IIO defines only, all won't support
 * STM CUSTOM IIO defines.
 *
 * Part of the IIO types that SensorHAL uses are already defined in the types.h
 * file of the IIO UAPI interface, while others are custom and are exported by
 * the STMicroelectrinics IIO drivers from the repository:
 * https://github.com/STMicroelectronics/st-mems-android-linux-drivers-iio
 *
 * This file introduces the define __STM_IIO_DEFAULT_TYPE__ which leads in the
 * sensorhal_iio_types.h file to the declaration of the custom types of sensors
 * used in the HAL backwards compatible with the previous management.
 *
 * When this file is replaced with the file exported from:
 * https://github.com/STMicroelectronics/st-mems-android-linux-drivers-iio/drivers/iio/stm/common/stm_iio_types.h
 * it introduces custom sensor type to starting support the kernel 6.1 for the new
 * drivers.
 */

#endif /* __STM_IIO_DEFAULT_TYPE__ */

