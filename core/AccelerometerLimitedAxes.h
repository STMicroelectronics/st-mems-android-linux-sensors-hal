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

#pragma once

#include "HWSensorBase.h"

#include <STMAccelCalibration.h>

namespace stm {
namespace core {

/*
 * class AccelerometerLimitedAxes
 */
class AccelerometerLimitedAxes : public HWSensorBaseWithPollrate {
public:
    AccelerometerLimitedAxes(HWSensorBaseCommonData *data, const char *name,
                  struct device_iio_sampling_freqs *sfa, int handle,
                  unsigned int hw_fifo_len, float power_consumption,
                  bool wakeup, int module,
                  bool x_is_supp, bool y_is_supp, bool z_is_supp);

    int64_t bias_last_pollrate;

    virtual int libsInit(void) override;

    virtual int Enable(int handle, bool enable, bool lock_en_mutex) override;
    virtual void ProcessData(SensorBaseData *data) override;
    virtual void postSetup(void) override;

private:
    STMAccelCalibration& accelCalibration;

    void saveBiasValues(void) const;

    void loadBiasValues(void);

    Matrix<3, 3, float> rotMatrix;

    std::string biasFileName;
};

} // namespace core
} // namespace stm
