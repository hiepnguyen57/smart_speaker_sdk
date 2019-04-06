#include <algorithm>
#include <cstring>
#include <string>
#include <unordered_map>

#include <Common/Utils/Bluetooth/BluetoothEvents.h>
#include <Common/Utils/Bluetooth/SDPRecords.h>
#include <Common/Utils/UUIDGeneration/UUIDGeneration.h>
#include <Common/Utils/Logger/Log.h>

#include "BlueZ/BlueZBluetoothDevice.h"
#include "BlueZ/BlueZConstants.h"
#include "BlueZ/BlueZDeviceManager.h"
#include "BlueZ/BlueZHostController.h"
//#include "BlueZ/MediaEndpoint.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

using namespace common::sdkInterfaces::bluetooth;
using namespace common::sdkInterfaces::bluetooth::services;
using namespace common::utils;
using namespace common::utils::bluetooth;
using namespace common::utils::logger;

// MediaTransport1 interface property "state"
static const char* MEDIATRANSPORT_PROPERTY_STATE = "State";

// DBus object root path
static const char* OBJECT_PATH_ROOT = "/";

// BlueZ codec id for SBC
static const int A2DP_CODEC_SBC = 0x00;

// Support all available capabilities for this byte
static const int SBC_CAPS_ALL = 0xff;

// Minimum bitpool size supported
static const int SBC_BITPOOL_MIN = 2;

// Maxmimum bitpool size supported
static const int SBC_BITPOOL_MAX = 64;

/**
 * DBus object path for the SINK media endpoint
 */
static const char* DBUS_ENDPOINT_PATH_SINK = "/com/device/sdk/sinkendpoint";

/**
 * BlueZ A2DP streaming state when audio data is streaming from the device, but we still did not acquire the file
 * descriptor.
 */
static const std::string STATE_PENDING = "pending";

/**
 * BlueZ A2DP streaming state when no audio data is streaming from the device.
 */
static const std::string STATE_IDLE = "idle";

/**
 * BlueZ A2DP streaming state when audio data is streaming from the device and we are reading it.
 */
static const std::string STATE_ACTIVE = "active";

#define TAG_BLUEZDEVICEMANAGER          "BlueZDeviceManager\t"

std::shared_ptr<BlueZDeviceManager> BlueZDeviceManager::create(
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus) {
    if(!eventBus) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "createFailed, reason: eventBus is nullptr";
        return nullptr;
    }

    std::shared_ptr<BlueZDeviceManager> deviceManager(new BlueZDeviceManager(eventBus));
    if(!deviceManager->init()) {
        return nullptr;
    }
    return deviceManager;
}

bool BlueZDeviceManager::init() {
    m_connection = DBusConnection::create();
    if(m_connection == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to create DBus connection";
        return false;
    }

    // Create ObjectManager proxy used to find Adapter to use and list of known Devices.
    m_objectManagerProxy = DBusProxy::create(BlueZConstants::OBJECT_MANAGER_INTERFACE, OBJECT_PATH_ROOT);
    if(m_objectManagerProxy == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to create ObjectManager proxy";
        return false;
    }

    if(!getStateFromBlueZ()) {
        return false;
    }

    m_hostController = initializeHostController();
    if(!m_hostController) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to initialize Host Controller";
        return false;
    }

    m_mediaProxy = DBusProxy::create(BlueZConstants::BLUEZ_MEDIA_INTERFACE, m_adapterPath);
    if(m_mediaProxy == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to create Media Proxy";
        return false;
    }

    m_workerContext = g_main_context_new();
    if(m_workerContext == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to create glib main context";
        return false;
    }

    m_eventLoop = g_main_loop_new(m_workerContext, false);
    if(m_eventLoop == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to create glib main loop";
        return false;
    }

    m_eventThread = std::thread(&BlueZDeviceManager::mainLoopThread, this);
    if(!m_mainLoopInitPromise.get_future().get()) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to initialize glib main loop";
        return false;
    }

    BluetoothDeviceManagerInitializedEvent event;
    m_eventBus->sendEvent(event);

    return true;
}

