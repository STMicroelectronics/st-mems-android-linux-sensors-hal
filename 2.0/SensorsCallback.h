// FIXME: your file license if you have one

#pragma once

#include <android/hardware/sensors/2.0/ISensorsCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct SensorsCallback : public ISensorsCallback {
    // Methods from ::android::hardware::sensors::V2_0::ISensorsCallback follow.
    Return<void> onDynamicSensorsConnected(const hidl_vec<::android::hardware::sensors::V1_0::SensorInfo>& sensorInfos) override;
    Return<void> onDynamicSensorsDisconnected(const hidl_vec<int32_t>& sensorHandles) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" ISensorsCallback* HIDL_FETCH_ISensorsCallback(const char* name);

}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
