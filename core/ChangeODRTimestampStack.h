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

#define ST_ODR_STACK_MAX_ELEMENTS       (20)

/*
 * class ChangeODRTimestampStack
 */
class ChangeODRTimestampStack {
private:
    pthread_mutex_t data_mutex;
    unsigned int elements_available;

    int64_t new_pollrate[ST_ODR_STACK_MAX_ELEMENTS];
    int64_t timestamps[ST_ODR_STACK_MAX_ELEMENTS];

public:
    ChangeODRTimestampStack();

    int writeElement(int64_t timestamp, int64_t newpollrate);
    int64_t readLastElement(int64_t *newpollrate);
    void removeLastElement(void);
    void resetBuffer();
};
