##
## Copyright (C) 2018 The Android Open Source Project
## Copyright (C) 2021 STMicroelectronics
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

add_library(stm-geomag-fusion
            STATIC
            STMGeomagFusion.cpp)

target_compile_features(stm-geomag-fusion PUBLIC cxx_std_14)
set_target_properties(stm-geomag-fusion PROPERTIES CXX_EXTENSIONS OFF)
