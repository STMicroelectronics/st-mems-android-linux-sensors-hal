#
# Copyright (C) 2018 The Android Open Source Project
# Copyright (C) 2019-2020 STMicroelectronics
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
LOCAL_PATH_BAK := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := stmicroelectronics-sensors-core-legacy

LOCAL_MODULE_OWNER := STMicroelectronics

LOCAL_PROPRIETARY_MODULE := true

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/libs/accel-calibration \
    $(LOCAL_PATH)/libs/gyro-calibration \
    $(LOCAL_PATH)/libs/gyro-temperature-calibration \
    $(LOCAL_PATH)/libs/magn-calibration \
    $(LOCAL_PATH)/libs/sensors-fusion \
    $(LOCAL_PATH)/libs/timesync

LOCAL_CFLAGS += \
    -Wall \
    -Wextra \
    -DHAL_ENABLE_ACCEL_CALIBRATION=1 \
    -DHAL_ENABLE_GYRO_CALIBRATION=1 \
    -DHAL_ENABLE_GYRO_TEMPERATURE_CALIBRATION=0 \
    -DHAL_ENABLE_MAGN_CALIBRATION=1 \
    -DHAL_ENABLE_SENSORS_FUSION=1 \
    -DHAL_ENABLE_TIMESYNC=1 \
    -DHAL_MAX_ODR_HZ=110 \
    -DHAL_ACCEL_MAX_RANGE_MS2=18 \
    -DHAL_MAGN_MAX_RANGE_UT=2000 \
    -DHAL_GYRO_MAX_RANGE_RPS=17

ifeq ($(DEBUG),y)
LOCAL_CFLAGS += -g -O0
LOCAL_LDFLAGS += -Wl,-Map,$(LOCAL_PATH)/../$(LOCAL_MODULE).map
endif # DEBUG

LOCAL_SRC_FILES := \
    ISTMSensorsHAL.cpp \
    STMSensorsHAL.cpp \
    STMSensorsList.cpp \
    STMSensorType.cpp \
    STMSensor.cpp \
    ISTMSensorsCallbackData.cpp \
    STMSensorsCallbackData.cpp \
    SensorsSupported.cpp \
    Accelerometer.cpp \
    AccelerometerLimitedAxes.cpp \
    ChangeODRTimestampStack.cpp \
    CircularBuffer.cpp \
    DeviceOrientation.cpp \
    FlushBufferStack.cpp \
    Gesture.cpp \
    Gyroscope.cpp \
    GyroscopeLimitedAxes.cpp \
    HWSensorBase.cpp \
    Magnetometer.cpp \
    Pressure.cpp \
    RHumidity.cpp \
    SensorBase.cpp \
    SensorHAL.cpp \
    SignificantMotion.cpp \
    StepCounter.cpp \
    StepDetector.cpp \
    Temp.cpp \
    IntTemp.cpp \
    TiltSensor.cpp \
    utils.cpp \
    WristTiltGesture.cpp \
    SWSensorBase.cpp \
    SWAccelerometerUncalibrated.cpp \
    SWAccelerometerLimitedAxesUncalibrated.cpp \
    SWMagnetometerUncalibrated.cpp \
    SWGyroscopeUncalibrated.cpp \
    SWGyroLimitedAxesUncalibrated.cpp \
    SWAccelGyroFusion6X.cpp \
    SWAccelMagnGyroFusion9X.cpp \
    SWGameRotationVector.cpp \
    SWRotationVector.cpp \
    SWOrientation.cpp \
    SWGravity.cpp \
    SWLinearAccel.cpp \
    SelfTest.cpp \
    PropertiesManager.cpp \
    PropertiesParser.cpp

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

LOCAL_STATIC_LIBRARIES := \
    libstm-gyro-temperature-calibration-legacy \
    libstm-gyro-calibration-legacy \
    libstm-timesync-legacy \
    libstm-sensors-fusion-legacy \
    libstm-accel-calibration-legacy \
    libstm-magn-calibration-legacy

LOCAL_HEADER_LIBRARIES := \
    libhardware_headers

include $(BUILD_STATIC_LIBRARY)

include $(LOCAL_PATH)/libs/accel-calibration/Android.mk
LOCAL_PATH := $(LOCAL_PATH_BAK)
include $(LOCAL_PATH)/libs/gyro-calibration/Android.mk
LOCAL_PATH := $(LOCAL_PATH_BAK)
include $(LOCAL_PATH)/libs/gyro-temperature-calibration/Android.mk
LOCAL_PATH := $(LOCAL_PATH_BAK)
include $(LOCAL_PATH)/libs/magn-calibration/Android.mk
LOCAL_PATH := $(LOCAL_PATH_BAK)
include $(LOCAL_PATH)/libs/sensors-fusion/Android.mk
LOCAL_PATH := $(LOCAL_PATH_BAK)
include $(LOCAL_PATH)/libs/timesync/Android.mk

endif # !TARGET_SIMULATOR
