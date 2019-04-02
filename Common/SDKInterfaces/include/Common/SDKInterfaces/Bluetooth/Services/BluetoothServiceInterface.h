#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTHSERVICEINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTHSERVICEINTERFACE_H_

#include <memory>

#include "Common/SDKInterfaces/Bluetooth/Services/SDPRecordInterface.h"

namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {
namespace services {

// Interface representing a BluetoothService.
class BluetoothServiceInterface {
public:
    /**
     * Returns an SPDRecord for the service.
     * 
     * @return A @c SDPRecordInterface for the service.
     */
    virtual std::shared_ptr<SDPRecordInterface> getRecord() = 0;

    // Destructor
    virtual ~BluetoothServiceInterface() = default;

    // Called for any necessary setup of the service.
    virtual void setup() = 0;

    // Called for any necessary cleanup of the service.
    virtual void cleanup() = 0;
};

}  // namespace services
}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTHSERVICEINTERFACE_H_