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

LOCAL_MODULE := stmicroelectronics-sensors-core-legacy

LOCAL_MODULE_OWNER := STMicroelectronics

LOCAL_PROPRIETARY_MODULE := true

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

LOCAL_CFLAGS += \
    -DLOG_TAG=\"stmicroelectronics-sensors-core\" \
    -Wall \
    -Wextra \
    -Wpedantic

ifeq ($(DEBUG),y)
LOCAL_CFLAGS += -g -O0
LOCAL_LDFLAGS += -Wl,-Map,$(LOCAL_PATH)/../$(LOCAL_MODULE).map
endif # DEBUG

LOCAL_SRC_FILES := \
    ISTMSensorsHAL.cpp \
    STMSensorsHAL.cpp \
    STMSensorsList.cpp \
    STMSensor.cpp \
    ISTMSensorsCallbackData.cpp \
    STMSensorsCallbackData.cpp

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/include

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
    libhardware_legacy \
    libhardware \
    libcutils \
    libutils \
    liblog \
    libdl \
    libc

LOCAL_HEADER_LIBRARIES := \
    libhardware_headers

include $(BUILD_STATIC_LIBRARY)

endif # !TARGET_SIMULATOR
