#include <Common/Utils/Logger/Log.h>
#include "BlueZ/BlueZAVRCPTarget.h"

namespace deviveClientSDK {
namespace bluetooth {
namespace blueZ {

using namespace common::utils;
using namespace common::sdkInterfaces::bluetooth::services;

/// The AVRCP Play command.
static const std::string PLAY_CMD = "Play";

/// The AVRCP Play command.
static const std::string PAUSE_CMD = "Pause";

/// The AVRCP Play command.
static const std::string NEXT_CMD = "Next";

/// The AVRCP Play command.
static const std::string PREVIOUS_CMD = "Previous";

std::shared_ptr<BlueZAVRCPTarget> BlueZAVRCPTarget::create(std::shared_ptr<DBusProxy> mediaControlProxy) {
    if (!mediaControlProxy) {
        return nullptr;
    }

    return std::shared_ptr<BlueZAVRCPTarget>(new BlueZAVRCPTarget(mediaControlProxy));
}

BlueZAVRCPTarget::BlueZAVRCPTarget(std::shared_ptr<DBusProxy> mediaControlProxy) :
        m_record{std::make_shared<bluetooth::AVRCPTargetRecord>("")},
        m_mediaControlProxy{mediaControlProxy} {
}

std::shared_ptr<SDPRecordInterface> BlueZAVRCPTarget::getRecord() {
    return m_record;
}

void BlueZAVRCPTarget::setup() {
}

void BlueZAVRCPTarget::cleanup() {
}

bool BlueZAVRCPTarget::play() {
    std::lock_guard<std::mutex> lock(m_cmdMutex);

    ManagedGError error;
    m_mediaControlProxy->callMethod(PLAY_CMD, nullptr, error.toOutputParameter());

    if (error.hasError()) {
        return false;
    }

    return true;
}

bool BlueZAVRCPTarget::pause() {
    std::lock_guard<std::mutex> lock(m_cmdMutex);

    ManagedGError error;
    m_mediaControlProxy->callMethod(PAUSE_CMD, nullptr, error.toOutputParameter());

    if (error.hasError()) {
        return false;
    }

    return true;
}

bool BlueZAVRCPTarget::next() {
    std::lock_guard<std::mutex> lock(m_cmdMutex);

    ManagedGError error;
    m_mediaControlProxy->callMethod(NEXT_CMD, nullptr, error.toOutputParameter());

    if (error.hasError()) {
        return false;
    }

    return true;
}

bool BlueZAVRCPTarget::previous() {
    std::lock_guard<std::mutex> lock(m_cmdMutex);

    ManagedGError error;
    m_mediaControlProxy->callMethod(PREVIOUS_CMD, nullptr, error.toOutputParameter());

    if (error.hasError()) {
        return false;
    }

    return true;
}

} // namespace blueZ
} // namespce bluetooth
} // namespace deviveClientSDK