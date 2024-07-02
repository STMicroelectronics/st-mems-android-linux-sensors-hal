/*
 * STMicroelectronics IIO default types
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

