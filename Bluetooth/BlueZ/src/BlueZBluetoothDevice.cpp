#include <Common/Utils/Bluetooth/SDPRecords.h>
#include <Common/Utils/Logger/Log.h>
#include "BlueZ/BlueZA2DPSink.h"
#include "BlueZ/BlueZA2DPSource.h"
#include "BlueZ/BlueZAVRCPController.h"
#include "BlueZ/BlueZAVRCPTarget.h"
#include "BlueZ/BlueZConstants.h"
#include "BlueZ/BlueZDeviceManager.h"

#include "BlueZ/BlueZBluetoothDevice.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

using namespace common::utils;
using namespace common::utils::bluetooth;
using namespace common::sdkInterfaces::bluetooth;
using namespace common::sdkInterfaces::bluetooth::services;
using namespace common::utils::logger;

#define TAG_BLUEZBLUETOOTHDEVICE            "BlueZBluetoothDevice\t"

// The Name property that BlueZ uses.
static const std::string BLUEZ_DEVICE_PROPERTY_ALIAS = "Alias";

// The UUID property that BlueZ uses.
static const std::string BLUEZ_DEVICE_PROPERTY_UUIDS = "UUIDs";

// An BlueZ error indicating when an object no longer exists.
static const std::string BLUEZ_ERROR_NOTFOUND = "org.bluez.Error.DoesNotExist";

// A BlueZ connect error indicating authentication was rejected.
static const std::string BLUEZ_ERROR_RESOURCE_UNAVAILABLE = "org.bluez.Error.Failed: Resource temporarily unavailable";

// BlueZ org.bluez.Device1 method to pair.
static const std::string BLUEZ_DEVICE_METHOD_PAIR = "Pair";

// BlueZ org.bluez.Device1 method to connect.
static const std::string BLUEZ_DEVICE_METHOD_CONNECT = "Connect";

// BlueZ org.bluez.Device1 method to disconnect.
static const std::string BLUEZ_DEVICE_METHOD_DISCONNECT = "Disconnect";

// BlueZ org.bluez.Device1 paired property.
static const std::string BLUEZ_DEVICE_PROPERTY_PAIRED = "Paired";

// BlueZ org.bluez.Device1 connected property.
static const std::string BLUEZ_DEVICE_PROPERTY_CONNECTED = "Connected";

// BlueZ org.bluez.Adapter1 method to remove device.
static const std::string BLUEZ_ADAPTER_REMOVE_DEVICE = "RemoveDevice";

// The Media Control interface on the DBus object.
static const std::string MEDIA_CONTROL_INTERFACE = "org.bluez.MediaControl1";

std::shared_ptr<BlueZBluetoothDevice> BlueZBluetoothDevice::create(
    const std::string& mac,
    const std::string& objectPath,
    std::shared_ptr<BlueZDeviceManager> deviceManager) {

    if(!g_variant_is_object_path(objectPath.c_str())) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: invalidObjectPath";
        return nullptr;
    }

    auto device = std::shared_ptr<BlueZBluetoothDevice>(new BlueZBluetoothDevice(mac, objectPath, deviceManager));
    if(!device->init()) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: initFailed";
        return nullptr;
    }

    return device;
}

BlueZBluetoothDevice::BlueZBluetoothDevice(
    const std::string& mac,
    const std::string& objectPath,
    std::shared_ptr<BlueZDeviceManager> deviceManager) :
        m_mac{mac},
        m_objectPath{objectPath},
        m_deviceState{BlueZDeviceState::FOUND},
        m_deviceManager{deviceManager} {
}

std::string BlueZBluetoothDevice::getMac() const {
    return m_mac;
}

std::string BlueZBluetoothDevice::getFriendlyName() const {
    return m_friendlyName;
}

bool BlueZBluetoothDevice::updateFriendlyName() {
    if(!m_propertiesProxy->getStringProperty(
        BlueZConstants::BLUEZ_DEVICE_INTERFACE, BLUEZ_DEVICE_PROPERTY_ALIAS, &m_friendlyName)) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: getNameFailed";
        return false;
    }
    return true;
}

BlueZBluetoothDevice::~BlueZBluetoothDevice() {
    m_executor.shutdown();

    {
        std::lock_guard<std::mutex> lock(m_servicesMapMutex);

        for (auto& entry : m_servicesMap) {
            entry.second->cleanup();
        }
        m_servicesMap.clear();
    }
}