// Implement later
// bool BlueZDeviceManager::initializeMedia() {
//     // Create Media interface proxy to register MediaEndpoint
//     m_mediaEndpoint = std::make_shared<MediaEndpoint>(m_connection, DBUS_ENDPOINT_PATH_SINK);

//     if (!m_mediaEndpoint->registerWithDBus()) {
//         LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "registerEndpointFailed";
//         return false;
//     }

//     ManagedGError error;

//     // Creating a dictionary
//     GVariantBuilder* b;
//     GVariantBuilder* capBuilder;
//     capBuilder = g_variant_builder_new(G_VARIANT_TYPE("ay"));

//     // Channel Modes: Mono DualChannel Stereo JointStereo
//     // Frequencies: 16Khz 32Khz 44.1Khz 48Khz
//     g_variant_builder_add(capBuilder, "y", SBC_CAPS_ALL);

//     // Subbands: 4 8
//     // Blocks: 4 8 12 16
//     // Allocation mode: both
//     g_variant_builder_add(capBuilder, "y", SBC_CAPS_ALL);

//     // Bitpool Range: 2-64
//     g_variant_builder_add(capBuilder, "y", SBC_BITPOOL_MIN);
//     g_variant_builder_add(capBuilder, "y", SBC_BITPOOL_MAX);

//     GVariant* caps = g_variant_builder_end(capBuilder);

//     b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

//     std::string a2dpSinkUuid = A2DPSinkInterface::UUID;
//     std::transform(a2dpSinkUuid.begin(), a2dpSinkUuid.end(), a2dpSinkUuid.begin(), ::toupper);

//     g_variant_builder_add(b, "{sv}", "UUID", g_variant_new_string(a2dpSinkUuid.c_str()));
//     g_variant_builder_add(b, "{sv}", "Codec", g_variant_new_byte(A2DP_CODEC_SBC));
//     g_variant_builder_add(b, "{sv}", "Capabilities", caps);

//     // Second parameter
//     GVariant* parameters = g_variant_builder_end(b);

//     m_mediaProxy->callMethod(
//         "RegisterEndpoint", g_variant_new("(o@a{sv})", DBUS_ENDPOINT_PATH_SINK, parameters), error.toOutputParameter());

//     if (error.hasError()) {
//         LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to register MediaEndpoint";
//         return false;
//     }

//     return true;
// }

std::shared_ptr<BlueZBluetoothDevice> BlueZDeviceManager::getDeviceByPath(const std::string& path) const {
    {
        std::lock_guard<std::mutex> guard(m_devicesMutex);
        auto iter = m_devices.find(path);
        if(iter != m_devices.end()) {
            return iter->second;
        }
    }

    LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "getDeviceByPathFailed, reason: deviceNotFound";
    return nullptr;
}

void BlueZDeviceManager::onMediaStreamPropertyChanged(const std::string& path, const GVariantMapReader& changesMap) {
    const std::string FD_KEY = "/fd";

    // Get device path without the /fd number
    auto pos = path.find(FD_KEY);
    if(std::string::npos == pos) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "reason: unexpectedPath";
        return;
    }

    std::string devicePath = path.substr(0, pos);

    auto device = getDeviceByPath(devicePath);
    if(!device) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "deviceDoesNotExits";
        return;
    }

    auto mediaTransportProperties = DBusPropertiesProxy::create(path);
    if(!mediaTransportProperties) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "nullPropertiesProxy";
        return;
    }

    std::string uuid;
    if(!mediaTransportProperties->getStringProperty(BlueZConstants::BLUEZ_MEDIATRANSPORT_INTERFACE, "UUID", &uuid)) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "getPropertiesFailed";
        return;
    }

    std::transform(uuid.begin(), uuid.end(), uuid.begin(), ::tolower);

    char* newStateStr;
    common::utils::bluetooth::MediaStreamingState newState;
    if(changesMap.getCString(MEDIATRANSPORT_PROPERTY_STATE, &newStateStr)) {
        if(newStateStr == STATE_ACTIVE) {
            newState = common::utils::bluetooth::MediaStreamingState::ACTIVE;
        } else if(newStateStr == STATE_PENDING) {
            newState = common::utils::bluetooth::MediaStreamingState::PENDING;
        } else if(newStateStr == STATE_IDLE) {
            newState = common::utils::bluetooth::MediaStreamingState::IDLE;
        } else {
            LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "onMediaStreamPropertyChangedFailed, Unknown State";
            return;
        }
    }

    if(A2DPSourceInterface::UUID == uuid) {
        auto sink = device ->getA2DPSink();
        if(!sink) {
            LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "nullSink";
            return;
        }

        MediaStreamingStateChangedEvent event(newState, A2DPRole::SOURCE, device);
        m_eventBus->sendEvent(event);
        return;
    }
    // } else if(A2DPSinkInterface::UUID == uuid) {
    //     if (path != m_mediaEndpoint->getStreamingDevicePath()) {
    //         ACSDK_DEBUG5(LX(__func__)
    //                          .d("reason", "pathMismatch")
    //                          .d("path", path)
    //                          .d("streamingDevicePath", m_mediaEndpoint->getStreamingDevicePath()));
    //         return;
    //     }

    //     if (m_streamingState == newState) {
    //         return;
    //     }

    //     m_streamingState = newState;
    //     m_mediaEndpoint->onMediaTransportStateChanged(newState, path);

    //     MediaStreamingStateChangedEvent event(newState, bluetooth::A2DPRole::SINK, device);
    //     m_eventBus->sendEvent(event);
    // }

}

void BlueZDeviceManager::onDevicePropertyChanged(const std::string& path, const GVariantMapReader& changesMap) {
    std::shared_ptr<BlueZBluetoothDevice> device = getDeviceByPath(path);
    if(!device) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "onDevicePropertyChangedFailed, reason: device not found";
        return;
    }

    device->onPropertyChanged(changesMap);
}

void BlueZDeviceManager::onAdapterPropertyChanged(const std::string& path, const GVariantMapReader& changesMap) {
    if(!m_hostController) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "onAdapterPropertyChangedFailed, reason: nullHostController";
        return;
    }

    m_hostController->onPropertyChanged(changesMap);
}

std::string BlueZDeviceManager::getAdapterPath() const {
    return m_adapterPath;
}

void BlueZDeviceManager::interfacesAddedCallback(
    GDBusConnection* conn,
    const gchar* sender_name,
    const gchar* object_path,
    const gchar* interface_name,
    const gchar* signal_name,
    GVariant* parameters,
    gpointer deviceManager) {
    
    if(parameters == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "interfacesAddedCallbackFailed, reason: parameters is null";
        return;
    }

    if(deviceManager == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "interfacesAddedCallbackFailed, reason: deviceManager is null";
        return;
    }

    GVariantTupleReader tupleReader(parameters);
    char* addedObjectPath = tupleReader.getObjectPath(0);
    ManagedGVariant interfacesChangedMap = tupleReader.getVariant(1);

    static_cast<BlueZDeviceManager*>(deviceManager)->onInterfaceAdded(addedObjectPath, interfacesChangedMap);
}

void BlueZDeviceManager::interfacesRemovedCallback(
    GDBusConnection* conn,
    const gchar* sender_name,
    const gchar* object_path,
    const gchar* interface_name,
    const gchar* signal_name,
    GVariant* variant,
    gpointer deviceManager) {
    
    char* interfaceRemovedPath;

    if(variant == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "interfacesRemovedCallbackFailed, reason: variant is null";
        return;
    }

    if(deviceManager == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "interfacesRemovedCallbackFailed, reason: deviceManager is null";
        return;
    }

    g_variant_get(variant, "(oas)", &interfaceRemovedPath, NULL);

    static_cast<BlueZDeviceManager*>(deviceManager)->onInterfaceRemoved(interfaceRemovedPath);
}

