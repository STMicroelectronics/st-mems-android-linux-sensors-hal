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

#include "ChangeODRTimestampStack.h"

ChangeODRTimestampStack::ChangeODRTimestampStack()
{
    pthread_mutex_init(&data_mutex, NULL);
    elements_available = 0;
}

int ChangeODRTimestampStack::writeElement(int64_t timestamp, int64_t newpollrate)
{
    pthread_mutex_lock(&data_mutex);

    if (elements_available >= ST_ODR_STACK_MAX_ELEMENTS) {
        pthread_mutex_unlock(&data_mutex);
        return -ENOMEM;
    }

    timestamps[elements_available] = timestamp;
    new_pollrate[elements_available] = newpollrate;
    elements_available++;

    pthread_mutex_unlock(&data_mutex);

    return 0;
}

int64_t ChangeODRTimestampStack::readLastElement(int64_t *newpollrate)
{
    int64_t latest_change_odr_timestamp;

    pthread_mutex_lock(&data_mutex);

    if (elements_available == 0) {
        pthread_mutex_unlock(&data_mutex);
        return (int64_t)(int)(-EIO);
    }

    latest_change_odr_timestamp = timestamps[0];
    *newpollrate = new_pollrate[0];

    pthread_mutex_unlock(&data_mutex);

    return latest_change_odr_timestamp;
}

void ChangeODRTimestampStack::removeLastElement()
{
    pthread_mutex_lock(&data_mutex);

    if (elements_available == 0) {
        pthread_mutex_unlock(&data_mutex);
        return;
    }

    for (auto i = 0U; i < elements_available - 1; i++) {
        timestamps[i] = timestamps[i + 1];
        new_pollrate[i] = new_pollrate[i + 1];
    }

    elements_available--;

    pthread_mutex_unlock(&data_mutex);
}

void ChangeODRTimestampStack::resetBuffer()
{
    pthread_mutex_lock(&data_mutex);

    elements_available = 0;

    pthread_mutex_unlock(&data_mutex);
}
