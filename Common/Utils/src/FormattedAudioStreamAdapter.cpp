#include "Common/Utils/Bluetooth/FormattedAudioStreamAdapter.h"
#include "Common/Utils/Logger/Log.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {

using namespace deviveClientSDK::common::utils::logger;

#define TAG_FORMATTEDAUDIO          "FormattedAudioStreamAdapter\t"

FormattedAudioStreamAdapter::FormattedAudioStreamAdapter(const AudioFormat& audioFormat) {
    m_audioFormat = audioFormat;
}

AudioFormat FormattedAudioStreamAdapter::getAudioFormat() const {
    return m_audioFormat;
}

void FormattedAudioStreamAdapter::setListener(std::shared_ptr<FormattedAudioStreamAdapterListener> listener) {
    std::lock_guard<std::mutex> guard(m_readerFunctionMutex);
    m_listener = listener;
}

size_t FormattedAudioStreamAdapter::send(const unsigned char* buffer, size_t size) {
    if(!buffer) {
        LOG_ERROR << TAG_FORMATTEDAUDIO << "sendFailed. Reason: buffer is null";
        return 0;
    }
    if(size == 0) {
        LOG_ERROR << TAG_FORMATTEDAUDIO << "sendFailed. Reason: size is 0";
        return 0;
    }
    std::shared_ptr<FormattedAudioStreamAdapterListener> listener;
    {
        std::lock_guard<std::mutex> guard(m_readerFunctionMutex);
        listener = m_listener.lock();
    }

    if(listener) {
        listener->onFormattedAudioStreamAdapterData(m_audioFormat, buffer, size);
        return size;
    } else {
        return 0;
    }
}

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK