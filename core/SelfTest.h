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

namespace stm {
namespace core {

typedef enum {
    NOT_AVAILABLE = 0,
    GENERIC_ERROR,
    FAILURE,
    PASS
} selftest_status;

struct selftest_cmd_t {
    int handle;
    short mode;
};

struct selftest_results_t {
    int handle;
    selftest_status status;
};

/*
 * class SelfTest
 */
class SelfTest {
private:
    struct STSensorHAL_data *hal_data;
    bool valid_class;
    int fd_cmd, fd_results;
    pthread_t cmd_thread;

    static void *ThreadCmdWork(void *context);
    void ThreadCmdTask();

public:
    SelfTest(struct STSensorHAL_data *ST_hal_data);
    virtual ~SelfTest();

    bool IsValidClass();
};

} // namespace core
} // namespace stm
