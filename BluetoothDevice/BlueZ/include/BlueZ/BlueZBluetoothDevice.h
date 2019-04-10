#ifndef DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZBLUETOOTHDEVICE_H_
#define DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZBLUETOOTHDEVICE_H_

#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <gio/gio.h>

#include <Common/SDKInterfaces/Bluetooth/BluetoothDeviceInterface.h>
#include <Common/Utils/Threading/Executor.h>
#include <Common/Utils/Bluetooth/SDPRecords.h>
#include "BlueZ/DBusPropertiesProxy.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

class BlueZDeviceManager;

// A BlueZ implementation of the @c BluetoothDeviceInterface.
class BlueZBluetoothDevice
        : public common::sdkInterfaces::bluetooth::BluetoothDeviceInterface
        , public std::enable_shared_from_this<BlueZBluetoothDevice> {

public:
    enum class BlueZDeviceState {
        FOUND,
        UNPAIRED,
        PAIRED,
        IDLE,
        DISCONNECTED,
        CONNECTED,
        CONNECTION_FAILED
    };
    /// @name BluetoothDeviceInterface Functions
    /// @{
    ~BlueZBluetoothDevice() override;

    std::string getMac() const override;
    std::string getFriendlyName() const override;
    common::sdkInterfaces::bluetooth::DeviceState getDeviceState() override;

    bool isPaired() override;
    std::future<bool> pair() override;
    std::future<bool> unpair() override;

    bool isConnected() override;
    std::future<bool> connect() override;
    std::future<bool> disconnect() override;

    std::vector<std::shared_ptr<common::sdkInterfaces::bluetooth::services::SDPRecordInterface>>
    getSupportedServices() override;
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::A2DPSinkInterface> getA2DPSink() override;
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::A2DPSourceInterface> getA2DPSource() override;
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::AVRCPTargetInterface> getAVRCPTarget() override;
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::AVRCPControllerInterface> getAVRCPController() override;

    // Creates an instance of the BlueZBluetoothDevice.
    static std::shared_ptr<BlueZBluetoothDevice> create(
        const std::string& mac,
        const std::string& objectPath,
        std::shared_ptr<BlueZDeviceManager> deviceManager);
    
    // Get the DBus object path of the device.
    std::string getObjectPath() const;

    // A function for BlueZDeviceManager to alert the BlueZ device when its property has changed.
    void onPropertyChanged(const GVariantMapReader& changesMap);

private:
    // Constructor
    BlueZBluetoothDevice(
        const std::string& mac,
        const std::string& objectPath,
        std::shared_ptr<BlueZDeviceManager> deviceManager);

    bool init();

    bool updateFriendlyName();

    // Helper function to extract and parse the supported services for this device from the BlueZ device property.
    std::unordered_set<std::string> getServiceUuids();

    // Helper function to extract and parse the supported services based on the uuids.
    std::unordered_set<std::string> getServiceUuids(GVariant* array);

    // Create @c BluetoothServiceInterface for supported services based on uuids.
    bool initializeServices(const std::unordered_set<std::string>& uuids);

    // Pair with this device.
    bool executePair();

    // Unpair with this device
    bool executeUnpair();

    // Connect with this device.
    bool executeConnect();

    // Disconnect with this device
    bool executeDisconnect();

    // Helper function to check if paired
    bool executeIsPaired();

    // Helper function to check if connected
    bool executeIsConnected();

    // Queries BlueZ for the value of the property as reported by the adapter.
    bool queryDeviceProperty(const std::string& name, bool* value);

    // A function to convert the BlueZDeviceState to the normal DeviceState
    common::sdkInterfaces::bluetooth::DeviceState convertToDeviceState(BlueZDeviceState bluezDeviceState);

    // Transition to new state and optionally notify listeners.
    void transitionToState(BlueZDeviceState newState, bool sendEvent);

    // Helper function to check if a service exists in the @c m_servicesMap.
    bool serviceExists(const std::string& uuid);

    // Helper function to insert service into @c m_servicesMap.
    bool insertService(
        std::shared_ptr<common::sdkInterfaces::bluetooth::services::BluetoothServiceInterface> service);
    
    // Helper function to insert service into @c m_servicesMap.
    template<typename ServiceType>
    std::shared_ptr<ServiceType> getService();

    // Proxy to interact with the org.bluez.Device1 interface.
    std::shared_ptr<DBusProxy> m_deviceProxy;

    // Proxy to interact with the org.bluez.Device1 interface.
    std::shared_ptr<DBusPropertiesProxy> m_propertiesProxy;

    // The MAC address
    const std::string m_mac;

    // The DBus object path.
    const std::string m_objectPath;

    // The friendly name
    std::string m_friendlyName;

    // Mutex
    std::mutex m_servicesMapMutex;

    // A map of UUID to services.
    std::unordered_map<
        std::string,
        std::shared_ptr<common::sdkInterfaces::bluetooth::services::BluetoothServiceInterface>>
        m_servicesMap;

    // The current state of the device
    BlueZDeviceState m_deviceState;

    // The associated @c BlueZDeviceManager.
    std::shared_ptr<BlueZDeviceManager> m_deviceManager;

    // An executor used ofr serializing request on the Device's own thread of execution.
    common::utils::threading::Executor m_executor;
};

inline std::string deviceStateToString(BlueZBluetoothDevice::BlueZDeviceState state) {
    switch(state) {
        case BlueZBluetoothDevice::BlueZDeviceState::FOUND:
            return "FOUND";
        case BlueZBluetoothDevice::BlueZDeviceState::UNPAIRED:
            return "UNPAIRED";
        case BlueZBluetoothDevice::BlueZDeviceState::PAIRED:
            return "PAIRED";
        case BlueZBluetoothDevice::BlueZDeviceState::IDLE:
            return "IDLE";
        case BlueZBluetoothDevice::BlueZDeviceState::DISCONNECTED:
            return "DISCONNECTED";
        case BlueZBluetoothDevice::BlueZDeviceState::CONNECTED:
            return "CONNECTED";
        case BlueZBluetoothDevice::BlueZDeviceState::CONNECTION_FAILED:
            return "CONNECTION_FAILED";
    }
    return "UNKNOWN";
}

// Overload for the @c BlueZDeviceState enum. This will write the @c BlueZDeviceState as a string to the provided
inline std::ostream& operator<<(std::ostream& stream, const BlueZBluetoothDevice::BlueZDeviceState& state) {
    return stream << deviceStateToString(state);
}

} // namespace blueZ
} // namespace bluetoothDevice
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZBLUETOOTHDEVICE_H_