/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2019 STMicroelectronics
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

#include <STMSensor.h>

class STMSensorTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

/**
 * verifyParameters: verify that given parameters are returned correctly
 */
TEST_F(STMSensorTest, verifyParameters)
{
    STMSensor accel(std::string("name"), // name
                    std::string("vendor"), // vendor
                    1, // handle
                    2, // version
                    STMSensorType::ACCELEROMETER, // type
                    10, //maxRange
                    0.2, // resolution
                    1.3, // power
                    1, // minRateHz
                    414, // maxRateHz
                    5, // fifoRsvdCount
                    220, // fifoMaxCount
                    false // wakeUp
                    );

    ASSERT_STREQ("name", accel.getName().c_str());
    ASSERT_STREQ("vendor", accel.getVendor().c_str());
    ASSERT_EQ(1, accel.getHandle());
    ASSERT_EQ(2, accel.getVersion());
    ASSERT_EQ(STMSensorType::ACCELEROMETER, accel.getType());
    ASSERT_FLOAT_EQ(10, accel.getMaxRange());
    ASSERT_FLOAT_EQ(0.2, accel.getResolution());
    ASSERT_FLOAT_EQ(1.3, accel.getPower());
    ASSERT_FLOAT_EQ(1, accel.getMinRateHz());
    ASSERT_FLOAT_EQ(414, accel.getMaxRateHz());
    ASSERT_EQ(5, accel.getFifoRsvdCount());
    ASSERT_EQ(220, accel.getFifoMaxCount());
    ASSERT_FALSE(accel.isWakeUp());
}
