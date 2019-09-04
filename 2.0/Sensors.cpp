// FIXME: your file license if you have one

#include "Sensors.h"

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

// Methods from ::android::hardware::sensors::V2_0::ISensors follow.
Return<void> Sensors::getSensorsList(getSensorsList_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<::android::hardware::sensors::V1_0::Result> Sensors::setOperationMode(::android::hardware::sensors::V1_0::OperationMode mode) {
    // TODO implement
    return ::android::hardware::sensors::V1_0::Result {};
}

Return<::android::hardware::sensors::V1_0::Result> Sensors::activate(int32_t sensorHandle, bool enabled) {
    // TODO implement
    return ::android::hardware::sensors::V1_0::Result {};
}

Return<::android::hardware::sensors::V1_0::Result> Sensors::initialize(const ::android::hardware::MQDescriptorSync<::android::hardware::sensors::V1_0::Event>& eventQueueDescriptor, const ::android::hardware::MQDescriptorSync<uint32_t>& wakeLockDescriptor, const sp<::android::hardware::sensors::V2_0::ISensorsCallback>& sensorsCallback) {
    // TODO implement
    return ::android::hardware::sensors::V1_0::Result {};
}

Return<::android::hardware::sensors::V1_0::Result> Sensors::batch(int32_t sensorHandle, int64_t samplingPeriodNs, int64_t maxReportLatencyNs) {
    // TODO implement
    return ::android::hardware::sensors::V1_0::Result {};
}

Return<::android::hardware::sensors::V1_0::Result> Sensors::flush(int32_t sensorHandle) {
    // TODO implement
    return ::android::hardware::sensors::V1_0::Result {};
}

Return<::android::hardware::sensors::V1_0::Result> Sensors::injectSensorData(const ::android::hardware::sensors::V1_0::Event& event) {
    // TODO implement
    return ::android::hardware::sensors::V1_0::Result {};
}

Return<void> Sensors::registerDirectChannel(const ::android::hardware::sensors::V1_0::SharedMemInfo& mem, registerDirectChannel_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<::android::hardware::sensors::V1_0::Result> Sensors::unregisterDirectChannel(int32_t channelHandle) {
    // TODO implement
    return ::android::hardware::sensors::V1_0::Result {};
}

Return<void> Sensors::configDirectReport(int32_t sensorHandle, int32_t channelHandle, ::android::hardware::sensors::V1_0::RateLevel rate, configDirectReport_cb _hidl_cb) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ISensors* HIDL_FETCH_ISensors(const char* /* name */) {
    //return new Sensors();
//}
//
}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
