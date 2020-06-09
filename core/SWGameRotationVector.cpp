/*
 * STMicroelectronics SW Game Rotation Vector Sensor Class
 *
 * Copyright 2015-2020 STMicroelectronics Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <fcntl.h>
#include <assert.h>
#include <signal.h>

#include "SWGameRotationVector.h"

namespace stm {
namespace core {

SWGameRotationVector::SWGameRotationVector(const char *name, int handle)
    : SWSensorBaseWithPollrate(name, handle,
                               GameRotationVecSensorType,
                               true, true, true, false)
{
    dependencies_type_list.push_back(AccelGyroFusion6XSensorType);
    id_sensor_trigger = SENSOR_DEPENDENCY_ID_0;
    sensor_event.data.dataLen = SENSOR_DATA_4AXIS;
}

void SWGameRotationVector::ProcessData(SensorBaseData *data)
{
    memcpy(sensor_event.data.data2, data->processed, SENSOR_DATA_4AXIS_ACCUR * sizeof(float));
    sensor_event.timestamp = data->timestamp;

    SWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    SWSensorBaseWithPollrate::ProcessData(data);
}

} // namespace core
} // namespace stm
