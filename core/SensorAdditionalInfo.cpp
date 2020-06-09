/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2015-2020 STMicroelectronics
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

#include <stdlib.h>

#include "SensorAdditionalInfo.h"

SensorAdditionalInfoEvent::SensorAdditionalInfoEvent(int32_t payload_type, int32_t serial)
{
    sensor_additional_info_event.type = payload_type;
    sensor_additional_info_event.serial = serial;
}

SensorAdditionalInfoEvent::~SensorAdditionalInfoEvent()
{
   //delete or so;
}

const additional_info_event_t* SensorAdditionalInfoEvent::getBeginFrameEvent()
{
    static additional_info_event_t sensor_additional_info_beginFrame;

    sensor_additional_info_beginFrame.type = AINFO_BEGIN;
    sensor_additional_info_beginFrame.serial = 0;

    return &sensor_additional_info_beginFrame;
}

const additional_info_event_t* SensorAdditionalInfoEvent::getEndFrameEvent()
{
    static additional_info_event_t sensor_additional_info_endFrame;

    sensor_additional_info_endFrame.type = AINFO_END;
    sensor_additional_info_endFrame.serial = 0;

    return &sensor_additional_info_endFrame;
}

void SensorAdditionalInfoEvent::incrementEventSerial()
{
    sensor_additional_info_event.serial++;
}
