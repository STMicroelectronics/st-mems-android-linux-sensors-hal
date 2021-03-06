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

#pragma once

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <array>

typedef struct SensorBaseData {
    float raw[4];
    float offset[4];
    float processed[5];
    int64_t timestamp;
    int64_t hwTimestamp;
    int8_t accuracy;
    std::array<int, 10> flushEventHandles;
    int flushEventsNum;
    int64_t pollrate_ns;
    bool hasHwTimestamp = false;
} SensorBaseData;

/*
 * class CircularBuffer
 */
class CircularBuffer {
private:
    pthread_mutex_t data_mutex;
    unsigned int length, elements_available;

    SensorBaseData *data_sensor;
    SensorBaseData *first_available_element;
    SensorBaseData *first_free_element;

public:
    CircularBuffer(unsigned int num_elements);
    ~CircularBuffer();

    int writeElement(SensorBaseData *data);
    int readElement(SensorBaseData *data);
    int readSyncElement(SensorBaseData *data, int64_t timestamp_sync);
    void resetBuffer();
};
