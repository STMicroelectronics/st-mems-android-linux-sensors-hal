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

class STMSensorsListTest : public ::testing::Test {
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
    STMSensorsList sensorsList;

    STMSensor accelInvalid(std::string("name"), // name
                           std::string("vendor"), // vendor
                           0, // handle
                           1, // version
                           STMSensorType::ACCELEROMETER, // type
                           1, //maxRange
                           1, // resolution
                           1, // power
                           1, // minRateHz
                           1, // maxRateHz
                           0, // fifoRsvdCount
                           0, // fifoMaxCount
                           false // wakeUp
                           );

    ASSERT_FALSE(sensorsList.addSensor(accelInvalid));
    ASSERT_EQ(0, sensorsList.getList().size());

    STMSensor accelValid(std::string("name"), // name
                         std::string("vendor"), // vendor
                         3, // handle
                         1, // version
                         STMSensorType::ACCELEROMETER, // type
                         1, //maxRange
                         1, // resolution
                         1, // power
                         1, // minRateHz
                         1, // maxRateHz
                         0, // fifoRsvdCount
                         0, // fifoMaxCount
                         false // wakeUp
                         );

    ASSERT_TRUE(sensorsList.addSensor(accelValid));
    ASSERT_EQ(1, sensorsList.getList().size());

    STMSensor gyroInvalid(std::string("name"), // name
                          std::string("vendor"), // vendor
                          0, // handle
                          1, // version
                          STMSensorType::GYROSCOPE, // type
                          1, //maxRange
                          1, // resolution
                          1, // power
                          1, // minRateHz
                          1, // maxRateHz
                          0, // fifoRsvdCount
                          0, // fifoMaxCount
                          false // wakeUp
                          );

    ASSERT_FALSE(sensorsList.addSensor(gyroInvalid));
    ASSERT_EQ(1, sensorsList.getList().size());
}

/**
 * addSensor_rateHzCheck: verify *rateHz parameters
 * - rates cannot be negative
 * - maxRateHz must be greater than minRateHz
 */
TEST_F(STMSensorsListTest, addSensor_rateHzCheck)
{
    STMSensorsList sensorsList;

    STMSensor accelInvalid(std::string("name"), // name
                           std::string("vendor"), // vendor
                           1, // handle
                           1, // version
                           STMSensorType::ACCELEROMETER, // type
                           1, //maxRange
                           1, // resolution
                           1, // power
                           -3.3f, // minRateHz
                           0, // maxRateHz
                           0, // fifoRsvdCount
                           0, // fifoMaxCount
                           false // wakeUp
                           );

    ASSERT_FALSE(sensorsList.addSensor(accelInvalid));
    ASSERT_EQ(0, sensorsList.getList().size());

    STMSensor gyroInvalid(std::string("name"), // name
                          std::string("vendor"), // vendor
                          1, // handle
                          1, // version
                          STMSensorType::GYROSCOPE, // type
                          1, //maxRange
                          1, // resolution
                          1, // power
                          10, // minRateHz
                          1, // maxRateHz
                          0, // fifoRsvdCount
                          0, // fifoMaxCount
                          false // wakeUp
                          );

    ASSERT_FALSE(sensorsList.addSensor(gyroInvalid));
    ASSERT_EQ(0, sensorsList.getList().size());

    STMSensor accelValid(std::string("name"), // name
                         std::string("vendor"), // vendor
                         1, // handle
                         1, // version
                         STMSensorType::ACCELEROMETER, // type
                         1, //maxRange
                         1, // resolution
                         1, // power
                         1, // minRateHz
                         2, // maxRateHz
                         0, // fifoRsvdCount
                         0, // fifoMaxCount
                         false // wakeUp
                         );

    ASSERT_TRUE(sensorsList.addSensor(accelValid));
    ASSERT_EQ(1, sensorsList.getList().size());
}

/**
 * addSensor_sameSensorType: verify that adding same sensor type respect rules
 * - same sensor type must have different name
 */
TEST_F(STMSensorsListTest, addSensor_sameSensorType)
{
    STMSensorsList sensorsList;

    STMSensor accelValid(std::string("name"), // name
                         std::string("vendor"), // vendor
                         1, // handle
                         1, // version
                         STMSensorType::ACCELEROMETER, // type
                         1, //maxRange
                         1, // resolution
                         1, // power
                         1, // minRateHz
                         1, // maxRateHz
                         0, // fifoRsvdCount
                         0, // fifoMaxCount
                         false // wakeUp
                         );

    ASSERT_TRUE(sensorsList.addSensor(accelValid));
    ASSERT_EQ(1, sensorsList.getList().size());

    STMSensor accelInvalid(std::string("name"), // name
                           std::string("vendor"), // vendor
                           2, // handle
                           1, // version
                           STMSensorType::ACCELEROMETER, // type
                           1, //maxRange
                           1, // resolution
                           1, // power
                           1, // minRateHz
                           1, // maxRateHz
                           0, // fifoRsvdCount
                           0, // fifoMaxCount
                           false // wakeUp
                           );

    ASSERT_FALSE(sensorsList.addSensor(accelInvalid));
    ASSERT_EQ(1, sensorsList.getList().size());

    STMSensor accelValid_2(std::string("name_2"), // name
                           std::string("vendor"), // vendor
                           2, // handle
                           1, // version
                           STMSensorType::ACCELEROMETER, // type
                           1, //maxRange
                           1, // resolution
                           1, // power
                           1, // minRateHz
                           1, // maxRateHz
                           0, // fifoRsvdCount
                           0, // fifoMaxCount
                           false // wakeUp
                           );

    ASSERT_TRUE(sensorsList.addSensor(accelValid_2));
    ASSERT_EQ(2, sensorsList.getList().size());
}
