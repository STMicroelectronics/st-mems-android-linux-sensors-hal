/*
 * STMicroelectronics SW Geomagnetic Rotation Vector Sensor Class
 *
 * Copyright 2015-2016 STMicroelectronics Inc.
 * Author: Denis Ciocca - <denis.ciocca@st.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <fcntl.h>
#include <assert.h>
#include <signal.h>

#include "SWGeoMagRotationVector.h"

namespace stm {
namespace core {

SWGeoMagRotationVector::SWGeoMagRotationVector(const char *name, int handle,
                                               int module)
    : SWSensorBaseWithPollrate(name, handle, GeoRotationVecSensorType,
                               true, true, true, false, module)
{
    dependencies_type_list.push_back(AccelMagnFusion6XSensorType);
    id_sensor_trigger = SENSOR_DEPENDENCY_ID_0;
    sensor_event.data.dataLen = SENSOR_DATA_4AXIS;
}

void SWGeoMagRotationVector::ProcessData(SensorBaseData *data)
{
    memcpy(sensor_event.data.data2, data->processed,
           SENSOR_DATA_4AXIS * sizeof(float));
    sensor_event.timestamp = data->timestamp;

    SWSensorBaseWithPollrate::WriteDataToPipe(data->pollrate_ns);
    SWSensorBaseWithPollrate::ProcessData(data);
}

} // namespace core
} // namespace stm
