# Introduction

This repository hosts the STMicroelectronics HAL (Hardware Abstraction Layer) for MEMS sensors running on Android or Linux systems. It is based on the STMicroelectronics IIO drivers available on [GitHub](https://github.com/STMicroelectronics/st-mems-android-linux-drivers-iio).

This HAL implementation is leaning on the [Linux IIO framework](https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/iio) to gather data from sensor device drivers.

You can find a list of sensors supported directly in the source code at [`Supported Sensors`](core/SensorsSupported.cpp).

# Architecture

Four different components can be identified:

- core :: This is the main component of the HAL. All the adapter components are interacting with the core in order to send commands and receive sensors data and events.
- android multi-hal (Android 11 and above) :: Android [multi-hal](https://source.android.com/devices/sensors/sensors-multihal) adapter.
- android hidl 2.0 (Android 10 and above) :: Android [HIDL 2.0](https://source.android.com/devices/architecture/hidl-cpp) adapter (full treble).
- android aidl (Android 13 and above) :: Android [AIDL](https://source.android.com/docs/core/architecture/aidl/aidl-hals) AIDL adapter (full treble).
- android legacy (Android 10 and inferior) :: Android [legacy](https://source.android.com/devices/architecture/hal) (pre full treble) adapter.
- linux :: Linux implementation adapter.

# Android

HAL is compiled using different makefiles in order to support the two different modes. Fortunately this is transparent at the end for the build system, correct declaration is needed in order to choose the version to build and install.

In Android legacy mode (pre full treble) the HAL is build as dynamic library (.so) and the Android framework or HIDL 1.0 service load the library at runtime. 
In Android HIDL 2.x and AIDL (full treble) mode, HAL is build as an executable and a service is created to run within it.

You can find the related documentation for Android here:

- [`Multi-hal`](multi-hal/README.md)
- [`HIDL 2.0`](2.0/README.md)
- [`AIDL`](aidl/README.md)
- [`Legacy`](legacy/README.md)

# Linux

In Linux, HAL is build as an executable using cmake. Current version does list the sensors found and exit, user needs to implement the final application.

[`Linux`](linux/README.md)

# Core

This is the main component of the project. All the 'wrappers' are using this component for sending commands and receive data stream.

[`Core`](core/README.md) you can find the related documentation.
