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

#include <STMSensorsList.h>

const STMSensor accelHandleZero(std::string("name"), // name
                                std::string("vendor"), // vendor
                                0, // handle
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

const STMSensor accelHandleOne(std::string("name"), // name
                               std::string("vendor"), // vendor
                               1, // handle
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

const STMSensor accelHandleTwo(std::string("name"), // name
                               std::string("vendor"), // vendor
                               2, // handle
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

const STMSensor accelHandleTwoName2(std::string("name_2"), // name
                                    std::string("vendor"), // vendor
                                    2, // handle
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

const STMSensor gyroHandleOne(std::string("name"), // name
                              std::string("vendor"), // vendor
                              1, // handle
                              1, // version
                              STMSensorType::GYROSCOPE, // type
                              100, //maxRange
                              2, // resolution
                              1, // power
                              1, // minRateHz
                              10, // maxRateHz
                              0, // fifoRsvdCount
                              0, // fifoMaxCount
                              false // wakeUp
                              );

const STMSensor magnMinRateHzNegative(std::string("name"), // name
                                      std::string("vendor"), // vendor
                                      3, // handle
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

const STMSensor magnMinRateHzBigger(std::string("name"), // name
                                    std::string("vendor"), // vendor
                                    4, // handle
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
 * addSensor_handleCheck: verify sensor handle
 * - sensor can't use handle 0 or negative
 * - same handle cannot be reused
 */
TEST_F(STMSensorsListTest, addSensor_handleCheck)
{
    // Adding a sensor with handle equal to 0 must fail
    ASSERT_FALSE(sensorsList.addSensor(accelHandleZero));
    ASSERT_EQ(0, sensorsList.getList().size());

    // Adding a sensor with valid parameters should succeed
    ASSERT_TRUE(sensorsList.addSensor(accelHandleOne));
    EXPECT_EQ(1, sensorsList.getList().size());

    // Adding a sensor with same handle must fail
    ASSERT_FALSE(sensorsList.addSensor(gyroHandleOne));
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
    ASSERT_TRUE(sensorsList.addSensor(accelHandleOne));
    ASSERT_EQ(1, sensorsList.getList().size());

    // Adding a sensor with same type and same name must fail
    ASSERT_FALSE(sensorsList.addSensor(accelHandleTwo));
    ASSERT_EQ(1, sensorsList.getList().size());

    // Adding a sensor with same type but different name should succeed
    ASSERT_TRUE(sensorsList.addSensor(accelHandleTwoName2));
    EXPECT_EQ(2, sensorsList.getList().size());
}

/**
 * addSensor_maxRangeNotZero: verify that adding a sensor with maxRange equal to zero fails
 */
TEST_F(STMSensorsListTest, addSensor_maxRangeNotZero)
{
    const STMSensor stepCounterMaxRangeZero(std::string("name"), // name
                                            std::string("vendor"), // vendor
                                            1, // handle
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
    const STMSensor accelFifoRsvdBigger(std::string("name"), // name
                                        std::string("vendor"), // vendor
                                        1, // handle
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
