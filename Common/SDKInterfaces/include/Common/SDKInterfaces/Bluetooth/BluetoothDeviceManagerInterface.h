#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEMANAGERINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEMANAGERINTERFACE_H_

#include <list>
#include <memory>

#include "Common/SDKInterfaces/Bluetooth/BluetoothDeviceInterface.h"
#include "Common/SDKInterfaces/Bluetooth/BluetoothHostControllerInterface.h"
#include "Common/Utils/Bluetooth/BluetoothEventBus.h"

namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {

class BluetoothDeviceManagerInterface {
public:
    /**
     * Destructor.
     */
    virtual ~BluetoothDeviceManagerInterface() = default;

    // Get @c BluetoothhostControllerInterface instance.
    virtual std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothHostControllerInterface>
    getHostController() = 0;

    /**
     * Get a list of device the Host Controller is aware of. This list must contain
     * i, Paired devices.
     * ii, Devices found during the scaning process.
     */
    virtual std::list<std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>>
    getDiscoveredDevices() = 0;

    /**
     * Get the @c BluetoothEventBus used by this device manager to post bluetooth related events.
     */
    virtual std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> getEventBus() = 0;

};
}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEMANAGERINTERFACE_H_