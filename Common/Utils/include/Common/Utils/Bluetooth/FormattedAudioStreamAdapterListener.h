#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTERLISTENER_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTERLISTENER_H_

#include "Common/Utils/AudioFormat.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {

/**
 * Interface to be implemented by class listening for data from @c FormattedAudioStreamAdapter
 */
class FormattedAudioStreamAdapterListener {
public:
    /**
     * Method to receive data sent by @c FormattedAudioStreamAdapter
     * @param audioFormat Audio format of the data sent
     * @param buffer Pointer to the buffer containing the data
     * @param size Length of the data in the buffer in bytes.
     */
    virtual void onFormattedAudioStreamAdapterData(
        common::utils::AudioFormat audioFormat,
        const unsigned char* buffer,
        size_t size) = 0;

    /**
     * Destructor.
     */
    virtual ~FormattedAudioStreamAdapterListener() = default;
};

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTERLISTENER_H_