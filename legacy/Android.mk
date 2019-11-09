#
# Copyright (C) 2018 The Android Open Source Project
# Copyright (C) 2019 STMicroelectronics
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

PLATFORM_VERSION_MAJOR := $(word 1, $(subst ., ,$(PLATFORM_VERSION)))
PLATFORM_VERSION_MINOR := $(word 2, $(subst ., ,$(PLATFORM_VERSION)))
PLATFORM_VERSION_REVISION := $(word 3, $(subst ., ,$(PLATFORM_VERSION)))

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libhardware \
    libhardware_legacy \
    libutils \
    liblog \
    libdl \
    libc

LOCAL_STATIC_LIBRARIES := \
    stmicroelectronics-sensors-core-legacy

LOCAL_HEADER_LIBRARIES := \
    libhardware_headers

LOCAL_EXPORT_STATIC_LIBRARY_HEADERS := stmicroelectronics-sensors-core-legacy

LOCAL_VENDOR_MODULE := true

LOCAL_PRELINK_MODULE := false
LOCAL_PROPRIETARY_MODULE := true

ifdef TARGET_BOARD_PLATFORM
LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)
else # TARGET_BOARD_PLATFORM
LOCAL_MODULE := sensors.default
endif # TARGET_BOARD_PLATFORM

ifeq ($(ST_HAL_ANDROID_VERSION),0)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
else # ST_HAL_ANDROID_VERSION
LOCAL_MODULE_RELATIVE_PATH := hw
endif # ST_HAL_ANDROID_VERSION

LOCAL_MODULE_OWNER := STMicroelectronics

LOCAL_CFLAGS += \
    -DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR) \
    -DPLATFORM_VERSION_MINOR=$(PLATFORM_VERSION_MINOR) \
    -DPLATFORM_VERSION_REVISION=$(PLATFORM_VERSION_REVISION) \
    -DLOG_TAG=\"sensors@legacy-stmicroelectronics\" \
    -Wall \
    -Wextra

ifeq ($(DEBUG),y)
LOCAL_CFLAGS += -g -O0
LOCAL_LDFLAGS += -Wl,-Map,$(LOCAL_PATH)/../$(LOCAL_MODULE).map
endif # DEBUG

LOCAL_SRC_FILES := \
    SensorsModule.cpp \
    SensorsLegacyInterface.cpp \
    Convert.cpp \
    IUtils.cpp \
    Utils.cpp \
    IConsole.cpp \
    Console.cpp

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif # !TARGET_SIMULATOR