bool BlueZBluetoothDevice::init() {
    m_deviceProxy = DBusProxy::create(BlueZConstants::BLUEZ_DEVICE_INTERFACE, m_objectPath);
    if(!m_deviceProxy) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: createDeviceProxyFailed";
        return false;
    }

    m_propertiesProxy = DBusPropertiesProxy::create(m_objectPath);
    if(!m_propertiesProxy) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: createPropertyProxyFailed";
        return false;        
    }

    updateFriendlyName();

    bool isPaired = false;
    if(queryDeviceProperty(BLUEZ_DEVICE_PROPERTY_PAIRED, &isPaired) && isPaired) {
        m_deviceState = BlueZDeviceState::IDLE;
    }

    // Parse UUIDs and find versions.
    if(!initializeServices(getServiceUuids())) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: initializeServicesFailed";
        return false;
    }
    return true;
}

bool BlueZBluetoothDevice::initializeServices(const std::unordered_set<std::string>& uuids) {
    for(const auto& uuid : uuids) {
        if(A2DPSourceInterface::UUID == uuid && !serviceExists(uuid)) {
            auto a2dpSource = BlueZA2DPSource::create(m_deviceManager);
            if(!a2dpSource) {
                LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: createA2DPFailed";
                return false;
            } else {
                a2dpSource->setup();
                //insertService(a2dpSource);
            }
        } else if(AVRCPTargetInterface::UUID == uuid && !serviceExists(uuid)) {
            auto mediaControlProxy = DBusProxy::create(MEDIA_CONTROL_INTERFACE, m_objectPath);
            if(!mediaControlProxy) {
                LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: nullMediaControlProxy";
                return false;               
            }

            auto avrcpTarget = BlueZAVRCPTarget::create(mediaControlProxy);
            if(!avrcpTarget) {
                LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: createAVRCPTargetFailed";
                return false;
            } else {
                avrcpTarget->setup();
                //insertService(avrcpTarget);
            }
        } else if(A2DPSinkInterface::UUID == uuid && !serviceExists(uuid)) {
            auto a2dpSink = BlueZA2DPSink::create();
            if(!a2dpSink) {
                LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: createA2DPSinkFailed";
                return false;
            } else {
                a2dpSink->setup();
                //insertService(a2dpSink);
            }
        } else if(AVRCPControllerInterface::UUID == uuid && !serviceExists(uuid)) {
            auto avrcpController = BlueZAVRCPController::create();
            if (!avrcpController) {
                LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: createAVRCPControllerFailed";
                return false;
            } else {
                avrcpController->setup();
                //insertService(avrcpController);
            }           
        }
    }
    return true;
}

bool BlueZBluetoothDevice::isPaired() {
    auto future = m_executor.submit([this] { return executeIsPaired(); });
    if(future.valid()) {
        return future.get();
    } else {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason:  invalidFuture";
        return false;
    }
}

bool BlueZBluetoothDevice::executeIsPaired() {
    return BlueZDeviceState::UNPAIRED != m_deviceState && BlueZDeviceState::FOUND != m_deviceState;
}

std::future<bool> BlueZBluetoothDevice::pair() {
    return m_executor.submit([this] { return executePair(); });
}

bool BlueZBluetoothDevice::executePair() {
    ManagedGError error;
    m_deviceProxy->callMethod(BLUEZ_DEVICE_METHOD_PAIR, nullptr, error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: " << error.getMessage();
        return false;
    }
    return true;
}

std::future<bool> BlueZBluetoothDevice::unpair() {
    return m_executor.submit([this] { return executeUnpair(); });
}

bool BlueZBluetoothDevice::executeUnpair() {
    ManagedGError error;

    auto adapterProxy = DBusProxy::create(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, m_deviceManager->getAdapterPath());
    if(!adapterProxy) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: createAdapterProxyFailed";
        return false;
    }

    adapterProxy->callMethod(
        BLUEZ_ADAPTER_REMOVE_DEVICE, g_variant_new("(o)", m_objectPath.c_str()), error.toOutputParameter());

    if(error.hasError()) {
        std::string errorMsg = error.getMessage();

        if(std::string::npos != errorMsg.find(BLUEZ_ERROR_NOTFOUND)) {
            return true;
        }
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: " << errorMsg;
        return false;
    }
    return true;
}

std::string BlueZBluetoothDevice::getObjectPath() const {
    return m_objectPath;
}

std::unordered_set<std::string> BlueZBluetoothDevice::getServiceUuids(GVariant* array) {
    std::unordered_set<std::string> uuids;

    if (!array) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: nullArray";
        return uuids;
    } else if (!g_variant_is_of_type(array, G_VARIANT_TYPE_ARRAY)) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: invalidType, type: " << g_variant_get_type_string(array);
        return uuids;
    }

    GVariantTupleReader arrayReader(array);
    arrayReader.forEach([&uuids](GVariant* variant) {
        if (!variant) {
            LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "iteratingArrayFailed, reason: nullVariant";
            return false;
        }
        // Do not free, this is not allocated.
        const gchar* temp = g_variant_get_string(variant, NULL);
        std::string uuid(temp);
        uuids.insert(uuid);
        return true;
    });

    return uuids;
}

std::unordered_set<std::string> BlueZBluetoothDevice::getServiceUuids() {

    // DBus returns this as (a{v},). We have to drill into the tuple to retrieve the array.
    ManagedGVariant uuidsTuple;
    if (!m_propertiesProxy->getVariantProperty(
            BlueZConstants::BLUEZ_DEVICE_INTERFACE, BLUEZ_DEVICE_PROPERTY_UUIDS, &uuidsTuple)) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: getVariantPropertyFailed";
        return std::unordered_set<std::string>();
    }

    GVariantTupleReader tupleReader(uuidsTuple);
    ManagedGVariant array = tupleReader.getVariant(0).unbox();

    if (!array.hasValue()) {
        // The format isn't what we were expecting. Print the original tuple for debugging.
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: unexpectedVariantFormat; variant: " << uuidsTuple.dumpToString(false);
        return std::unordered_set<std::string>();
    }

    return getServiceUuids(array.get());
}

bool BlueZBluetoothDevice::isConnected() {
    auto future = m_executor.submit([this] { return executeIsConnected(); });

    if (future.valid()) {
        return future.get();
    } else {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "reason: invalidFuture; action: defaultingFalse";
        return false;
    }
}

bool BlueZBluetoothDevice::executeIsConnected() {
    return BlueZDeviceState::CONNECTED == m_deviceState;
}

std::future<bool> BlueZBluetoothDevice::connect() {
    return m_executor.submit([this] { return executeConnect(); });
}

bool BlueZBluetoothDevice::executeConnect() {
    if(executeIsConnected()) {
        return true;
    }

    ManagedGError error;
    m_deviceProxy->callMethod(BLUEZ_DEVICE_METHOD_CONNECT, nullptr, error.toOutputParameter());
    if(error.hasError()) {
        std::string errStr = error.getMessage() ? error.getMessage() : "";
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: " << errStr;

        // This indicates an issue with authentication, likely the other device has unpaired.
        if (std::string::npos != errStr.find(BLUEZ_ERROR_RESOURCE_UNAVAILABLE)) {
            transitionToState(BlueZDeviceState::CONNECTION_FAILED, false);
        }
        return false;        
    }

    /*
     * If the current state is CONNECTION_FAILED, another Connected = true property changed signal may not appear.
     * We'll transition to the CONNECTED state directly here. If that signal does come, we simply
     * ignore it because there's no transition when you're already CONNECTED and you see a Connected = true.
     */
    if (BlueZDeviceState::CONNECTION_FAILED == m_deviceState) {
        transitionToState(BlueZDeviceState::CONNECTED, true);
    }

    return true;
}

std::future<bool> BlueZBluetoothDevice::disconnect() {
    return m_executor.submit([this] { return executeDisconnect(); });
}

bool BlueZBluetoothDevice::executeDisconnect() {
    ManagedGError error;
    m_deviceProxy->callMethod(BLUEZ_DEVICE_METHOD_DISCONNECT, nullptr, error.toOutputParameter());

    if (error.hasError()) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: " << error.getMessage();
        return false;
    }

    return true;
}

std::vector<std::shared_ptr<SDPRecordInterface>> BlueZBluetoothDevice::getSupportedServices() {
    std::vector<std::shared_ptr<SDPRecordInterface>> services;

    {
        std::lock_guard<std::mutex> lock(m_servicesMapMutex);

        for (auto& it : m_servicesMap) {
            services.push_back(it.second->getRecord());
        }
    }

    return services;
}