void BlueZDeviceManager::propertiesChangedCallback(
    GDBusConnection* conn,
    const gchar* sender_name,
    const gchar* object_path,
    const gchar* interface_name,
    const gchar* signal_name,
    GVariant* prop,
    gpointer deviceManager) {

    if (prop == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "propertiesChangedCallbackFailed, reason: variant is null";
        return;
    }

    if (object_path == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "propertiesChangedCallbackFailed, reason: object_path is null";
        return;
    }

    if (deviceManager == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "propertiesChangedCallbackFailed, reason: deviceManager is null";
        return;
    }

    char* propertyOwner;

    GVariantTupleReader tupleReader(prop);
    propertyOwner = tupleReader.getCString(0);
    ManagedGVariant propertyMapVariant = tupleReader.getVariant(1);

    GVariantMapReader mapReader(propertyMapVariant);
    static_cast<BlueZDeviceManager*>(deviceManager)->onPropertiesChanged(propertyOwner, object_path, mapReader);
}

void BlueZDeviceManager::onPropertiesChanged(
    const std::string& propertyOwner,
    const std::string& objectPath,
    const GVariantMapReader& changesMap) {
    if(propertyOwner == BlueZConstants::BLUEZ_MEDIATRANSPORT_INTERFACE) {
        onMediaStreamPropertyChanged(objectPath, changesMap);
    } else if(propertyOwner == BlueZConstants::BLUEZ_DEVICE_INTERFACE) {
        onDevicePropertyChanged(objectPath, changesMap);
    } else if(propertyOwner == BlueZConstants::BLUEZ_ADAPTER_INTERFACE) {
        onAdapterPropertyChanged(objectPath, changesMap);
    }
}

void BlueZDeviceManager::onInterfaceAdded(const char* objectPath, ManagedGVariant& interfacesChangedMap) {
    // LOG_DEBUG << "onInterfaceAdded, path: " << objectPath;
    // LOG_DEBUG << "onInterfaceAdded, Details" << g_variant_print(interfacesChangedMap.get(), true);

    GVariantMapReader mapReader(interfacesChangedMap.get());
    ManagedGVariant deviceInterfaceObject = mapReader.getVariant(BlueZConstants::BLUEZ_DEVICE_INTERFACE);

    if(deviceInterfaceObject.get() != nullptr) {
        std::shared_ptr<BlueZBluetoothDevice> device = addDeviceFromDBusObject(objectPath, deviceInterfaceObject.get());
        notifyDeviceAdded(device);
    }
}

void BlueZDeviceManager::onInterfaceRemoved(const char* objectPath) {
    removeDevice(objectPath);
}

void BlueZDeviceManager::addDevice(const char* devicePath, std::shared_ptr<BlueZBluetoothDevice> device) {
    if(devicePath == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "addDeviceFailed, reason: devicePath is null";
    }
    if(device == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "addDeviceFailed, reason: device is null";
    }

    {
        std::lock_guard<std::mutex> guard(m_devicesMutex);
        m_devices[devicePath] = device;
    }
}

void BlueZDeviceManager::notifyDeviceAdded(std::shared_ptr<BlueZBluetoothDevice> device) {
    common::utils::bluetooth::DeviceDiscoveredEvent event(device);
    m_eventBus->sendEvent(event);
}

void BlueZDeviceManager::removeDevice(const char* devicePath) {
    if(devicePath == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "removeDeviceFailed, reason: devicePath is null";
    }

    std::shared_ptr<BluetoothDeviceInterface> device;
    {
        std::lock_guard<std::mutex> guard(m_devicesMutex);
        auto it = m_devices.find(devicePath);
        if(it != m_devices.end()) {
            device = it->second;
            m_devices.erase(it);
        }
    }

    if(device) {
        common::utils::bluetooth::DeviceRemovedEvent event(device);
        m_eventBus->sendEvent(event);
    }
}

