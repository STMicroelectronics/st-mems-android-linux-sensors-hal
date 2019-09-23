/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019 STMicroelectronics
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

#define LOG_TAG "android.hardware.sensors@2.0-service"

#include <android/hardware/sensors/2.0/ISensors.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/StrongPointer.h>
#include <log/log.h>

#include "SensorsHidlInterface.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::sensors::V2_0::ISensors;
using android::hardware::sensors::V2_0::implementation::SensorsHidlInterface;

int main(int /* argc */, char** /* argv */)
{
    android::status_t err;

    configureRpcThreadpool(1, true);

    android::sp<ISensors> sensors = new SensorsHidlInterface();
    err = sensors->registerAsService();
    if (err != android::OK) {
        ALOGE("Failed to register STM Sensors-HAL instance");
        return err;
    }

    joinRpcThreadpool();

    return 1;
}
