// FIXME: your file license if you have one

#include "Sensors.h"

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

// Methods from ::android::hardware::sensors::V2_0::ISensors follow.
Return<void> Sensors::getSensorsList(getSensorsList_cb _hidl_cb)
{
    (void) _hidl_cb;
    // TODO implement
    return Void();
}

Return<V1_0::Result> Sensors::setOperationMode(V1_0::OperationMode mode)
{
    (void) mode;
    // TODO implement
    return V1_0::Result {};
}

Return<V1_0::Result> Sensors::activate(int32_t sensorHandle,
                                       bool enabled)
{
    (void) sensorHandle;
    (void) enabled;
    // TODO implement
    return ::android::hardware::sensors::V1_0::Result {};
}

Return<V1_0::Result> Sensors::initialize(const MQDescriptorSync<V1_0::Event>& eventQueueDescriptor,
                                         const MQDescriptorSync<uint32_t>& wakeLockDescriptor,
                                         const sp<ISensorsCallback>& sensorsCallback)
{
    (void) eventQueueDescriptor;
    (void) wakeLockDescriptor;
    (void) sensorsCallback;
    // TODO implement
    return V1_0::Result {};
}

Return<V1_0::Result> Sensors::batch(int32_t sensorHandle,
                                    int64_t samplingPeriodNs,
                                    int64_t maxReportLatencyNs)
{
    (void) sensorHandle;
    (void) samplingPeriodNs;
    (void) maxReportLatencyNs;
    // TODO implement
    return V1_0::Result {};
}

Return<V1_0::Result> Sensors::flush(int32_t sensorHandle)
{
    (void) sensorHandle;
    // TODO implement
    return V1_0::Result {};
}

Return<V1_0::Result> Sensors::injectSensorData(const V1_0::Event& event)
{
    (void) event;
    // TODO implement
    return V1_0::Result {};
}

Return<void> Sensors::registerDirectChannel(const V1_0::SharedMemInfo& mem,
                                            registerDirectChannel_cb _hidl_cb)
{
    (void) mem;
    (void) _hidl_cb;
    // TODO implement
    return Void();
}

Return<V1_0::Result> Sensors::unregisterDirectChannel(int32_t channelHandle)
{
    (void) channelHandle;
    // TODO implement
    return V1_0::Result {};
}

Return<void> Sensors::configDirectReport(int32_t sensorHandle,
                                         int32_t channelHandle,
                                         V1_0::RateLevel rate,
                                         configDirectReport_cb _hidl_cb)
{
    (void) sensorHandle;
    (void) channelHandle;
    (void) rate;
    (void) _hidl_cb;
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
