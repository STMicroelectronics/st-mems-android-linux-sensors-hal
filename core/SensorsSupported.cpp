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

SensorsSupported SensorsSupported::Pressure(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, STMSensorType(SensorType::PRESSURE), DEVICE_IIO_PRESSURE, a_name, power);
}

SensorsSupported SensorsSupported::AmbientTemperature(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, STMSensorType(SensorType::AMBIENT_TEMPERATURE), DEVICE_IIO_TEMP, a_name, power);
}

SensorsSupported SensorsSupported::InternalTemperature(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, STMSensorType(SensorType::INTERNAL_TEMPERATURE), DEVICE_IIO_TEMP, a_name, power);
}

SensorsSupported SensorsSupported::RelativeHumidity(const std::string &d_name, const std::string &a_name, float power)
{
    return SensorsSupported(d_name, STMSensorType(SensorType::RELATIVE_HUMIDITY), DEVICE_IIO_HUMIDITYRELATIVE, a_name, power);
}

const std::array<struct SensorsSupported, 144> sensorsSupportedList = {
    /* LSM6DSV16X */
    SensorsSupported::Accel("lsm6dsv16x_accel", "LSM6DSV16X Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm6dsv16x_magn", "LSM6DSV16X Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dsv16x_gyro", "LSM6DSV16X Gyroscope Sensor", 0.0f),
    SensorsSupported::Pressure("lsm6dsv16x_press", "LSM6DSV16X Pressure Sensor", 0.0f),

    /* LSM6DSR */
    SensorsSupported::Accel("lsm6dsr_accel", "LSM6DSR Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dsr_magn", "LSM6DSR Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dsr_gyro", "LSM6DSR Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dsr_step_d", "LSM6DSR Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dsr_step_c", "LSM6DSR Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dsr_sign_motion", "LSM6DSR Significant Motion Sensor", 0.0f),
    SensorsSupported::Pressure("lsm6dsr_press", "LSM6DSR Pressure Sensor", 0.0f),

    /* LSM6DSO */
    SensorsSupported::Accel("lsm6dso_accel", "LSM6DSO Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dso_magn", "LSM6DSO Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dso_gyro", "LSM6DSO Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dso_step_d", "LSM6DSO Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dso_step_c", "LSM6DSO Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dso_sign_motion", "LSM6DSO Significant Motion Sensor", 0.0f),
    SensorsSupported::Pressure("lsm6dso_press", "LSM6DSO Pressure Sensor", 0.0f),

    /* LSM6DSOX */
    SensorsSupported::Accel("lsm6dsox_accel", "LSM6DSOX Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dsox_magn", "LSM6DSOX Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dsox_gyro", "LSM6DSOX Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dsox_step_d", "LSM6DSOX Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dsox_step_c", "LSM6DSOX Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dsox_sign_motion", "LSM6DSOX Significant Motion Sensor", 0.0f),
    SensorsSupported::Pressure("lsm6dsox_press", "LSM6DSOX Pressure Sensor", 0.0f),

    /* LSM6DSO32 */
    SensorsSupported::Accel("lsm6dso32_accel", "LSM6DSO32 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dso32_magn", "LSM6DSO32 Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dso32_gyro", "LSM6DSO32 Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dso32_step_d", "LSM6DSO32 Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dso32_step_c", "LSM6DSO32 Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dso32_sign_motion", "LSM6DSO32 Significant Motion Sensor", 0.0f),
    SensorsSupported::Pressure("lsm6dso32_press", "LSM6DSO32 Pressure Sensor", 0.0f),

    /* LSM6DS3H */
    SensorsSupported::Accel("lsm6ds3h_accel", "LSM6DS3H Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6ds3h_magn", "LSM6DS3H Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6ds3h_gyro", "LSM6DS3H Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6ds3h_step_d", "LSM6DS3H Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6ds3h_step_c", "LSM6DS3H Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6ds3h_sign_motion", "LSM6DS3H Significant Motion Sensor", 0.0f),
    SensorsSupported::Pressure("lsm6ds3h_press", "LSM6DS3H Pressure Sensor", 0.0f),

    /* LSM6DS33 */
    SensorsSupported::Accel("lsm6ds33_accel", "LSM6DS33 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Gyro("lsm6ds33_gyro", "LSM6DS33 Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6ds33_step_d", "LSM6DS33 Step Detector Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6ds33_sign_motion", "LSM6DS33 Significant Motion Sensor", 0.0f),

    /* LSM6DS3 */
    SensorsSupported::Accel("lsm6ds3_accel", "LSM6DS3 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6ds3_magn", "LSM6DS3 Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6ds3_gyro", "LSM6DS3 Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6ds3_step_d", "LSM6DS3 Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6ds3_step_c", "LSM6DS3 Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6ds3_sign_motion", "LSM6DS3 Significant Motion Sensor", 0.0f),
    SensorsSupported::Pressure("lsm6ds3_press", "LSM6DS3 Pressure Sensor", 0.0f),

    /* LSM6DSM */
    SensorsSupported::Accel("lsm6dsm_accel", "LSM6DSM Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dsm_magn", "LSM6DSM Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dsm_gyro", "LSM6DSM Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dsm_step_d", "LSM6DSM Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dsm_step_c", "LSM6DSM Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dsm_sign_motion", "LSM6DSM Significant Motion Sensor", 0.0f),

    /* LSM6DSL */
    SensorsSupported::Accel("lsm6dsl_accel", "LSM6DSL Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm6dsl_magn", "LSM6DSL Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm6dsl_gyro", "LSM6DSL Gyroscope Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm6dsl_step_d", "LSM6DSL Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm6dsl_step_c", "LSM6DSL Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm6dsl_sign_motion", "LSM6DSL Significant Motion Sensor", 0.0f),

    /* ASM330LHH */
    SensorsSupported::Accel("asm330lhh_accel", "ASM330LHH Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("asm330lhh_gyro", "ASM330LHH Gyroscope Sensor", 0.0f),
    SensorsSupported::InternalTemperature("asm330lhh_temp", "ASM330LHH Temperature Sensor", 0.0f),

    /* ASM330LHHX */
    SensorsSupported::Accel("asm330lhhx_accel", "ASM330LHHX Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("asm330lhhx_gyro", "ASM330LHHX Gyroscope Sensor", 0.0f),
    SensorsSupported::InternalTemperature("asm330lhhx_temp", "ASM330LHHX Temperature Sensor", 0.0f),

    /* ISM330DLC */
    SensorsSupported::Accel("ism330dlc", "ISM330DLC Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("ism330dlc_magn", "ISM330DLC Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("ism330dlc_gyro", "ISM330DLC Gyroscope Sensor", 0.0f),

    /* ISM330DAC */
    SensorsSupported::Accel("ism330dac", "ISM330DAC Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("ism303dac_magn", "ISM303DAC Magnetometer Sensor", 0.0f),

    /* LSM6DS0 */
    SensorsSupported::Accel("lsm6ds0_accel", "LSM6DS0 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Gyro("lsm6ds0_gyro", "LSM6DS0 Gyroscope Sensor", 0.0f),

    /* LSM9DS1 */
    SensorsSupported::Accel("lsm9ds1_accel", "LSM9DS1 Accelerometer Sensor", 240E-3f),
    SensorsSupported::Magn("lsm9ds1_magn", "LSM9DS1 Magnetometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm9ds1_gyro", "LSM9DS1 Gyroscope Sensor", 0.0f),

    /* LSM303DLHC */
    SensorsSupported::Accel("lsm303dlhc_accel", "LSM303DLHC Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303dlhc_magn", "LSM303DLHC Magnetometer Sensor", 0.0f),

    /* LSM330 */
    SensorsSupported::Accel("lsm330_accel", "LSM330 Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm330_gyro", "LSM330 Gyroscope Sensor", 0.0f),

    /* LSM330D */
    SensorsSupported::Accel("lsm330d_accel", "LSM330D Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm330d_gyro", "LSM330D Gyroscope Sensor", 0.0f),

    /* LSM330DL */
    SensorsSupported::Accel("lsm330dl_accel", "LSM330DL Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm330dl_gyro", "LSM330DL Gyroscope Sensor", 0.0f),

    /* LSM330DLC */
    SensorsSupported::Accel("lsm330dlc_accel", "LSM330DLC Accelerometer Sensor", 0.0f),
    SensorsSupported::Gyro("lsm330dlc_gyro", "LSM330DLC Gyroscope Sensor", 0.0f),

    /* LSM303DLH */
    SensorsSupported::Accel("lsm303dlh_accel", "LSM303DLH Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303dlh_magn", "LSM303DLH Magnetometer Sensor", 0.0f),

    /* LSM303DLM */
    SensorsSupported::Accel("lsm303dlm_accel", "LSM303DLM Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303dlm_magn", "LSM303DLM Magnetometer Sensor", 0.0f),

    /* LSM303DL */
    SensorsSupported::Accel("lsm303dl_accel", "LSM303DL Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303dl_magn", "LSM303DL Magnetometer Sensor", 0.0f),

    /* LSM303AH */
    SensorsSupported::Accel("lsm303ah_accel", "LSM303AH Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303ah_magn", "LSM303AH Magnetometer Sensor", 0.0f),
    SensorsSupported::StepDetector("lsm303ah_step_d", "LSM303AH Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lsm303ah_step_c", "LSM303AH Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lsm303ah_sign_motion", "LSM303AH Significant Motion Sensor", 0.0f),

    /* LSM303AGR */
    SensorsSupported::Accel("lsm303agr_accel", "LSM303AGR Accelerometer Sensor", 0.0f),
    SensorsSupported::Magn("lsm303agr_magn", "LSM303AGR Magnetometer Sensor", 0.0f),

    /* LIS2DS12 */
    SensorsSupported::Accel("lis2ds12_accel", "LIS2DS12 Accelerometer Sensor", 0.0f),
    SensorsSupported::StepDetector("lis2ds12_step_d", "LIS2DS12 Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lis2ds12_step_c", "LIS2DS12 Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lis2ds12_sign_motion", "LIS2DS12 Significant Motion Sensor", 0.0f),

    /* LIS2DG */
    SensorsSupported::Accel("lis2dg_accel", "LIS2DG Accelerometer Sensor", 0.0f),
    SensorsSupported::StepDetector("lis2dg_step_d", "LIS2DG Step Detector Sensor", 0.0f),
    SensorsSupported::StepCounter("lis2dg_step_c", "LIS2DG Step Counter Sensor", 0.0f),
    SensorsSupported::SignMotion("lis2dg_sign_motion", "LIS2DG Significant Motion Sensor", 0.0f),

    /* LIS3DHH */
    SensorsSupported::Accel("lis3dhh", "LIS3DHH Accelerometer Sensor", 0.0f),

    /* IIS2DH */
    SensorsSupported::Accel("iis2dh", "IIS2DH Accelerometer Sensor", 0.0f),

    /* IIS3DHHC */
    SensorsSupported::Accel("iis3dhhc", "IIS3DHHC Accelerometer Sensor", 0.0f),

    /* LIS3DH */
    SensorsSupported::Accel("lis3dh", "LIS3DH Accelerometer Sensor", 0.0f),

    /* LIS331DLH */
    SensorsSupported::Accel("lis331dlh", "LIS331DLH Accelerometer Sensor", 0.0f),

    /* LIS2HH12 */
    SensorsSupported::Accel("lis2hh12_accel", "LIS2HH12 Accelerometer Sensor", 0.0f),

    /* LIS2DW12 */
    SensorsSupported::Accel("lis2dw12_accel", "LIS2DW12 Accelerometer Sensor", 0.0f),

    /* LIS2DH */
    SensorsSupported::Accel("lis2dh", "LIS2DH Accelerometer Sensor", 0.0f),

    /* LIS2MDL */
    SensorsSupported::Magn("lis2mdl_magn", "LIS2MDL Magnetometer Sensor", 0.0f),

    /* LIS3MDL */
    SensorsSupported::Magn("lis3mdl_magn", "LIS3MDL Magnetometer Sensor", 0.0f),

    /* IIS2MDC */
    SensorsSupported::Magn("iis2mdc_magn", "IIS2MDC Magnetometer Sensor", 0.0f),

    /* L3G4200D */
    SensorsSupported::Gyro("l3g4200d", "L3G4200D Gyroscope Sensor", 0.0f),

    /* L3GD20 */
    SensorsSupported::Gyro("l3gd20", "L3GD20 Gyroscope Sensor", 0.0f),

    /* LSM9DS0 */
    SensorsSupported::Gyro("lsm9ds0_gyro", "LSM9DS0 Gyroscope Sensor", 0.0f),

    /* LPS331AP */
    SensorsSupported::Pressure("lps331ap", "LPS331AP Pressure Sensor", 0.0f),

    /* LPS25H */
    SensorsSupported::Pressure("lps25h", "LPS25H Pressure Sensor", 0.0f),

    /* LPS33HW */
    SensorsSupported::Pressure("lps33hw_press", "LPS33HW Pressure Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("lps33hw_temp", "LPS33HW Temperature Sensor", 0.0f),

    /* LPS35HW */
    SensorsSupported::Pressure("lps35hw_press", "LPS35HW Pressure Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("lps35hw_temp", "LPS35HW Temperature Sensor", 0.0f),

    /* LPS22CH */
    SensorsSupported::Pressure("lps22ch_press", "LPS22CH Pressure Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("lps22ch_temp", "LPS22CH Temperature Sensor", 0.0f),

    /* LPS22DF */
    SensorsSupported::Pressure("lps22df_press", "LPS22DF Pressure Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("lps22df_temp", "LPS22DF Temperature Sensor", 0.0f),

    /* LPS22HD */
    SensorsSupported::Pressure("lps22hd_press", "LPS22HD Pressure Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("lps33hd_temp", "LPS22HD Temperature Sensor", 0.0f),

    /* LPS22HB */
    SensorsSupported::Pressure("lps22hb_press", "LPS22HB Pressure Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("lps22hb_temp", "LPS22HB Temperature Sensor", 0.0f),

    /* LPS22HH */
    SensorsSupported::Pressure("lps22hh_press", "LPS22HH Pressure Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("lps22hh_temp", "LPS22HH Temperature Sensor", 0.0f),

    /* LPS27HHW */
    SensorsSupported::Pressure("lps27hhw_press", "LPS27HHW Pressure Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("lps27hhw_temp", "LPS27HHW Temperature Sensor", 0.0f),

    /* HTS221 */
    SensorsSupported::RelativeHumidity("hts221_rh", "HTS221 Humidity Sensor", 0.0f),
    SensorsSupported::AmbientTemperature("hts221_temp", "HTS221 Temperature Sensor", 0.0f),

    /* STTS22H */
    SensorsSupported::AmbientTemperature("stts22h", "STTS22H Temperature Sensor", 0.0f),
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
