/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2020 STMicroelectronics
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

#include "SensorsSupported.h"

namespace stm {
namespace core {

SensorsSupported SensorsSupported::Accel(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, AccelSensorType, DEVICE_IIO_ACC, a_name, power);
}

SensorsSupported SensorsSupported::Magn(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, MagnSensorType, DEVICE_IIO_MAGN, a_name, power);
}

SensorsSupported SensorsSupported::Gyro(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, GyroSensorType, DEVICE_IIO_GYRO, a_name, power);
}

SensorsSupported SensorsSupported::StepDetector(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, STMSensorType(SensorType::STEP_DETECTOR), DEVICE_IIO_STEP_DETECTOR, a_name, power);
}

SensorsSupported SensorsSupported::StepCounter(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, STMSensorType(SensorType::STEP_COUNTER), DEVICE_IIO_STEP_COUNTER, a_name, power);
}

SensorsSupported SensorsSupported::SignMotion(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, STMSensorType(SensorType::SIGNIFICANT_MOTION), DEVICE_IIO_SIGN_MOTION, a_name, power);
}

const std::array<struct SensorsSupported, 103> sensorsSupportedList = {
    SensorsSupported::Accel("lsm6dsr_accel", "LSM6DSR Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dsr_magn", "LSM6DSR Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dsr_gyro", "LSM6DSR Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dsr_step_d", "LSM6DSR Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dsr_step_c", "LSM6DSR Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dsr_sign_motion", "LSM6DSR Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lsm6dso_accel", "LSM6DSO Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dso_magn", "LSM6DSO Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dso_gyro", "LSM6DSO Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dso_step_d", "LSM6DSO Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dso_step_c", "LSM6DSO Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dso_sign_motion", "LSM6DSO Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lsm6dso32_accel", "LSM6DSO32 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dso32_magn", "LSM6DSO32 Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dso32_gyro", "LSM6DSO32 Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dso32_step_d", "LSM6DSO32 Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dso32_step_c", "LSM6DSO32 Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dso32_sign_motion", "LSM6DSO32 Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lsm6ds3h_accel", "LSM6DS3H Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6ds3h_magn", "LSM6DS3H Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6ds3h_gyro", "LSM6DS3H Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6ds3h_step_d", "LSM6DS3H Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6ds3h_step_c", "LSM6DS3H Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6ds3h_sign_motion", "LSM6DS3H Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lsm6ds33_accel", "LSM6DS33 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Gyro("lsm6ds33_gyro", "LSM6DS33 Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6ds33_step_d", "LSM6DS33 Step Detector Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6ds33_sign_motion", "LSM6DS33 Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lsm6ds3_accel", "LSM6DS3 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6ds3_magn", "LSM6DS3 Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6ds3_gyro", "LSM6DS3 Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6ds3_step_d", "LSM6DS3 Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6ds3_step_c", "LSM6DS3 Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6ds3_sign_motion", "LSM6DS3 Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lsm6dsm_accel", "LSM6DSM Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dsm_magn", "LSM6DSM Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dsm_gyro", "LSM6DSM Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dsm_step_d", "LSM6DSM Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dsm_step_c", "LSM6DSM Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dsm_sign_motion", "LSM6DSM Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lsm6dsl_accel", "LSM6DSL Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dsl_magn", "LSM6DSL Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dsl_gyro", "LSM6DSL Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dsl_step_d", "LSM6DSL Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dsl_step_c", "LSM6DSL Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dsl_sign_motion", "LSM6DSL Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("asm330lhh_accel", "ASM330LHH Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("asm330lhh_gyro", "ASM330LHH Gyroscope Sensor", 0.0f),

    SensorsSupported::Accel("ism330dlc", "ISM330DLC Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("ism330dlc_magn", "ISM330DLC Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("ism330dlc_gyro", "ISM330DLC Gyroscope Sensor", 0.0f),

    SensorsSupported::Accel("ism330dac", "ISM330DAC Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("ism303dac_magn", "ISM303DAC Magnetometer Sensor", 0.0f),

    SensorsSupported::Accel("lsm6ds0_accel", "LSM6DS0 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Gyro("lsm6ds0_gyro", "LSM6DS0 Gyroscope Sensor", 0.0f),

    SensorsSupported::Accel("lsm9ds1_accel", "LSM9DS1 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm9ds1_magn", "LSM9DS1 Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm9ds1_gyro", "LSM9DS1 Gyroscope Sensor", 0.0f),

    SensorsSupported::Accel("lsm303dlhc_accel", "LSM303DLHC Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303dlhc_magn", "LSM303DLHC Magnetometer Sensor", 0.0f),

    SensorsSupported::Accel("lsm330_accel", "LSM330 Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm330_gyro", "LSM330 Gyroscope Sensor", 0.0f),

    SensorsSupported::Accel("lsm330d_accel", "LSM330D Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm330d_gyro", "LSM330D Gyroscope Sensor", 0.0f),

    SensorsSupported::Accel("lsm330dl_accel", "LSM330DL Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm330dl_gyro", "LSM330DL Gyroscope Sensor", 0.0f),

    SensorsSupported::Accel("lsm330dlc_accel", "LSM330DLC Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm330dlc_gyro", "LSM330DLC Gyroscope Sensor", 0.0f),

    SensorsSupported::Accel("lsm303dlh_accel", "LSM303DLH Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303dlh_magn", "LSM303DLH Magnetometer Sensor", 0.0f),

    SensorsSupported::Accel("lsm303dlm_accel", "LSM303DLM Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303dlm_magn", "LSM303DLM Magnetometer Sensor", 0.0f),

    SensorsSupported::Accel("lsm303dl_accel", "LSM303DL Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303dl_magn", "LSM303DL Magnetometer Sensor", 0.0f),

    SensorsSupported::Accel("lsm303ah_accel", "LSM303AH Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303ah_magn", "LSM303AH Magnetometer Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm303ah_step_d", "LSM303AH Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm303ah_step_c", "LSM303AH Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm303ah_sign_motion", "LSM303AH Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lsm303agr_accel", "LSM303AGR Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303agr_magn", "LSM303AGR Magnetometer Sensor", 0.0f),

    SensorsSupported::Accel("lis2ds12_accel", "LIS2DS12 Accelerometer Sensor", 0.0f),
    SensorsSupported::StepDetector("lis2ds12_step_d", "LIS2DS12 Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lis2ds12_step_c", "LIS2DS12 Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lis2ds12_sign_motion", "LIS2DS12 Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lis2dg_accel", "LIS2DG Accelerometer Sensor", 0.0f),
    SensorsSupported::StepDetector("lis2dg_step_d", "LIS2DG Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lis2dg_step_c", "LIS2DG Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lis2dg_sign_motion", "LIS2DG Significant Motion Sensor", 0.0f),

    SensorsSupported::Accel("lis3dhh", "LIS3DHH Accelerometer Sensor", 0.0f),
    SensorsSupported::Accel("iis2dh", "IIS2DH Accelerometer Sensor", 0.0f),
    SensorsSupported::Accel("iis3dhhc", "IIS3DHHC Accelerometer Sensor", 0.0f),
    SensorsSupported::Accel("lis3dh", "LIS3DH Accelerometer Sensor", 0.0f),
    SensorsSupported::Accel("lis331dlh", "LIS331DLH Accelerometer Sensor", 0.0f),
    SensorsSupported::Accel("lis2hh12_accel", "LIS2HH12 Accelerometer Sensor", 0.0f),
    SensorsSupported::Accel("lis2dw12_accel", "LIS2DW12 Accelerometer Sensor", 0.0f),
    SensorsSupported::Accel("lis2dh", "LIS2DH Accelerometer Sensor", 0.0f),

    SensorsSupported::Magn("lis2mdl_magn", "LIS2MDL Magnetometer Sensor", 0.0f),
    SensorsSupported::Magn("lis3mdl_magn", "LIS3MDL Magnetometer Sensor", 0.0f),
    SensorsSupported::Magn("iis2mdc_magn", "IIS2MDC Magnetometer Sensor", 0.0f),

    SensorsSupported::Gyro("l3g4200d", "L3G4200D Gyroscope Sensor", 0.0f),
    SensorsSupported::Gyro("l3gd20", "L3GD20 Gyroscope Sensor", 0.0f),
    SensorsSupported::Gyro("lsm9ds0_gyro", "LSM9DS0 Gyroscope Sensor", 0.0f),
};

const std::array<struct SWSensorsSupported, 10> sensorsSWSupportedList = {
    SWSensorsSupported(STMSensorType(SensorType::ACCELEROMETER_UNCALIBRATED)),
    SWSensorsSupported(STMSensorType(SensorType::MAGNETOMETER_UNCALIBRATED)),
    SWSensorsSupported(STMSensorType(SensorType::GYROSCOPE_UNCALIBRATED)),
    SWSensorsSupported(STMSensorType(SensorTypeInternal::ACCEL_GYRO_FUSION_6X)),
    SWSensorsSupported(STMSensorType(SensorTypeInternal::ACCEL_MAGN_GYRO_FUSION_9X)),
    SWSensorsSupported(STMSensorType(SensorType::GAME_ROTATION_VECTOR)),
    SWSensorsSupported(STMSensorType(SensorType::ROTATION_VECTOR)),
    SWSensorsSupported(STMSensorType(SensorType::ORIENTATION)),
    SWSensorsSupported(STMSensorType(SensorType::GRAVITY)),
    SWSensorsSupported(STMSensorType(SensorType::LINEAR_ACCELERATION)),
};

} // namespace core
} // namespace stm
