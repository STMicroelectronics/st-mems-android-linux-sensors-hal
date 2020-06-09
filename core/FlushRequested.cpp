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

#include "FlushRequested.h"

FlushRequested::FlushRequested()
{
    pthread_mutex_init(&data_mutex, NULL);

    elements_available = 0;
}

int FlushRequested::writeElement(int handle)
{
    pthread_mutex_lock(&data_mutex);

    if (elements_available >= ST_FLUSH_REQUESTED_STACK_MAX_ELEMENTS) {
        pthread_mutex_unlock(&data_mutex);
        return -ENOMEM;
    }

    handles[elements_available] = handle;
    elements_available++;

    pthread_mutex_unlock(&data_mutex);

    return 0;
}

int FlushRequested::readElement()
{
    int handle;

    pthread_mutex_lock(&data_mutex);

    if (elements_available == 0) {
        pthread_mutex_unlock(&data_mutex);
        return -EIO;
    }

    handle = handles[0];

    for (auto i = 0U; i < elements_available - 1; i++) {
        handles[i] = handles[i + 1];
    }

    elements_available--;

    pthread_mutex_unlock(&data_mutex);

    return handle;
}

void FlushRequested::resetBuffer()
{
    pthread_mutex_lock(&data_mutex);

    elements_available = 0;

    pthread_mutex_unlock(&data_mutex);
}
