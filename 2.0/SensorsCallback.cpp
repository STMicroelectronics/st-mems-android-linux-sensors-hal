// FIXME: your file license if you have one

#include "SensorsCallback.h"

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

// Methods from ::android::hardware::sensors::V2_0::ISensorsCallback follow.
Return<void> SensorsCallback::onDynamicSensorsConnected(const hidl_vec<::android::hardware::sensors::V1_0::SensorInfo>& sensorInfos) {
    // TODO implement
    return Void();
}

Return<void> SensorsCallback::onDynamicSensorsDisconnected(const hidl_vec<int32_t>& sensorHandles) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ISensorsCallback* HIDL_FETCH_ISensorsCallback(const char* /* name */) {
    //return new SensorsCallback();
//}
//
}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
