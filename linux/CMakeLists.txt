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

cmake_minimum_required(VERSION 3.3)

set(PROJECT_NAME "stmicroelectronics-sensors-linux")
set(PROJECT_DESCRIPTION "STMicroelectronics Sensors IIO Module")
set(PROJECT_VERSION 1.0)
set(PROJECT_TARGET "stm-sensors-linux")

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION}
        DESCRIPTION ${PROJECT_DESCRIPTION}
        LANGUAGES CXX C ASM)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core libstm-sensors-core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core/libs/accel-calibration libstm-accel-calibration)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core/libs/gyro-calibration libstm-gyro-calibration)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core/libs/gyro-temperature-calibration libstm-gyro-temperature-calibration)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core/libs/magn-calibration libstm-magn-calibration)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core/libs/sensors-fusion libstm-sensors-fusion)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core/libs/geomag-fusion libstm-geomag-fusion)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core/libs/timesync libstm-timesync)

add_compile_options(-Wall -Wextra -pedantic)

add_executable(${PROJECT_TARGET}
               main.cpp
               SensorsLinuxInterface.cpp
               IUtils.cpp
               IConsole.cpp
               LinuxPropertiesLoader.cpp)

target_include_directories(${PROJECT_TARGET} PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads)
target_link_libraries(${PROJECT_TARGET} PUBLIC Threads::Threads)

target_link_libraries(${PROJECT_TARGET} PRIVATE stmicroelectronics-sensors-core-linux)
target_link_libraries(${PROJECT_TARGET} PRIVATE stm-accel-calibration)
target_link_libraries(${PROJECT_TARGET} PRIVATE stm-gyro-calibration)
target_link_libraries(${PROJECT_TARGET} PRIVATE stm-gyro-temperature-calibration)
target_link_libraries(${PROJECT_TARGET} PRIVATE stm-geomag-fusion)
target_link_libraries(${PROJECT_TARGET} PRIVATE stm-magn-calibration)
target_link_libraries(${PROJECT_TARGET} PRIVATE stm-sensors-fusion)
target_link_libraries(${PROJECT_TARGET} PRIVATE stm-timesync)

target_compile_features(${PROJECT_TARGET} PUBLIC cxx_std_14)
set_target_properties(${PROJECT_TARGET} PROPERTIES CXX_EXTENSIONS OFF)
