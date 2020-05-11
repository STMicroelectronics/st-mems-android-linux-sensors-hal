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

add_compile_options(-Wall -Wextra -pedantic)

add_library(stmicroelectronics-sensors-core-linux
            ISTMSensorsHAL.cpp
            STMSensorsHAL.cpp
            STMSensorsList.cpp
            STMSensor.cpp
            ISTMSensorsCallbackData.cpp
            STMSensorsCallbackData.cpp)

target_include_directories(stmicroelectronics-sensors-core-linux PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(stmicroelectronics-sensors-core-linux PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR})
