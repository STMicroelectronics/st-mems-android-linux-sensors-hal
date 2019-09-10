// FIXME: your file license if you have one

#pragma once

#include <android/hardware/sensors/2.0/ISensors.h>
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

struct Sensors : public ISensors {
    // Methods from ::android::hardware::sensors::V2_0::ISensors follow.
    Return<void> getSensorsList(getSensorsList_cb _hidl_cb) override;
    Return<V1_0::Result> setOperationMode(V1_0::OperationMode mode) override;
    Return<V1_0::Result> activate(int32_t sensorHandle,
                                  bool enabled) override;
    Return<V1_0::Result> initialize(const MQDescriptorSync<V1_0::Event>& eventQueueDescriptor,
                                    const MQDescriptorSync<uint32_t>& wakeLockDescriptor,
                                    const sp<V2_0::ISensorsCallback>& sensorsCallback) override;
    Return<V1_0::Result> batch(int32_t sensorHandle,
                               int64_t samplingPeriodNs,
                               int64_t maxReportLatencyNs) override;
    Return<V1_0::Result> flush(int32_t sensorHandle) override;
    Return<V1_0::Result> injectSensorData(const V1_0::Event& event) override;
    Return<void> registerDirectChannel(const V1_0::SharedMemInfo& mem,
                                       registerDirectChannel_cb _hidl_cb) override;
    Return<V1_0::Result> unregisterDirectChannel(int32_t channelHandle) override;
    Return<void> configDirectReport(int32_t sensorHandle,
                                    int32_t channelHandle,
                                    V1_0::RateLevel rate,
                                    configDirectReport_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
