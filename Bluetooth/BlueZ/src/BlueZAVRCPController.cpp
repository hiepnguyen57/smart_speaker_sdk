#include <Common/Utils/Logger/Log.h>
#include "BlueZ/BlueZAVRCPController.h"

namespace deviveClientSDK {
namespace bluetooth {
namespace blueZ {

using namespace common::utils;
using namespace common::sdkInterfaces::bluetooth::services;

std::shared_ptr<BlueZAVRCPController> BlueZAVRCPController::create() {
    return std::shared_ptr<BlueZAVRCPController>(new BlueZAVRCPController());
}

BlueZAVRCPController::BlueZAVRCPController() : m_record{std::make_shared<bluetooth::AVRCPControllerRecord>("")} {
}

std::shared_ptr<SDPRecordInterface> BlueZAVRCPController::getRecord() {
    return m_record;
}

void BlueZAVRCPController::setup() {
}

void BlueZAVRCPController::cleanup() {
}

} // namespace blueZ
} // namespce bluetooth
} // namespace deviveClientSDK
