#include <Common/Utils/Logger/Log.h>

#include "BlueZ/BlueZA2DPSink.h"
#include "BlueZ/BlueZDeviceManager.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {


using namespace common::utils;
using namespace common::sdkInterfaces::bluetooth::services;

std::shared_ptr<BlueZA2DPSink> BlueZA2DPSink::create() {
    return std::shared_ptr<BlueZA2DPSink>(new BlueZA2DPSink());
}

void BlueZA2DPSink::setup() {
}

void BlueZA2DPSink::cleanup() {
}

std::shared_ptr<SDPRecordInterface> BlueZA2DPSink::getRecord() {
    return m_record;
}

BlueZA2DPSink::BlueZA2DPSink() : m_record{std::make_shared<bluetooth::A2DPSinkRecord>("")} {
}

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK