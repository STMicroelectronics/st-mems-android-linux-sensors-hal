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

#pragma once

#define ANDROID_VERSION(major, minor, rev) \
                        ((rev) | (minor << 8) | (major << 16))

#ifndef PLATFORM_VERSION_REVISION
#define PLATFORM_VERSION_REVISION 0
#endif /* PLATFORM_VERSION_REVISION */

#define ANDROID_VERSION_CODE \
    ANDROID_VERSION(PLATFORM_VERSION_MAJOR, \
                    PLATFORM_VERSION_MINOR, \
                    PLATFORM_VERSION_REVISION)
