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

#include <STMSensorType.h>

namespace stm {
namespace core {

STMSensorType::STMSensorType(SensorType type)
    : isInternalType(false), sType(type)
{

}

STMSensorType::STMSensorType(SensorTypeInternal type)
    : isInternalType(true), sTypeInt(type)
{

}

bool STMSensorType::isInternal(void) const
{
    return isInternalType;
}

bool STMSensorType::operator==(const STMSensorType &other) const
{
    if (other.isInternalType == isInternalType) {
        if (isInternalType) {
            return other.sTypeInt == sTypeInt;
        } else {
            return other.sType == sType;
        }
    }

    return false;
}

bool STMSensorType::operator!=(const STMSensorType &other) const
{
    return !(*this == other);
}

} // namespace core
} // namespace stm
