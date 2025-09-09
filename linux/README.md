# Introduction

This is the documentation page for the Linux components of the sensors-hal iio.

# Configuration
## runtime configuration (startup)

Configuration file can be used to set parameters at runtime (during init).
If a parameter is set in the configuration file, default value for that parameter will be discarded.

| Property                    | Description                                 | Example      |
|-----------------------------|---------------------------------------------|--------------|
| `max-odr` | Maximum ODR common for all sensors        |    `250`     |
| `max-range.SENSORTYPE`   | Sensor full-scale   |     `70`     |
| `rot-matrix-1.SENSORTYPE-INSTANCE` | Rotation matrix #1 for sensor instance | `"1,0,0,0,1,0,0,0,1"` |
| `rot-matrix-2.SENSORTYPE-INSTANCE` | Rotation matrix #2 for sensor instance | `"1,0,0,0,1,0,0,0,1"` |
| `placement-1.SENSORTYPE-INSTANCE` | Sensor placement #1 in mm for sensor instance | `"10,20,30"` |
| `placement-2.SENSORTYPE-INSTANCE` | Sensor placement #2 in mm for sensor instance | `"10,20,30"` |

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

Example of configuration file usage (default /etc/stm-sensors-hal/config):

| Porperty                    | Description                                  |
|-----------------------------|---------------------------------------------|
| `max-odr = 250` | max ODR that can be used is 250 Hz for all sensors |
| `rot-matrix-1.accel-0 = "1,0,0,0,1,0,0,0,1"` | rotation matrix #1 for accel index 0 |
| `placement-1.accel-0 = "10,20,30"` | sensor position #1 in cm for accel index 0 |
| `max-range.accel = 70` | accel full-scale to support reading of at least 70m/s^2 |
| `max-range.magn = 2000` | magn full-scale to support reading of at least 2000uT |
| `max-range.gyro = 8` | gyro full-scale to support reading of at least 8rad/s |

## Default settings

Default parameters can be set at compile time by changing the CMakeLists.txt cflags (under core, see core documentation).

# Build instructions

1. clone this repository into desired folder:

> git clone https://github.com/STMicroelectronics/st-mems-android-linux-sensors-hal.git

2. build hal

## release build

> cmake -DCMAKE_BUILD_TYPE=Release ${PROJECT_SOURCE_PATH}

## debug build

> cmake -DCMAKE_BUILD_TYPE=Debug ${PROJECT_SOURCE_PATH}

## export compile commands

> cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ${PROJECT_SOURCE_PATH}

## cross-compile

- arm64

> cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm64.cmake ${PROJECT_SOURCE_PATH}

- arm32

> cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm32.cmake ${PROJECT_SOURCE_PATH}

## ninja build

> cmake -GNinja ${PROJECT_SOURCE_PATH}
