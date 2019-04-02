#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSOURCEINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSOURCEINTERFACE_H_

#include <memory>

#include "Common/SDKInterfaces/Bluetooth/Services/BluetoothServiceInterface.h"
#include "Common/Utils/Bluetooth/FormattedAudioStreamAdapter.h"


namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {
namespace services {

/**
 * Interface to support A2DP streaming from bluetooth device to SDK
 */
class A2DPSourceInterface : public BluetoothServiceInterface {
public:
    /// The Service UUID.
    static constexpr const char* UUID = "0000110a-0000-1000-8000-00805f9b34fb";

    /// The Service Name.
    static constexpr const char* NAME = "AudioSource";

    /**
     * Returns the stream containing the decoded raw PCM data sent by the connected device.
     *
     * @return A shared_ptr to a @c FormattedAudioStreamAdapter object to be consumed.
     */
    virtual std::shared_ptr<common::utils::bluetooth::FormattedAudioStreamAdapter> getSourceStream() = 0;

    /**
     * Destructor.
     */
    virtual ~A2DPSourceInterface() = default;
};

}  // namespace services
}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSOURCEINTERFACE_H_