bool BlueZBluetoothDevice::serviceExists(const std::string& uuid) {
    std::lock_guard<std::mutex> lock(m_servicesMapMutex);
    return m_servicesMap.count(uuid) != 0;
}

bool BlueZBluetoothDevice::insertService(std::shared_ptr<BluetoothServiceInterface> service) {
    if (!service) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: nullService";
        return false;
    }

    std::shared_ptr<SDPRecordInterface> record = service->getRecord();

    if (!record) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: nullRecord";
        return false;
    }

    bool success = false;
    {
        std::lock_guard<std::mutex> lock(m_servicesMapMutex);
        success = m_servicesMap.insert({record->getUuid(), service}).second;
    }

    if (!success) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: serviceAlreadyExists";
    }

    return success;
}

template <typename ServiceType>
std::shared_ptr<ServiceType> BlueZBluetoothDevice::getService() {
    std::shared_ptr<ServiceType> service = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_servicesMapMutex);
        auto it = m_servicesMap.find(ServiceType::UUID);
        if (it == m_servicesMap.end()) {
            LOG_DEBUG << TAG_BLUEZBLUETOOTHDEVICE << "reason: serviceNotFound";
        } else {
            // We completely control the types these are going to be, so avoid the overhead of dynamic_pointer_cast.
            service = std::static_pointer_cast<ServiceType>(it->second);
        }
    }

    return service;
}

std::shared_ptr<A2DPSinkInterface> BlueZBluetoothDevice::getA2DPSink() {
    return getService<A2DPSinkInterface>();
}

std::shared_ptr<A2DPSourceInterface> BlueZBluetoothDevice::getA2DPSource() {
    return getService<A2DPSourceInterface>();
}

std::shared_ptr<AVRCPTargetInterface> BlueZBluetoothDevice::getAVRCPTarget() {
    return getService<AVRCPTargetInterface>();
}

std::shared_ptr<AVRCPControllerInterface> BlueZBluetoothDevice::getAVRCPController() {
    return getService<AVRCPControllerInterface>();
}

DeviceState BlueZBluetoothDevice::getDeviceState() {
    return m_executor.submit([this] { return convertToDeviceState(m_deviceState); }).get();
}

common::sdkInterfaces::bluetooth::DeviceState BlueZBluetoothDevice::convertToDeviceState(
    BlueZDeviceState bluezDeviceState) {
    switch (bluezDeviceState) {
        case BlueZDeviceState::FOUND:
            return DeviceState::FOUND;
        case BlueZDeviceState::UNPAIRED:
            return DeviceState::UNPAIRED;
        case BlueZDeviceState::PAIRED:
            return DeviceState::PAIRED;
        case BlueZDeviceState::CONNECTION_FAILED:
        case BlueZDeviceState::IDLE:
            return DeviceState::IDLE;
        case BlueZDeviceState::DISCONNECTED:
            return DeviceState::DISCONNECTED;
        case BlueZDeviceState::CONNECTED:
            return DeviceState::CONNECTED;
    }

    LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: noConversionFound";
    return DeviceState::FOUND;
}

bool BlueZBluetoothDevice::queryDeviceProperty(const std::string& name, bool* value) {
    if (!value) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: nullValue";
        return false;
    } else if (!m_propertiesProxy) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: nullPropertiesProxy";
        return false;
    }

    return m_propertiesProxy->getBooleanProperty(BlueZConstants::BLUEZ_DEVICE_INTERFACE, name.c_str(), value);
}

void BlueZBluetoothDevice::transitionToState(BlueZDeviceState newState, bool sendEvent) {
    m_deviceState = newState;
    if (!m_deviceManager) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: nullDeviceManager";
        return;
    } else if (!m_deviceManager->getEventBus()) {
        LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "error: nullEventBus";
        return;
    }

    if (sendEvent) {
        m_deviceManager->getEventBus()->sendEvent(
            DeviceStateChangedEvent(shared_from_this(), convertToDeviceState(newState)));
    }
}

