# Introduction

This is the documentation page for the Android HIDL 2.0 components of the sensors-hal iio.

# Configuration

## Android properties

Android allows properties to be configured both at run-time ([system properties](https://source.android.com/devices/architecture/configuration/add-system-properties)) and using a properties file at build time. If a parameter is set by using system properties, the default value for that property will be discarded.
Following are the properties names which allow to configure SensorsHAL parameters and behavior:

| Property                    | Description                                 | Example      |
|-----------------------------|---------------------------------------------|--------------|
| `persist.vendor.stm.sensors.max-odr` | Maximum ODR common for all sensors        |    `250`     |
| `persist.vendor.stm.sensors.max-range.SENSORTYPE`   | Sensor full-scale   |     `70`     |
| `persist.vendor.stm.sensors.rot-matrix-1.SENSORTYPE-INSTANCE` | Rotation matrix #1 for sensor instance | `"1,0,0,0,1,0,0,0,1"` |
| `persist.vendor.stm.sensors.rot-matrix-2.SENSORTYPE-INSTANCE` | Rotation matrix #2 for sensor instance | `"1,0,0,0,1,0,0,0,1"` |
| `persist.vendor.stm.sensors.placement-1.SENSORTYPE-INSTANCE` | Sensor placement #1 in mm for sensor instance | `"10,20,30"` |
| `persist.vendor.stm.sensors.placement-2.SENSORTYPE-INSTANCE` | Sensor placement #2 in mm for sensor instance | `"10,20,30"` |

where:
- **SENSORTYPE** can be one of the following sensor types:
	1. `accel` &mdash; Accelerometer
	2. `magn` &mdash; Magnetometer
	3. `gyro` &mdash; Gyroscope

- **INSTANCE** is a numeric index identifying the sensor instance:
	- If there is only one instance, the index will be `0`
	- For multiple instances, use `0`, `1`, `2`, etc., to distinguish between them

- **max-odr** is a property common to all hardware sensor
- **max-range** is a property shared between all sensors of the same type
- **rot-matrix-1** is a property that represents the rotation of the sensor axes with respect to how it is mounted
- **rot-matrix-2** is a property that represents the rotation of the mounted sensor axes with respect to the Android reference. The final rotation matrix reported in the Additional Info sensor placement message is the product of rot-matrix-1 and rot-matrix-2. If rot-matrix-2 is not specified, only rot-matrix-1 will be applied. If rot-matrix-1 is also not specified, the identity matrix will be used. The final rotation matrix provides the orientation of the Android device coordinate frame relative to the local coordinate frame of the sensor.
- **placement-1** is a property that represents the geometric center of the sensor placement with respect to how it is mounted
- **placement-2** is a property that represents the sensor placement of the mounted sensor axes with respect to the Android reference. The final location vector represents the translation from the origin of the Android sensor coordinate system to the geometric center of the sensor, specified in millimeters (mm). If a rotation matrix (rot-matrix-2) exists, the rotation matrix is ​​applied to placement-1 and the result is added to placement-2 component by component to obtain the final placement specified by the additional sensor info message. If the rotation matrix rot-matrix-2 does not exist, the final placement will be simply placement-1.

Example of properties usage, for inizializig them at Android boot, create a file as follow with properties settings (i.e. device/\<vendor>/\<board>/stm_sensors_hal.prop), containing:
| Porperty                    | Description                                  |
|-----------------------------|---------------------------------------------|
| `persist.vendor.stm.sensors.max-odr = 250` | max ODR that can be used is 250 Hz for all sensors |
| `persist.vendor.stm.sensors.rot-matrix-1.accel-0 = "1,0,0,0,1,0,0,0,1"` | rotation matrix #1 for accel index 0 |
| `persist.vendor.stm.sensors.placement-1.accel-0 = "10,20,30"` | sensor position #1 in cm for accel index 0 |
| `persist.vendor.stm.sensors.max-range.accel = 70` | accel full-scale to support reading of at least 70m/s^2 |
| `persist.vendor.stm.sensors.max-range.magn = 2000` | magn full-scale to support reading of at least 2000uT |
| `persist.vendor.stm.sensors.max-range.gyro = 8` | gyro full-scale to support reading of at least 8rad/s |

Add the following line to the device makefile (device/\<vendor>/\<board>/device.mk):

> TARGET_VENDOR_PROP += device/\<vendor>/\<board>/stm_sensors_hal.prop

Example of Android properties usage when initializing them at run-time:

```
setprop persist.vendor.stm.sensors.max-odr 250
setprop persist.vendor.stm.sensors.rot-matrix-1.accel-0 "1,0,0,0,1,0,0,0,1"
setprop persist.vendor.stm.sensors.placement-1.accel-0 "10,20,30"
setprop persist.vendor.stm.sensors.max-range.accel 70
setprop persist.vendor.stm.sensors.max-range.magn 2000
setprop persist.vendor.stm.sensors.max-range.gyro 8
```

## Android features

| Feature/Parameter                | Description  | Possible Values / Example |
|----------------------------------|--------------|---------------------------|
| `HAL_ENABLE_DIRECT_REPORT_CHANNEL` | Enables direct report channel feature during HAL build. | `0` (disabled) or any non-zero value (enabled) |
| `HAL_ENABLE_SENSOR_ADDITIONAL_INFO` | Enables sensor additional info feature during HAL build. | `0` (disabled) or any non-zero value (enabled) |
| `HAL_PRIVATE_DATA_PATH` | Absolute path (in data vendor) for persistent sensor calibration data.  | Example: `/data/vendor/sensors` |

> **Note:**  
> - To guarantee persistence of sensor calibration data, set `HAL_PRIVATE_DATA_PATH` to a directory where the HAL can create, write, and read files.
> - Proper SELinux rules and directory creation in device-specific `.mk` files are required to allow HAL access to persistent data files.

## Default settings

Default parameters can alternatively be set at compile time by changing the Android.bp cflags (under core, see core documentation).

# Build instructions

1. clone this repository into desired folder (i.e. hardware/st/sensors-hal):

> git clone https://github.com/STMicroelectronics/st-mems-android-linux-sensors-hal.git hardware/st/sensors-hal

## Android 10 and above

2. modify the device makefile (i.e. device/\<vendor>/\<board>/device.mk) by adding these lines:

```
# Build stm-sensors-hal in full treble mode
PRODUCT_PACKAGES += \
	android.hardware.sensors@2.0-service.stmicroelectronics

# Copy sensors config file(s)
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
	frameworks/native/data/etc/android.hardware.sensor.ambient_temperature.xml:system/etc/permissions/android.hardware.sensor.ambient_temperature.xml \
	frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
	frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
	frameworks/native/data/etc/android.hardware.sensor.hifi_sensors.xml:system/etc/permissions/android.hardware.sensor.hifi_sensors.xml \
	frameworks/native/data/etc/android.hardware.sensor.relative_humidity.xml:system/etc/permissions/android.hardware.sensor.relative_humidity.xml \
	frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:system/etc/permissions/android.hardware.sensor.stepcounter.xml \
	frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:system/etc/permissions/android.hardware.sensor.stepdetector.xml
```

3. modify the sepolicy file_contexts file (i.e. device/\<vendor>/\<board>/sepolicy/file_contexts) by adding these lines:

> /vendor/bin/hw/android\.hardware\.sensors@2\.0-service\.stmicroelectronics  u:object_r:hal_sensors_default_exec:s0


4. modify the sepolicy hal_sensors.te file (i.e. device/\<vendor>/\<board>/sepolicy/hal_sensors.te) by adding these lines:

```
# allow access to sysfs device iio
allow hal_sensors_default sysfs:dir { open read };
allow hal_sensors_default sysfs:file { open read write getattr };
allow hal_sensors_default sensors_device:chr_file rw_file_perms;
allow hal_sensors_default iio_device:chr_file { open read ioctl };

# allow access to save data to vendor data files
allow hal_sensors_default sensor_vendor_data_file:file { open read write getattr create };
```

5. modify the uevent rules file (i.e. device/\<vendor>/\<board>/ueventd.rc) by adding these lines:

```
# common iio char devices
/dev/iio:device* 0666 system system

# sensors common
/sys/bus/iio/devices/iio:device* buffer/enable 0666 system system
/sys/bus/iio/devices/iio:device* buffer/length 0666 system system
/sys/bus/iio/devices/iio:device* sampling_frequency 0666 system system
/sys/bus/iio/devices/iio:device* hwfifo_flush 0666 system system
/sys/bus/iio/devices/iio:device* hwfifo_enabled 0666 system system
/sys/bus/iio/devices/iio:device* hwfifo_watermark 0666 system system
/sys/bus/iio/devices/iio:device* injection_mode 0666 system system
/sys/bus/iio/devices/iio:device* current_timestamp_clock 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_timestamp_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_count_en 0666 system system

# accelerometer sensor
/sys/bus/iio/devices/iio:device* in_accel_x_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_accel_y_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_accel_z_scale 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_accel_x_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_accel_y_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_accel_z_en 0666 system system

# gyroscope sensor
/sys/bus/iio/devices/iio:device* in_anglvel_x_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_anglvel_y_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_anglvel_z_scale 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_anglvel_x_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_anglvel_y_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_anglvel_z_en 0666 system system

# magnetometer sensor
/sys/bus/iio/devices/iio:device* in_magn_x_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_magn_y_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_magn_z_scale 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_magn_x_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_magn_y_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_magn_z_en 0666 system system

# step counter sensor
/sys/bus/iio/devices/iio:device* scan_elements/in_step_counter_en 0666 system system
/sys/bus/iio/devices/iio:device* max_delivery_rate 0666 system system

# temperature sensor
/sys/bus/iio/devices/iio:device* scan_elements/in_temp_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_temp_scale 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_temp_offset 0666 system system

# gesture sensor
/sys/bus/iio/devices/iio:device* scan_elements/in_gesture_en 0666 system system

# pressure sensor
/sys/bus/iio/devices/iio:device* scan_elements/in_pressure_en 0666 system system

# humidity sensor
/sys/bus/iio/devices/iio:device* scan_elements/in_humidityrelative_en 0666 system system
```

6. build aosp as described into [official documentation](https://source.android.com/setup/build/building).