void BlueZDeviceManager::doShutdown() {
    {
        std::lock_guard<std::mutex> guard(m_devicesMutex);

        //Disconnect every device.
        for(auto iter : m_devices) {
            std::shared_ptr<BlueZBluetoothDevice> device = iter.second;
            device->disconnect().get();
        }

        m_devices.clear();
    }

    // Destroy all objects requiring m_connection firts.
    //finalizeMedia();
    m_pairingAgent.reset();
    m_mediaPlayer.reset();

    m_connection->close();
    if(m_eventLoop) {
        g_main_loop_quit(m_eventLoop);
    }

    if(m_eventThread.joinable()) {
        m_eventThread.join();
    }
}

BlueZDeviceManager::~BlueZDeviceManager() {

}

BlueZDeviceManager::BlueZDeviceManager(
    const std::shared_ptr<common::utils::bluetooth::BluetoothEventBus>& eventBus) : 
        RequiresShutdown{"BlueZDeviceManager"},
        m_eventBus{eventBus},
        m_streamingState{common::utils::bluetooth::MediaStreamingState::IDLE} {

}

bool BlueZDeviceManager::getStateFromBlueZ() {
    ManagedGError error;
    ManagedGVariant managedObjectsVar = 
        m_objectManagerProxy->callMethod("GetManagedObjects", nullptr, error.toOutputParameter());

    if(error.hasError()) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "initializeKnownDevicesFailed";
        return false;
    }

    GVariantTupleReader resultReader(managedObjectsVar);
    ManagedGVariant managedObjectsMap = resultReader.getVariant(0);
    GVariantMapReader mapReader(managedObjectsMap, true);

    mapReader.forEach([this](char* objectPath, GVariant* dbusObject) -> bool {
        GVariantMapReader supportedInterfacesMap(dbusObject);

        // Check for Adapter if none found yet.
        if(m_adapterPath.empty()) {
            ManagedGVariant adapterInterfaceVar =
                supportedInterfacesMap.getVariant(BlueZConstants::BLUEZ_ADAPTER_INTERFACE);

            if(adapterInterfaceVar.hasValue()) {
                // Get HW Adapter Path here.
                m_adapterPath = objectPath;
            }
        }

        ManagedGVariant deviceInterfaceVar = supportedInterfacesMap.getVariant(BlueZConstants::BLUEZ_DEVICE_INTERFACE);
        if(deviceInterfaceVar.hasValue()) {
            //Found a known device
            auto device = addDeviceFromDBusObject(objectPath, deviceInterfaceVar.get());
        }

        return true;
    });

    return true;
}

std::shared_ptr<BlueZBluetoothDevice> BlueZDeviceManager::addDeviceFromDBusObject(
    const char* objectPath, 
    GVariant* dbusObject) {
    
    if(objectPath == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "addDeviceFromDBusObjectFailed, reason: objectPath is null";
    }
    if(dbusObject == nullptr) {
        LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "addDeviceFromDBusObjectFailed, reason: dbusObject is null";
    }

    GVariantMapReader deviceMapReader(dbusObject);
    char* macAddress = nullptr;

    if(!deviceMapReader.getCString(BlueZConstants::BLUEZ_DEVICE_INTERFACE_ADDRESS, &macAddress)) {
        // No mac address, ignore the device
        return nullptr;
    }

    std::shared_ptr<BlueZBluetoothDevice> knownDevice = 
        BlueZBluetoothDevice::create(macAddress, objectPath, shared_from_this());
    if(knownDevice) {
        addDevice(objectPath, knownDevice);
    }

    return knownDevice;

}

std::list<std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>> BlueZDeviceManager::
    getDiscoveredDevices() {
    std::list<std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>> newList;

    std::lock_guard<std::mutex> guard(m_devicesMutex);

    for(const auto& it : m_devices) {
        newList.push_back(
            std::static_pointer_cast<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>(it.second));
    }

    return newList;
}

std::shared_ptr<BlueZHostController> BlueZDeviceManager::initializeHostController() {
    return BlueZHostController::create(m_adapterPath);
}

// std::shared_ptr<MediaEndPoint> BlueZDeviceManager::getMediaEndpoint() {
//     return m_mediaEndpoint;
// }

std::shared_ptr<BluetoothHostControllerInterface> BlueZDeviceManager::getHostController() {
    return m_hostController;
}

std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> BlueZDeviceManager::getEventBus() {
    return m_eventBus;
}

// bool BlueZDeviceManager::finalizeMedia() {
//     ManagedGError error;

//     m_mediaProxy->callMethod(
//         "UnregisterEndpoint", g_variant_new("(o)", DBUS_ENDPOINT_PATH_SINK), error.toOutputParameter());

//     if (error.hasError()) {
//         return false;
//     }

//     m_mediaEndpoint.reset();

//     return true;
// }

void BlueZDeviceManager::mainLoopThread() {
    g_main_context_push_thread_default(m_workerContext);

    do {
        // New interace added
        // Used to track new devices found by BlueZ
        int subscriptionId = m_connection->subcribeToSignal(
            BlueZConstants::BLUEZ_SERVICE_NAME,
            BlueZConstants::OBJECT_MANAGER_INTERFACE,
            "InterfacesAdded",
            nullptr,
            BlueZDeviceManager::interfacesAddedCallback,
            this);
        
        if(subscriptionId == 0) {
            LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to subcribe to InterfacesAdded signal";
            m_mainLoopInitPromise.set_value(false);
            break;
        }

        // Used to track device removal
        subscriptionId = m_connection->subcribeToSignal(
            BlueZConstants::BLUEZ_SERVICE_NAME,
            BlueZConstants::OBJECT_MANAGER_INTERFACE,
            "InterfacesRemoved",
            nullptr,
            BlueZDeviceManager::interfacesRemovedCallback,
            this);

        if(subscriptionId == 0) {
            LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to subcribe to InterfacesRemoved signal";
            m_mainLoopInitPromise.set_value(false);
            break;
        }

        // Track properties changes
        subscriptionId = m_connection->subcribeToSignal(
            BlueZConstants::BLUEZ_SERVICE_NAME,
            BlueZConstants::PROPERTIES_INTERFACE,
            "PropertiesChanged",
            nullptr,
            BlueZDeviceManager::propertiesChangedCallback,
            this);

        if(subscriptionId == 0) {
            LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "Failed to subcribe to PropertiesChanged signal";
            m_mainLoopInitPromise.set_value(false);
            break;
        }

        // if (!initializeMedia()) {
        //     LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "initBluetoothMediaFailed";
        //     m_mainLoopInitPromise.set_value(false);
        //     break;
        // }

        m_pairingAgent = PairingAgent::create(m_connection);
        if(!m_pairingAgent) {
            LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "initPairingAgentFailed";
            m_mainLoopInitPromise.set_value(false);
            break;
        }

        // m_mediaPlayer = MPRISPlayer::create(m_connection, m_mediaProxy, m_eventBus);
        // if(!m_mediaPlayer) {
        //     LOG_ERROR << TAG_BLUEZDEVICEMANAGER << "initMediaPlayerFailed";
        //     m_mainLoopInitPromise.set_value(false);
        //     break;
        // }

        m_mainLoopInitPromise.set_value(true);

        g_main_loop_run(m_eventLoop);
    } while(false);
    g_main_loop_unref(m_eventLoop);
    g_main_context_pop_thread_default(m_workerContext);
    g_main_context_unref(m_workerContext);
}


} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK