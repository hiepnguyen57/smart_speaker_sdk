#include <Common/Utils/Logger/Log.h>

#include "BlueZ/BlueZA2DPSource.h"
#include "BlueZ/BlueZDeviceManager.h"
//#include "BlueZ/MediaEndpoint.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

using namespace common::utils;
using namespace common::sdkInterfaces::bluetooth::services;
using namespace common::utils::logger;

std::shared_ptr<BlueZA2DPSource> BlueZA2DPSource::create(std::shared_ptr<BlueZDeviceManager> deviceManager) {
    if (nullptr == deviceManager) {
        LOG_ERROR << "createFailed, reason: deviceManager is null";
        return nullptr;
    }
    return std::shared_ptr<BlueZA2DPSource>(new BlueZA2DPSource(deviceManager));
}

std::shared_ptr<common::utils::bluetooth::FormattedAudioStreamAdapter> BlueZA2DPSource::getSourceStream() {

}

void BlueZA2DPSource::setup() {
}

void BlueZA2DPSource::cleanup() {
}

std::shared_ptr<SDPRecordInterface> BlueZA2DPSource::getRecord() {
    return m_record;
}

// BlueZA2DPSource::BlueZA2DPSource(std::shared_ptr<BlueZDeviceManager> deviceManager) :
//         m_record{std::make_shared<bluetooth::A2DPSourceRecord>("")},
//         m_deviceManager{deviceManager} {
// }

BlueZA2DPSource::BlueZA2DPSource(std::shared_ptr<BlueZDeviceManager> deviceManager) :  m_deviceManager{deviceManager} {
}
} // namespace blueZ
} // namespce bluetooth
} // namespace deviceClientSDK