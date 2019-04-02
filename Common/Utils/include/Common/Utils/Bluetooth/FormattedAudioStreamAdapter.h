#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTER_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTER_H_

#include <memory>
#include <mutex>

#include "Common/Utils/AudioFormat.h"
#include "Common/Utils/Bluetooth/FormattedAudioStreamAdapterListener.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {

/**
 * A class providing the way to receive a sequence of audio data blocks and a format associated with it.
 */
class FormattedAudioStreamAdapter
{
public:
    /**
     * Constructor initializing the class with an @c AudioFormat.
     * @param audioFormat @c AudioFormat describing the data being sent.
     */
    explicit FormattedAudioStreamAdapter(const AudioFormat& audioFormat);

    /**
     * Get @c AudioFormat associated with the class.
     * 
     * @return @c AudioFormat asociated with the class.
     */
    AudioFormat getAudioFormat() const;

    /**
     * Set the listener to receive data.
     * 
     * @param listener the listener to receive data.
     */
    void setListener(std::shared_ptr<FormattedAudioStreamAdapterListener> listener);

    /**
     * Publish data to the listener.
     *
     * @param buffer Buffer containing the data
     * @param size Size of the data block in bytes. The value must be greater than zero.
     * @return number of bytes processed.
     */
    size_t send(const unsigned char* buffer, size_t size);

private:
    // The @c AudioFormat associated with the class.
    AudioFormat m_audioFormat;

    // the listener to receive data.
    std::weak_ptr<FormattedAudioStreamAdapterListener> m_listener;

    // Mutex to guard listener changes.
    std::mutex m_readerFunctionMutex;
};

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTER_H_