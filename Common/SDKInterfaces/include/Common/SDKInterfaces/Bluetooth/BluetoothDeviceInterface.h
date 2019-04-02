#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEINTERFACE_H_

#include <future>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "Common/SDKInterfaces/Bluetooth/Services/A2DPSourceInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/A2DPSinkInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/AVRCPControllerInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/AVRCPTargetInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/BluetoothServiceInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/SDPRecordInterface.h"

namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {

/**
 * Represent the state of the device. The state diagram is as follows:
 *   +------UNPAIRED-------------+
 *   |                           |
 *   +------UNPAIRED---+         |
 *   V                 |         |
 * FOUND -> PAIRED -> IDLE -> CONNECTED
 *                     ^             |
 *                     +DISCONNECTED-+
 */
enum class DeviceState {
    // A device has been discovered.
    FOUND,
    // THe device has been unpaired.
    UNPAIRED,
    // The device has successfully paired.
    PAIRED,
    // A paired device.
    IDLE,
    // A device has successfully disconnected.
    DISCONNECTED,
    // A device that has successfully connected.
    CONNECTED
};

/**
 * Convert the @c DeviceState to a string
 */
inline std::string deviceStateToString(DeviceState state) {
    switch (state)
    {
        case DeviceState::FOUND:
            return "FOUND";
        case DeviceState::UNPAIRED:
            return "UNPAIRED";
        case DeviceState::PAIRED:
            return "PAIRED";
        case DeviceState::IDLE:
            return "IDLE";
        case DeviceState::DISCONNECTED:
            return "DISCONNECTED";
        case DeviceState::CONNECTED:
            return "CONNECTED";
    }
    return "UNKNOWN";
}

/**
 * Overload for the @c DeviceState enum. This will write the @c DeviceState as a string to the provided stream.
 */
inline std::ostream& operator<<(std::ostream& stream, const DeviceState state) {
    return stream << deviceStateToString(state);
}

// Represent a Bluetooth Device
class BluetoothDeviceInterface {
public:
    // Destructor
    virtual ~BluetoothDeviceInterface() = default;

/**
     * Getter for the MAC address.
     *
     * @return The MAC address of the Bluetooth Device.
     */
    virtual std::string getMac() const = 0;

    /**
     * Getter for the friendly name.
     *
     * @return The friendly name of the Bluetooth Device.
     */
    virtual std::string getFriendlyName() const = 0;

    /**
     * Getter for the @c DeviceState.
     *
     * @return The @c DeviceState of the current device.
     */
    virtual DeviceState getDeviceState() = 0;

    /**
     * Getter for the paired state of the device. This should return
     * the state after any pending state changes have been resolved.
     *
     * @return A bool representing whether the device is paired.
     */
    virtual bool isPaired() = 0;

    /**
     * Initiate a pair with this device.
     *
     * @return Indicates whether pairing was successful.
     */
    virtual std::future<bool> pair() = 0;

    /**
     * Initiate an unpair with this device.
     *
     * @return Indicates whether the unpairing was successful.
     */
    virtual std::future<bool> unpair() = 0;

    /**
     * Getter for the paired state of the device. This should return
     * the state after any pending state changes have been resolved.
     *
     * @return A bool representing whether the device is connected.
     */
    virtual bool isConnected() = 0;

    /**
     * Initiate a connect with this device.
     *
     * @return Indicates whether connecting was successful.
     */
    virtual std::future<bool> connect() = 0;

    /**
     * Initiate a disconnect with this device.
     *
     * @return Indicates whether disconnect was successful.
     */
    virtual std::future<bool> disconnect() = 0;

    // @return The Bluetooth Services that this device supports.
    virtual std::vector<std::shared_ptr<services::SDPRecordInterface>> getSupportedServices() = 0;

    // @return A pointer to an instance of the @c AD2PSourceInterface if supported, else a nullptr.
    virtual std::shared_ptr<services::A2DPSourceInterface> getA2DPSource() = 0;

    // @return A pointer to an instance of the @c AVRCPTargetInterface if supported, else a nullptr.
    virtual std::shared_ptr<services::AVRCPTargetInterface> getAVRCPTarget() = 0;

    // @return A pointer to an instance of the @c AVRCPControllerInterface if supported, else a nullptr.
    virtual std::shared_ptr<services::AVRCPControllerInterface> getAVRCPController() = 0;
};

}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEINTERFACE_H_