// TODO ACSDK-1398: Refactor this with a proper state machine.
void BlueZBluetoothDevice::onPropertyChanged(const GVariantMapReader& changesMap) {
    gboolean paired = false;
    bool pairedChanged = changesMap.getBoolean(BLUEZ_DEVICE_PROPERTY_PAIRED.c_str(), &paired);

    gboolean connected = false;
    bool connectedChanged = changesMap.getBoolean(BLUEZ_DEVICE_PROPERTY_CONNECTED.c_str(), &connected);

    // Changes to the friendlyName on the device will be saved on a new connect
    char* alias = nullptr;
    bool aliasChanged = changesMap.getCString(BLUEZ_DEVICE_PROPERTY_ALIAS.c_str(), &alias);
    std::string aliasStr;

    if(aliasChanged) {
        // This should never happen. If it does, don't update.
        if (!alias) {
            aliasChanged = false;
        } else {
            aliasStr = alias;
        }       
    }

    // This is used for checking connectedness.
    bool a2dpSourceAvailable = false;
    bool a2dpSinkAvailable =  false;

    /*
     * It's not guaranteed all services will be available at construction time.
     * If any become available at a later time, initialize them.
     */
    ManagedGVariant uuidsVariant = changesMap.getVariant(BLUEZ_DEVICE_PROPERTY_UUIDS.c_str());
    std::unordered_set<std::string> uuids;

    if (uuidsVariant.hasValue()) {
        auto uuids = getServiceUuids(uuidsVariant.get());
        initializeServices(uuids);

        a2dpSourceAvailable = (uuids.count(A2DPSourceInterface::UUID) > 0);
        a2dpSinkAvailable = (uuids.count(A2DPSinkInterface::UUID) > 0);
    }

    m_executor.submit([this,
                       pairedChanged,
                       paired,
                       connectedChanged,
                       connected,
                       a2dpSourceAvailable,
                       a2dpSinkAvailable,
                       aliasChanged,
                       aliasStr] {

        if (aliasChanged) {
            m_friendlyName = aliasStr;
        }

        switch (m_deviceState) {
            case BlueZDeviceState::FOUND: {
                if (pairedChanged && paired) {
                    transitionToState(BlueZDeviceState::PAIRED, true);
                    transitionToState(BlueZDeviceState::IDLE, true);

                    /*
                     * A connect signal doesn't always mean a device is connected by the BluetoothDeviceInterface
                     * definition. This sequence has been observed:
                     *
                     * 1) Pairing (BlueZ sends Connect = true).
                     * 2) Pair Successful.
                     * 3) Connect multimedia services.
                     * 4) Connect multimedia services successful (BlueZ sends Paired = true, UUIDs = [array of
                     * uuids]).
                     *
                     * Thus we will use the combination of Connect, Paired, and the availability of certain UUIDs to
                     * determine connectedness.
                     */
                    bool isConnected = false;
                    if (queryDeviceProperty(BLUEZ_DEVICE_PROPERTY_CONNECTED, &isConnected) && isConnected &&
                        (a2dpSourceAvailable || a2dpSinkAvailable)) {
                        transitionToState(BlueZDeviceState::CONNECTED, true);
                    }
                }
                break;
            }
            case BlueZDeviceState::IDLE: {
                if (connectedChanged && connected) {
                    transitionToState(BlueZDeviceState::CONNECTED, true);
                } else if (pairedChanged && !paired) {
                    transitionToState(BlueZDeviceState::UNPAIRED, true);
                    transitionToState(BlueZDeviceState::FOUND, true);
                }
                break;
            }
            case BlueZDeviceState::CONNECTED: {
                if (pairedChanged && !paired) {
                    transitionToState(BlueZDeviceState::UNPAIRED, true);
                    transitionToState(BlueZDeviceState::FOUND, true);
                } else if (connectedChanged && !connected) {
                    transitionToState(BlueZDeviceState::DISCONNECTED, true);
                    transitionToState(BlueZDeviceState::IDLE, true);
                }
                break;
            }
            case BlueZDeviceState::UNPAIRED:
            case BlueZDeviceState::PAIRED:
            case BlueZDeviceState::DISCONNECTED: {
                LOG_ERROR << TAG_BLUEZBLUETOOTHDEVICE << "onPropertyChanged; reason: invalidState";
                break;
            }
            case BlueZDeviceState::CONNECTION_FAILED: {
                if (pairedChanged && !paired) {
                    transitionToState(BlueZDeviceState::UNPAIRED, true);
                    transitionToState(BlueZDeviceState::FOUND, true);
                }
            }
        }
    });
}
} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK