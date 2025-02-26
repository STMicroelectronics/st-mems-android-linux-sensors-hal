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

#include <STMSensor.h>

using stm::core::STMSensor;
using stm::core::SensorType;

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
    // Test the first constructor
    STMSensor accel(std::string("name"), // name
                    std::string("vendor"), // vendor
                    2, // version
                    SensorType::ACCELEROMETER, // type
                    10, //maxRange
                    0.2, // resolution
                    1.3, // power
                    1, // minRateHz
                    414, // maxRateHz
                    5, // fifoRsvdCount
                    220, // fifoMaxCount
                    false, // wakeUp
                    0 // moduleId
                    );

    ASSERT_STREQ("name", accel.getName().c_str());
    ASSERT_STREQ("vendor", accel.getVendor().c_str());
    ASSERT_EQ(0, accel.getHandle());
    ASSERT_EQ(2, accel.getVersion());
    ASSERT_EQ(SensorType::ACCELEROMETER, accel.getType());
    ASSERT_FLOAT_EQ(10, accel.getMaxRange());
    ASSERT_FLOAT_EQ(0.2, accel.getResolution());
    ASSERT_FLOAT_EQ(1.3, accel.getPower());
    ASSERT_FLOAT_EQ(1, accel.getMinRateHz());
    ASSERT_FLOAT_EQ(414, accel.getMaxRateHz());
    ASSERT_EQ(5, accel.getFifoRsvdCount());
    ASSERT_EQ(220, accel.getFifoMaxCount());
    ASSERT_FALSE(accel.isWakeUp());

    // Test the second constructor
    STMSensor signMotion(std::string("name_2"), // name
                         std::string("vendor_2"), // vendor
                         1, // version
                         SensorType::SIGNIFICANT_MOTION, // type
                         1, //maxRange
                         1, // resolution
                         1.0, // power
                         0, // fifoRsvdCount
                         0, // fifoMaxCount
                         true, // wakeUp
                         0 // moduleId
                         );

    ASSERT_STREQ("name_2", signMotion.getName().c_str());
    ASSERT_STREQ("vendor_2", signMotion.getVendor().c_str());
    ASSERT_EQ(0, signMotion.getHandle());
    ASSERT_EQ(1, signMotion.getVersion());
    ASSERT_EQ(SensorType::SIGNIFICANT_MOTION, signMotion.getType());
    ASSERT_FLOAT_EQ(1, signMotion.getMaxRange());
    ASSERT_FLOAT_EQ(1, signMotion.getResolution());
    ASSERT_FLOAT_EQ(1, signMotion.getPower());
    ASSERT_FLOAT_EQ(0, signMotion.getMinRateHz());
    ASSERT_FLOAT_EQ(0, signMotion.getMaxRateHz());
    ASSERT_EQ(0, signMotion.getFifoRsvdCount());
    ASSERT_EQ(0, signMotion.getFifoMaxCount());
    ASSERT_TRUE(signMotion.isWakeUp());
}
