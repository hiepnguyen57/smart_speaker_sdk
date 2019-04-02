#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPCONTROLLERINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPCONTROLLERINTERFACE_H_

#include "Common/SDKInterfaces/Bluetooth/Services/BluetoothServiceInterface.h"

namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {
namespace services {

//Used to implement an instance of AVRCPController (Audio/Video Remote Control Profile).
class AVRCPControllerInterface : public BluetoothServiceInterface {
public:
    /**
     * The Service UUID.
     * 110e is the legacy UUID used as both the identifier for the AVRCP Profile as well as the AVRCP Controller service
     * before v1.3. 110f is the UUID used for AVRCP Controller service in newer versions of AVRCP.
     * However, the 110e record must always be present, in later versions of AVRCP for backwards compabitibility.
     * We will use 110e as the identifying record.
     */
    static constexpr const char* UUID = "0000110e-0000-1000-8000-00805f9b34fb";

    /// The Service Name.
    static constexpr const char* NAME = "A/V_RemoteControl";

    /**
     * Destructor.
     */
    virtual ~AVRCPControllerInterface() = default;   
};

}  // namespace services
}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPCONTROLLERINTERFACE_H_