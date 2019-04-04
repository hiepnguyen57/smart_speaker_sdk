#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZDEVICEMANAGER_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZDEVICEMANAGER_H_

#include <list>
#include <map>
#include <mutex>

#include <Common/SDKInterfaces/Bluetooth/BluetoothDeviceInterface.h>
#include <Common/SDKInterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>
#include <Common/SDKInterfaces/Bluetooth/BluetoothHostControllerInterface.h>
#include <Common/SDKInterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <Common/Utils/Bluetooth/BluetoothEventBus.h>
#include <Common/Utils/Bluetooth/BluetoothEvents.h>
#include <Common/Utils/RequiresShutdown.h>

#include "BlueZ/BlueZHostController.h"
#include "BlueZ/BlueZUtils.h"
#include "BlueZ/MPRISPlayer.h"
#include "BlueZ/PairingAgent.h"

#include <gio/gio.h>

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

//class MediaEndpoint;
class BlueZBluetoothDevice;

/**
 * Internal BlueZ implementation of @c BluetoothDeviceManagerInterface.
 */
class BlueZDeviceManager
    : public common::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface
    , public common::utils::RequiresShutdown
    , public std::enable_shared_from_this<BlueZDeviceManager> {
public:
    /**
     * Factory method to create a class.
     */
    static std::shared_ptr<BlueZDeviceManager> create(
        std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus);

    // Destructor.
    virtual ~BlueZDeviceManager() override;

    // name BluetoothDeviceManager Functions
    std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothHostControllerInterface> getHostController() override;
    std::list<std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>> getDiscoveredDevices() override;

    // requiresShutdown functions.
    void doShutdown() override;

    // Get the @c BluetoothEventBus used by this device manager to post bluetooth related events.
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> getEventBus();

    // Get the SINK @c MediaEndPoint associated with the device manager.
    // std::shared_ptr<MediaEndpoint> getMediaEndpoint();

    // Get the DBus object path of the current bluetooth hardware adapter used by this device manager.
    std::string getAdapterPath() const;

private:
    // Constructor
    explicit BlueZDeviceManager(const std::shared_ptr<common::utils::bluetooth::BluetoothEventBus>& eventBus);

    bool init();

    // Initialize A2DP streaming related components.
    // bool initializeMedia();

    // Finalize A2DP streaming related components.
    bool finalizeMedia();

    // Helper method to create a @c BlueZBluetoothDevice class instance from DBus object provided by BlueZ.
    std::shared_ptr<BlueZBluetoothDevice> addDeviceFromDBusObject(const char* objectPath, GVariant* dbusObject);

    // DBus callback called when BlueZ service has a new interface implemented by an object in a DBus object tree.
    static void interfacesAddedCallback(
        GDBusConnection* conn,
        const gchar* sender_name,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* signal_name,
        GVariant* parameters,
        gpointer data);
    
    // DBus callback called when BlueZ service loses an interface implementation.
    static void interfacesRemovedCallback(
        GDBusConnection* conn,
        const gchar* sender_name,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* signal_name,
        GVariant* parameters,
        gpointer data);
    
    // DBus callback called when BlueZ service has changed in one of its object.
    static void propertiesChangedCallback(
        GDBusConnection* conn,
        const gchar* sender_name,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* signal_name,
        GVariant* parameters,
        gpointer data);
    
    // Add device to list of known devices. This method is thread safe.
    void addDevice(const char* devicePath, std::shared_ptr<BlueZBluetoothDevice> device);

    // Remove device from list of known devices.
    void removeDevice(const char* devicePath);

    // Notifies @c EventBus listener of a new device added.
    void notifyDeviceAdded(std::shared_ptr<BlueZBluetoothDevice> device);

    // Handles the new interface being added to BlueZ objects.
    void onInterfaceAdded(const char* objectPath, ManagedGVariant& interfacesChangedMap);

    // Handles the removal of the DBus interfaces.
    void onInterfaceRemoved(const char* objectPath);

    // Handles the property values changes in the Adapter.
    void onAdapterPropertyChanged(const std::string& path, const GVariantMapReader& changesMap);

    // Handles the property values changes in BlueZ objects.
    void onDevicePropertyChanged(const std::string& path, const GVariantMapReader& changesMap);

    // Handles the A2DP media stream properties changes.
    void onMediaStreamPropertyChanged(const std::string& path, const GVariantMapReader& changesMap);

    // Initializes the @c BlueZHostController instance.
    std::shared_ptr<BlueZHostController> initializeHostController();

    // Handles the object's properties changes.
    void onPropertiesChanged(
        const std::string& propertyOwner,
        const std::string& objectPath,
        const GVariantMapReader& changesMap);
    
    // Retrieves the current state from BlueZ service and updates internal state accordingly.
    bool getStateFromBlueZ();

    // Get known device by its DBus object path
    std::shared_ptr<BlueZBluetoothDevice> getDeviceByPath(const std::string& path) const;

    // Thread procedure to setup and handle GLib events.
    void mainLoopThread();

    // DBus object path of the hardware bluetooth adapter used by device manager.
    std::string m_adapterPath;

    // DBus proxy for ObjectManager interface of the BlueZ service.
    std::shared_ptr<DBusProxy> m_objectManagerProxy;

    // DBus proxy for Media1 interface of the BlueZ service.
    std::shared_ptr<DBusProxy> m_mediaProxy;

    // List of known device
    std::map<std::string, std::shared_ptr<BlueZBluetoothDevice>> m_devices;

    // SINK media endpoint used for audio streaming
    // std::shared_ptr<MediaEndpoint> m_mediaEndpoint;

    // Pairing agent used for device paring.
    std::shared_ptr<PairingAgent> m_pairingAgent;

    // DBus MediaPlayer used for AVRCP target.
    std::shared_ptr<MPRISPlayer> m_mediaPlayer;

    // The EventBus to communicate with SDK componets
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> m_eventBus;

    /// Event loop to listen for signals.
    GMainLoop* m_eventLoop;

    // Glib context to run event loop in
    GMainContext* m_workerContext;

    // DBus connection
    std::shared_ptr<DBusConnection> m_connection;

    // Current streaming state
    common::utils::bluetooth::MediaStreamingState m_streamingState;

    // Mutex to synchronize known device list access.
    mutable std::mutex m_devicesMutex;

    // A host controller instance used by the device manager.
    std::shared_ptr<BlueZHostController> m_hostController;

    // Promise to hold the result of a glib's main loop thread initialization.
    std::promise<bool> m_mainLoopInitPromise;

    // Thread to run event listener on.
    std::thread m_eventThread;
};

} // namespace blueZ
} // namespce bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZDEVICEMANAGER_H_