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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core/ stmicroelectronics-sensors-core)

add_compile_options(-Wall -Wextra -pedantic)

add_executable(${PROJECT_TARGET}
               main.cpp
               SensorsLinuxInterface.cpp
               IUtils.cpp
               IConsole.cpp)

target_include_directories(${PROJECT_TARGET} PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads)
target_link_libraries(${PROJECT_TARGET} Threads::Threads)

target_link_libraries(${PROJECT_TARGET} stmicroelectronics-sensors-core-linux)
target_compile_features(${PROJECT_TARGET} PUBLIC cxx_std_11)
set_target_properties(${PROJECT_TARGET} PROPERTIES CXX_EXTENSIONS OFF)
