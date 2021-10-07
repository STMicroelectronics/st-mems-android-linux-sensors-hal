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

#include <cstdint>
#include <string>

#include <ISTMSensorsCallbackData.h>

namespace stm {
namespace core {

class ISTMSensorsCallback {
public:
    virtual ~ISTMSensorsCallback(void) = default;

    /**
     * onNewSensorsData: called whenever new data are available for consumers
     * @sensorsData: object reference containing all the data.
     */
    virtual void onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData) = 0;

    /**
     * onSaveDataRequest: called whenever a resource needs to be written to disk
     * @resourceID: identifier of the resource.
     * @data: data pointer.
     * @len: data buffer size.
     *
     * Return value: number of bytes written on success, else a negative error code.
     */
    virtual int onSaveDataRequest(const std::string& resourceID, const void *data, ssize_t len) = 0;

    /**
     * onLoadDataRequest: called whenever a resource needs to be loaded from disk
     * @resourceID: identifier of the resource.
     * @data: data pointer.
     * @len: data buffer size.
     *
     * Return value: number of bytes read on success, else a negative error code.
     */
    virtual int onLoadDataRequest(const std::string& resourceID, void *data, ssize_t len) = 0;
};

} // namespace core
} // namespace stm
