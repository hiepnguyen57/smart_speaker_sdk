#include "Common/SDKInterfaces/Bluetooth/Services/A2DPSinkInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/A2DPSourceInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/AVRCPControllerInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/AVRCPTargetInterface.h"

#include "Common/Utils/Bluetooth/SDPRecords.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {

using namespace common::sdkInterfaces::bluetooth::services;

SDPRecord::SDPRecord(const std::string& name, const std::string& uuid, const std::string& version) :
        m_name{name},
        m_uuid{uuid},
        m_version{version} {
}

std::string SDPRecord::getName() const {
    return m_name;
}

std::string SDPRecord::getUuid() const {
    return m_uuid;
}

std::string SDPRecord::getVersion() const {
    return m_version;
}

A2DPSourceRecord::A2DPSourceRecord(const std::string& version) :
        SDPRecord{A2DPSourceInterface::NAME, A2DPSourceInterface::UUID, version} {
}

A2DPSinkRecord::A2DPSinkRecord(const std::string& version) :
        SDPRecord{A2DPSinkInterface::NAME, A2DPSinkInterface::UUID, version} {
}

AVRCPTargetRecord::AVRCPTargetRecord(const std::string& version) :
        SDPRecord{AVRCPTargetInterface::NAME, AVRCPTargetInterface::UUID, version} {
}

AVRCPControllerRecord::AVRCPControllerRecord(const std::string& version) :
        SDPRecord{AVRCPControllerInterface::NAME, AVRCPControllerInterface::UUID, version} {
}

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK