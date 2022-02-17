/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2022 STMicroelectronics
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

#include <string>

struct STMTimesync {
    STMTimesync(void) = default;
    ~STMTimesync(void) = default;
    STMTimesync(const STMTimesync &) = delete;
    STMTimesync(STMTimesync &&) = delete;
    STMTimesync& operator=(const STMTimesync &) = delete;
    STMTimesync& operator=(STMTimesync &&) = delete;

    int init(uint8_t maxPoints);

    void reset(void);

    bool add(int64_t timestamp1, int64_t timestamp2);

    bool estimate(int64_t timestamp2, int64_t& timestamp1);

    static const std::string& getLibVersion(void);

private:
    int64_t lastTimestamp1;
    int64_t lastTimestamp2;
};
