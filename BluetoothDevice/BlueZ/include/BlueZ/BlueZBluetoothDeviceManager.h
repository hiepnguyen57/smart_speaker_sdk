#ifndef DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZBLUETOOTHDEVICEMANAGER_H_
#define DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZBLUETOOTHDEVICEMANAGER_H_

#include <Common/Utils/Bluetooth/BluetoothEventBus.h>
#include <Common/SDKInterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>
#include "BlueZ/BlueZDeviceManager.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

class BlueZBluetoothDeviceManager : public common::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface {
public:
    // A factory method to create instance class.
    static std::unique_ptr<BlueZBluetoothDeviceManager> create(
        std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus);

    virtual ~BlueZBluetoothDeviceManager() override;

    // BlueZBluetoothDeviceManager functions
    std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothHostControllerInterface> getHostController() override;
    std::list<std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>> getDiscoveredDevices()
        override;
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> getEventBus() override;

private:
    /**
     * A constructor
     *
     * @param eventBus event bus to communicate with SDK components
     */
    BlueZBluetoothDeviceManager(std::shared_ptr<BlueZDeviceManager> deviceManager);

    /// Pointer to the internal implementation of BluetoothDeviceManagerInterface
    std::shared_ptr<BlueZDeviceManager> m_deviceManager;
};

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZBLUETOOTHDEVICEMANAGER_H_