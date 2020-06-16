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

#include <STMSensorsList.h>

using stm::core::STMSensor;
using stm::core::STMSensorType;
using stm::core::STMSensorsList;

STMSensor accel(std::string("name"), // name
                std::string("vendor"), // vendor
                1, // version
                STMSensorType::ACCELEROMETER, // type
                1, //maxRange
                1, // resolution
                1, // power
                1, // minRateHz
                10, // maxRateHz
                0, // fifoRsvdCount
                0, // fifoMaxCount
                false // wakeUp
                );

STMSensor accelName2(std::string("name_2"), // name
                     std::string("vendor"), // vendor
                     1, // version
                     STMSensorType::ACCELEROMETER, // type
                     1, //maxRange
                     1, // resolution
                     1, // power
                     1, // minRateHz
                     10, // maxRateHz
                     0, // fifoRsvdCount
                     0, // fifoMaxCount
                     false // wakeUp
                     );

STMSensor magnMinRateHzNegative(std::string("name"), // name
                                std::string("vendor"), // vendor
                                1, // version
                                STMSensorType::MAGNETOMETER, // type
                                1, //maxRange
                                1, // resolution
                                1, // power
                                -7, // minRateHz
                                10, // maxRateHz
                                0, // fifoRsvdCount
                                0, // fifoMaxCount
                                false // wakeUp
                                );

STMSensor magnMinRateHzBigger(std::string("name"), // name
                              std::string("vendor"), // vendor
                              1, // version
                              STMSensorType::MAGNETOMETER, // type
                              1, //maxRange
                              1, // resolution
                              1, // power
                              30, // minRateHz
                              10, // maxRateHz
                              0, // fifoRsvdCount
                              0, // fifoMaxCount
                              false // wakeUp
                              );

class STMSensorsListTest : public ::testing::Test {
public:
    STMSensorsList sensorsList;

protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

/**
 * addSensor_validSensor: verify that a valid sensor is added correctly
 */
TEST_F(STMSensorsListTest, addSensor_validSensor)
{
    ASSERT_TRUE(sensorsList.addSensor(accel));
    ASSERT_EQ(1, sensorsList.getList().size());
}

/**
 * addSensor_rateHzCheck: verify *rateHz parameters
 * - rates cannot be negative
 * - maxRateHz must be greater or equal than minRateHz
 */
TEST_F(STMSensorsListTest, addSensor_rateHzCheck)
{
    // Adding a sensor with negative rate must fail
    ASSERT_FALSE(sensorsList.addSensor(magnMinRateHzNegative));
    ASSERT_EQ(0, sensorsList.getList().size());

    // Adding a sensor with minRate bigger than maxRate must fail
    ASSERT_FALSE(sensorsList.addSensor(magnMinRateHzBigger));
    ASSERT_EQ(0, sensorsList.getList().size());
}

/**
 * addSensor_sameSensorType: verify that adding same sensor type respect rules
 * - same sensor type must have different name
 */
TEST_F(STMSensorsListTest, addSensor_sameSensorType)
{
    ASSERT_TRUE(sensorsList.addSensor(accel));
    ASSERT_EQ(1, sensorsList.getList().size());

    // Adding a sensor with same type and same name must fail
    ASSERT_FALSE(sensorsList.addSensor(accel));
    ASSERT_EQ(1, sensorsList.getList().size());

    // Adding a sensor with same type but different name should succeed
    ASSERT_TRUE(sensorsList.addSensor(accelName2));
    ASSERT_EQ(2, sensorsList.getList().size());
}

/**
 * addSensor_maxRangeNotZero: verify that adding a sensor with maxRange equal to zero fails
 */
TEST_F(STMSensorsListTest, addSensor_maxRangeNotZero)
{
    STMSensor stepCounterMaxRangeZero(std::string("name"), // name
                                      std::string("vendor"), // vendor
                                      1, // version
                                      STMSensorType::STEP_COUNTER, // type
                                      0, //maxRange
                                      1, // resolution
                                      1, // power
                                      0, // minRateHz
                                      0, // maxRateHz
                                      0, // fifoRsvdCount
                                      0, // fifoMaxCount
                                      false // wakeUp
                                      );

    ASSERT_FALSE(sensorsList.addSensor(stepCounterMaxRangeZero));
    ASSERT_EQ(0, sensorsList.getList().size());
}

/**
 * addSensor_fifoRsvdAndFifoMax: verify that adding a sensor with fifo max smaller than reserved fails
 */
TEST_F(STMSensorsListTest, addSensor_fifoRsvdAndFifoMax)
{
    STMSensor accelFifoRsvdBigger(std::string("name"), // name
                                  std::string("vendor"), // vendor
                                  1, // version
                                  STMSensorType::ACCELEROMETER, // type
                                  20, //maxRange
                                  1, // resolution
                                  1, // power
                                  1, // minRateHz
                                  100, // maxRateHz
                                  100, // fifoRsvdCount
                                  0, // fifoMaxCount
                                  false // wakeUp
                                  );

    ASSERT_FALSE(sensorsList.addSensor(accelFifoRsvdBigger));
    ASSERT_EQ(0, sensorsList.getList().size());
}
