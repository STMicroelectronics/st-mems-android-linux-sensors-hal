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

#include <cstring>
#include <STMSensorsList.h>

namespace stm {
namespace core {

/**
 * addSensor: add new sensor to the list
 * @sensor: sensor reference to add
 *
 * Return value: true if added successfully, false in case of error.
 */
bool STMSensorsList::addSensor(STMSensor &sensor)
{
    if (sensor.getMaxRange() == 0) {
        return false;
    }

    if ((sensor.getMinRateHz() < 0) || (sensor.getMaxRateHz() < sensor.getMinRateHz())) {
        return false;
    }

    if (sensor.getFifoMaxCount() < sensor.getFifoRsvdCount()) {
        return false;
    }

    for (auto &elem : list) {
        if (elem.getType() == sensor.getType()) {
            if ((std::strcmp(elem.getName().c_str(), sensor.getName().c_str()) == 0) &&
                elem.getModuleId() == sensor.getModuleId()) {
                return false;
            }
        }
    }

    sensor.setHandle(list.size() + 1);
    list.push_back(sensor);

    return true;
}

/**
 * getList: return current sensors list
 *
 * Return value: current sensors list reference.
 */
const std::vector<STMSensor>& STMSensorsList::getList(void) const
{
    return list;
}

void STMSensorsList::clear()
{
    list.clear();
}

} // namespace core
} // namespace stm
