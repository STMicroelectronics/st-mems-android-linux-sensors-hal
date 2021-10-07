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

#include <gtest/gtest.h>

#include <STMSensorsHAL.h>
#include <IConsole.h>
#include <IUtils.h>

using stm::core::IUtils;
using stm::core::IConsole;
using stm::core::ISTMSensorsHAL;
using stm::core::STMSensorsHAL;
using stm::core::ISTMSensorsCallback;
using stm::core::ISTMSensorsCallbackData;

class STMSensorsCallback : public ISTMSensorsCallback {
    void onNewSensorsData(const std::vector<ISTMSensorsCallbackData> &sensorsData) override
    {
        (void) sensorsData;
    }
};

class STMSensorsHALTest : public ::testing::Test {
public:
    const STMSensorsCallback sensorsCallback;
    STMSensorsHAL &hal = STMSensorsHAL::getInstance();

protected:
    void SetUp() override {
        hal.initialize(sensorsCallback);
    }

    void TearDown() override {
    }
};

class Console : public IConsole {
    void info(const std::string &message) const override
    {
        (void) message;
    }

    void error(const std::string &message) const override
    {
        (void) message;
    }

    void debug(const std::string &message) const override
    {
        (void) message;
    }
};

IConsole& IConsole::getInstance(void)
{
    static Console instance;

    return instance;
}

class Utils : public IUtils {
public:
    int64_t getTime(void) const override;
};

int64_t Utils::getTime(void) const
{
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts)) {
      return 0;
    }

    return (ts.tv_sec * 1000000000) + ts.tv_nsec;
}

IUtils& IUtils::getInstance(void)
{
    static Utils instance;

    return instance;
}

/**
 * invalidHandle: verify that functions fail if sensor handle is not valid
 */
TEST_F(STMSensorsHALTest, invalidHandle)
{
    // Verify that handle 0 is not valid and library return negative value
    ASSERT_LT(hal.activate(0, true), 0);
    ASSERT_LT(hal.activate(0, false), 0);
    ASSERT_LT(hal.setRate(0, 0, 0), 0);
    ASSERT_LT(hal.setRate(0, 1, 100), 0);
    ASSERT_LT(hal.flushData(0), 0);

    auto lastValidHandle = hal.getSensorsList().getList().size();

    // Sensors handles must be consecutive, handles > lastValidHandle are not valid and library must return negative value
    ASSERT_LT(hal.activate(lastValidHandle + 1, true), 0);
    ASSERT_LT(hal.activate(lastValidHandle + 1, false), 0);
    ASSERT_LT(hal.setRate(lastValidHandle + 1, 0, 0), 0);
    ASSERT_LT(hal.setRate(lastValidHandle + 1, 1, 100), 0);
    ASSERT_LT(hal.flushData(lastValidHandle + 1), 0);
}
