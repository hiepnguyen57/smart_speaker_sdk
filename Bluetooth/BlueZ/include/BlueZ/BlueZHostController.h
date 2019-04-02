#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZHOSTCONTROLLER_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZHOSTCONTROLLER_H_

#include <string>
#include <memory>
#include <mutex>

#include <gio/gio.h>

#include <Common/SDKInterfaces/Bluetooth/BluetoothHostControllerInterface.h>
#include <Common/Utils/MacAddressString.h>

#include "BlueZ/BlueZUtils.h"
#include "BlueZ/DBusPropertiesProxy.h"


namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

// An implementation of the @c BluetoothHostControllerInterface using BlueZ.
class BlueZHostController : public common::sdkInterfaces::bluetooth::BluetoothHostControllerInterface {
public:
    virtual ~BlueZHostController() = default;

    std::string getMac() const override;
    std::string getFriendlyName() const override;

    bool isDiscoverable() const override;
    std::future<bool> enterDiscoverableMode() override;
    std::future<bool> exitDiscoverableMode() override;

    bool isScanning() const override;
    std::future<bool> startScan() override;
    std::future<bool> stopScan() override;

    // Creates an instance of the BlueZHostController.
    static std::unique_ptr<BlueZHostController> create(const std::string& adapterObjectPath);

    // A function for BlueZDeviceManager to alert device when its property has changed.
    void onPropertyChanged(const GVariantMapReader& changesMap);

private:
    // Constructor.
    BlueZHostController(const std::string& adapterObjectPath);

    // Perform any needed initialization.
    bool init();

    // set Discoverable
    std::future<bool> setDiscoverable(bool discoverable);

    // adjust the scanning state of the adapter.
    std::future<bool> changeScanState(bool scanning);

    // The BlueZ object path of the adapter.
    std::string m_adapterObjectPath;

    // the MAC address of the adatper.
    std::unique_ptr<common::utils::MacAddressString> m_mac;

    mutable std::mutex m_adapterMutex;

    // The friendly name of the adapter.
    std::string m_friendlyName;

    // A proxy of the Adapter Properties interface.
    std::shared_ptr<DBusPropertiesProxy> m_adapterProperties;

    // A proxy of the Adapter interface.
    std::shared_ptr<DBusProxy> m_adapter;   
};

} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZHOSTCONTROLLER_H_