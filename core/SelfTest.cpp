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

#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <poll.h>
#include <sys/file.h>

#include "SensorHAL.h"
#include "common_data.h"
#include "SelfTest.h"

namespace stm {
namespace core {

SelfTest::SelfTest(struct STSensorHAL_data *ST_hal_data)
{
    int err;

    valid_class = false;
    hal_data = ST_hal_data;

    mkdir(ST_HAL_SELFTEST_DATA_PATH, S_IRWXU);

    remove(ST_HAL_SELFTEST_CMD_DATA_PATH);
    remove(ST_HAL_SELFTEST_RESULTS_DATA_PATH);

    err = mkfifo(ST_HAL_SELFTEST_CMD_DATA_PATH, S_IRWXU);
    if ((err < 0) && (errno != EEXIST)) {
        return;
    }

    err = mkfifo(ST_HAL_SELFTEST_RESULTS_DATA_PATH, S_IRWXU);
    if ((err < 0) && (errno != EEXIST)) {
        remove(ST_HAL_SELFTEST_CMD_DATA_PATH);
        return;
    }

    fd_cmd = open(ST_HAL_SELFTEST_CMD_DATA_PATH, O_RDWR | O_NONBLOCK);
    if (fd_cmd < 0) {
        remove(ST_HAL_SELFTEST_CMD_DATA_PATH);
        remove(ST_HAL_SELFTEST_RESULTS_DATA_PATH);
        return;
    }

    fd_results = open(ST_HAL_SELFTEST_RESULTS_DATA_PATH, O_RDWR | O_NONBLOCK);
    if (fd_results < 0) {
        close(fd_cmd);
        remove(ST_HAL_SELFTEST_CMD_DATA_PATH);
        remove(ST_HAL_SELFTEST_RESULTS_DATA_PATH);
        return;
    }

    pthread_create(&cmd_thread, NULL, &SelfTest::ThreadCmdWork, (void *)this);

    valid_class = true;
}

SelfTest::~SelfTest()
{
    if (!valid_class) {
        return;
    }

    close(fd_cmd);
    close(fd_results);

    remove(ST_HAL_SELFTEST_CMD_DATA_PATH);
    remove(ST_HAL_SELFTEST_RESULTS_DATA_PATH);
}

bool SelfTest::IsValidClass()
{
return valid_class;
}

void *SelfTest::ThreadCmdWork(void *context)
{
    SelfTest *mypointer = (SelfTest *)context;

    mypointer->ThreadCmdTask();

    return mypointer;
}

void SelfTest::ThreadCmdTask()
{
    struct pollfd fds;
    int err, read_size;
    struct selftest_cmd_t cmd_data;
    struct selftest_results_t results_data;

    fds.fd = fd_cmd;
    fds.events = POLLIN;

    while (1) {
        err = poll(&fds, 1, -1);
        if (err <= 0) {
            continue;
        }

        if (fds.revents & POLLIN) {
            read_size = read(fds.fd, &cmd_data, sizeof(struct selftest_cmd_t));
            if (read_size != sizeof(struct selftest_cmd_t)) {
                continue;
            }

            if ((cmd_data.handle == 0) || (cmd_data.handle > (int)hal_data->handleToNodeId_.size())) {
                return;
            }
            auto nodeId = hal_data->handleToNodeId_.find(cmd_data.handle);

            results_data.status = NOT_AVAILABLE;

            for (auto &node : hal_data->graph) {
                if (nodeId->second == cmd_data.handle) {
                    results_data.status = node.payload->ExecuteSelfTest();
                }
            }

            results_data.handle = cmd_data.handle;
            write(fd_results, &results_data, sizeof(struct selftest_results_t));
        }

    }

    return;
}

} // namespace core
} // namespace stm
