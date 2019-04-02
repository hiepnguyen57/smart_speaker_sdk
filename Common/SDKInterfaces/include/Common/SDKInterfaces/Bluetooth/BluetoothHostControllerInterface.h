#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHHOSTCONTROLLERINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHHOSTCONTROLLERINTERFACE_H_

#include <future>
#include <string>

namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {

/**
 * An interface to represent the HostControllerInterface on the local system.
 * This is responsible for Scanning and Discovery.
 */
class BluetoothHostControllerInterface {
public:
    // Destructor
    virtual ~BluetoothHostControllerInterface() = default;

    /**
     * Getter for the MAC address of the adapter.
     * 
     * @return the MAC address of the adapter.
     */
    virtual std::string getMac() const = 0;

    /**
     * Getter for the friendly name of the adapter.
     * 
     * @return the friendly name of the adapter.
     */
    virtual std::string getFriendlyName() const = 0;

    /**
     * Getter for the discoverability of the device.
     * 
     * @return Whether the device is current discoverable by other devices.
     */
    virtual bool isDiscoverable() const = 0;

    /**
     * Set the adapter to become discoverable.
     * 
     * @return Indicates whether the operation was succesful.
     */
    virtual std::future<bool> enterDiscoverableMode() = 0;

    /**
     * Set the adapter become non-discoverable.
     * 
     * @return Indicates whether the operation was succesful.
     */
    virtual std::future<bool> exitDiscoverableMode() = 0;

    /**
     * Getter for the scanning state of the device. This mus wait until
     * any prior startScan and StopScan methods have finished.
     * 
     * @return Whether the device is currently scanning for other device.
     */
    virtual bool isScanning() const = 0;

    /**
     * Set the adapter to start scanning
     * 
     * @return Indicates whether the operation was successful.
     */
    virtual std::future<bool> startScan() = 0;

    /**
     * Set the adapter to stop scanning
     * 
     * @return Indicates whether the operation was successful.
     */
    virtual std::future<bool> stopScan() = 0;
};

}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHHOSTCONTROLLERINTERFACE_H_