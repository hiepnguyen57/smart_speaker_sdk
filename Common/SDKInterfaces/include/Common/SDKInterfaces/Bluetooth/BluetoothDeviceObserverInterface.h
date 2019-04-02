#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEOBSERVERINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEOBSERVERINTERFACE_H_

#include <string>
#include <unordered_set>


namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {

/**
 * This interface allows derived class to know when a bluetooth device is connected or disconnected.
 */
class BluetoothDeviceObserverInterface {
public:
    /**
     * The observable attributes of the bluetooth device.
     */
    struct DeviceAttributes {
        /**
         * Constructor
         */
        DeviceAttributes() = default;

        // The name of the active bluetooth device.
        std::string name;

        // The bluetooth services this device supports.
        std::unordered_set<std::string> supportedServices;
    };
    
    /**
     * Destructor
     */
    virtual ~BluetoothDeviceObserverInterface() = default;

    /**
     * Used to notify the observer when an active bluetooth device is connected.
     * 
     * @return attributes The @c DeviceAttributes of the active bluetooth device.
     */
    virtual void onActiveDeviceConnected(const DeviceAttributes& attributes) = 0;

    /**
     * Used to notify the observer when an active bluetooth device is disconnected.
     * 
     * @return attributes The @c DeviceAttributes of the active bluetooth device.
     */
    virtual void onActiveDeviceDisconnected(const DeviceAttributes& attributes) = 0;

};

}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEOBSERVERINTERFACE_H_