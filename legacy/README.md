# Introduction

This is the documentation page for the Android legacy components of the sensors-hal iio.

# Build instructions

1. clone this repository into desired folder (i.e. hardware/st/sensors-hal):

> git clone https://github.com/STMicroelectronics/st-mems-android-linux-sensors-hal.git hardware/st/sensors-hal

2. modify the uevent rules file (i.e. `device/<vendor>/<board>/ueventd.rc`) by adding these lines:

```
#common iio char devices
/dev/iio:device* 0666 system system

#sensors common
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

#accelerometer sensor
/sys/bus/iio/devices/iio:device* in_accel_x_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_accel_y_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_accel_z_scale 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_accel_x_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_accel_y_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_accel_z_en 0666 system system

#gyroscope sensor
/sys/bus/iio/devices/iio:device* in_anglvel_x_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_anglvel_y_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_anglvel_z_scale 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_anglvel_x_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_anglvel_y_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_anglvel_z_en 0666 system system

#magnetometer sensor
/sys/bus/iio/devices/iio:device* in_magn_x_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_magn_y_scale 0666 system system
/sys/bus/iio/devices/iio:device* in_magn_z_scale 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_magn_x_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_magn_y_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_magn_z_en 0666 system system

#step counter sensor
/sys/bus/iio/devices/iio:device* scan_elements/in_step_counter_en 0666 system system
/sys/bus/iio/devices/iio:device* max_delivery_rate 0666 system system

#temperature sensor
/sys/bus/iio/devices/iio:device* scan_elements/in_temp_en 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_temp_scale 0666 system system
/sys/bus/iio/devices/iio:device* scan_elements/in_temp_offset 0666 system system

#gesture sensor
/sys/bus/iio/devices/iio:device* scan_elements/in_gesture_en 0666 system system
```

## For Android 10 only

3. modify the device makefile (i.e. `device/<vendor>/<board>/device.mk`) by adding these lines:

```
PRODUCT_PACKAGES += \
	android.hardware.sensors@1.0-service \
	android.hardware.sensors@1.0-impl \
	sensors.default
```

```
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

4. create new manifest.xml file that describe hal service (i.e. `device/<vendor>/<board>/stm-sensors-hal/manifest.xml`) with this content:

```
<manifest version="1.0" type="device">
	<hal format="hidl">
		<name>android.hardware.sensors</name>
		<transport>hwbinder</transport>
		<version>1.0</version>
		<interface>
			<name>ISensors</name>
			<instance>default</instance>
		</interface>
	</hal>
</manifest>
```

5. modify the device board config (i.e. `device/<vendor>/<board>/BoardConfig.mk`) by adding these lines:

> DEVICE_MANIFEST_FILE += `device/<vendor>/<board>/stm-sensors-hal/manifest.xml`

6. export the environment variable TARGET_USES_STM_LEGACY_SENSORSHAL to true:

> export TARGET_USES_STM_LEGACY_SENSORSHAL=true

7. build aosp as described into [official documentation](https://source.android.com/setup/build/building).

## For Android 9 and inferior

3. modify the device makefile (i.e. `device/<vendor>/<board>/device.mk`) by adding these lines:

```
PRODUCT_PACKAGES += \
	sensors.default
```

```
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

4. export the environment variable TARGET_USES_STM_LEGACY_SENSORSHAL to true:

> export TARGET_USES_STM_LEGACY_SENSORSHAL=true

5. build aosp as described into [official documentation](https://source.android.com/setup/build/building).
