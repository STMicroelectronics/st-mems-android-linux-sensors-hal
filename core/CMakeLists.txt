##
## Copyright (C) 2018 The Android Open Source Project
## Copyright (C) 2019-2020 STMicroelectronics
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##      http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

cmake_minimum_required(VERSION 3.0.2)

add_compile_options(-Wall -Wextra -pedantic -static)

add_compile_options(-DHAL_ENABLE_ACCEL_CALIBRATION=1
                    -DHAL_ENABLE_GYRO_CALIBRATION=1
                    -DHAL_ENABLE_GYRO_TEMPERATURE_CALIBRATION=0
                    -DHAL_ENABLE_MAGN_CALIBRATION=1
                    -DHAL_ENABLE_SENSORS_FUSION=1
                    -DHAL_ENABLE_TIMESYNC=0
                    -DHAL_MAX_ODR_HZ=110
                    -DHAL_ACCEL_MAX_RANGE_MS2=18
                    -DHAL_MAGN_MAX_RANGE_UT=2000
                    -DHAL_GYRO_MAX_RANGE_RPS=17)

add_library(stmicroelectronics-sensors-core-linux
            STATIC
            ISTMSensorsHAL.cpp
            STMSensorsHAL.cpp
            STMSensorsList.cpp
            STMSensorType.cpp
            STMSensor.cpp
            ISTMSensorsCallbackData.cpp
            STMSensorsCallbackData.cpp
            SensorsSupported.cpp
            Accelerometer.cpp
            AccelerometerLimitedAxes.cpp
            ChangeODRTimestampStack.cpp
            CircularBuffer.cpp
            DeviceOrientation.cpp
            FlushBufferStack.cpp
            Gesture.cpp
            Gyroscope.cpp
            GyroscopeLimitedAxes.cpp
            HWSensorBase.cpp
            Magnetometer.cpp
            Pressure.cpp
            RHumidity.cpp
            SensorBase.cpp
            SensorHAL.cpp
            SignificantMotion.cpp
            StepCounter.cpp
            StepDetector.cpp
            Temp.cpp
            TiltSensor.cpp
            utils.cpp
            WristTiltGesture.cpp
            SWSensorBase.cpp
            SWAccelerometerUncalibrated.cpp
            SWAccelerometerLimitedAxesUncalibrated.cpp
            SWMagnetometerUncalibrated.cpp
            SWGyroscopeUncalibrated.cpp
            SWGyroLimitedAxesUncalibrated.cpp
            SWAccelGyroFusion6X.cpp
            SWAccelMagnGyroFusion9X.cpp
            SWGameRotationVector.cpp
            SWRotationVector.cpp
            SWOrientation.cpp
            SWGravity.cpp
            SWLinearAccel.cpp
            SelfTest.cpp
            PropertiesManager.cpp
            PropertiesParser.cpp)

target_compile_features(stmicroelectronics-sensors-core-linux PUBLIC cxx_std_14)
set_target_properties(stmicroelectronics-sensors-core-linux PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(stmicroelectronics-sensors-core-linux PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(stmicroelectronics-sensors-core-linux PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/libs/accel-calibration
                           ${CMAKE_CURRENT_SOURCE_DIR}/libs/gyro-calibration
                           ${CMAKE_CURRENT_SOURCE_DIR}/libs/gyro-temperature-calibration
                           ${CMAKE_CURRENT_SOURCE_DIR}/libs/magn-calibration
                           ${CMAKE_CURRENT_SOURCE_DIR}/libs/sensors-fusion
                           ${CMAKE_CURRENT_SOURCE_DIR}/libs/timesync)
