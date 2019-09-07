#include "BlueZ/BlueZHostController.h"
#include <Common/Utils/Logger/Log.h>

#include "BlueZ/BlueZConstants.h"
#include "BlueZ/BlueZDeviceManager.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

using namespace common::utils::logger;

#define TAG_BLUEZHOSTCONTROLLER             "BlueZHostController\t"

// String to identify name property.
static const std::string ALIAS_PROPERTY = "Alias";

// String to identify discovering property.
static const std::string DISCOVERABLE_PROPERTY = "Discoverable";

// The size of a MAC address in format XX:XX:XX:XX:XX:XX
static const unsigned int MAC_SIZE = 17;

// String to identify scanning property.
static const std::string SCANNING_PROPERTY = "Discovering";

// String to identify adapter method to start scanning.
static const std::string START_SCAN = "StartDiscovery";

// String to identify adapter method to stop scanning.
static const std::string STOP_SCAN = "StopDiscovery";

using namespace common::utils;

// A callback device name.
static const std::string DEFAULT_NAME = "Device";

static bool truncate(const std::unique_ptr<MacAddressString>& mac, std::string* truncatedMac) {
    if(!mac) {
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: nullMac";
        return false;
    } else if(!truncatedMac) {
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: nullTruncatedMAC";
        return false;
    } else if(mac->getString().length() != MAC_SIZE){
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: invalidMACLenght";
        return false;
    }

    *truncatedMac = mac->getString();

    char X = 'X';
    truncatedMac->at(0) = X;
    truncatedMac->at(1) = X;
    truncatedMac->at(3) = X;
    truncatedMac->at(4) = X;
    truncatedMac->at(6) = X;
    truncatedMac->at(7) = X;
    truncatedMac->at(9) = X;
    truncatedMac->at(10) = X;

    return true;   
}

std::unique_ptr<BlueZHostController> BlueZHostController::create(const std::string& adapterObjectPath) {
    if(adapterObjectPath.empty()) {
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: emptyAdapterPath";
        return nullptr;
    }
    
    auto hostController = std::unique_ptr<BlueZHostController>(new BlueZHostController(adapterObjectPath));
    if(!hostController->init()) {
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: initFailed";
        return nullptr;
    }
    
    return hostController;
}

BlueZHostController::BlueZHostController(const std::string& adapterObjectPath) :
    m_adapterObjectPath{adapterObjectPath} {

}

bool BlueZHostController::init() {
    m_adapter = DBusProxy::create(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, m_adapterObjectPath);
    if(!m_adapter) {
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: createAdapterProxyFailed";
        return false;
    }

    m_adapterProperties = DBusPropertiesProxy::create(m_adapterObjectPath);
    if(!m_adapterProperties) {
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: createAdapterPropertiesProxyFailed";
        return false;       
    }

    // Get the MAC address.
    std::string mac;
    if(!m_adapterProperties->getStringProperty(
        BlueZConstants::BLUEZ_ADAPTER_INTERFACE, BlueZConstants::BLUEZ_DEVICE_INTERFACE_ADDRESS, &mac)) {
        LOG_DEBUG << TAG_BLUEZHOSTCONTROLLER << "reason: noMACAddress";
        return false;
    }

    // Create a MacAddressString object to validate the MAC string.
    m_mac =  MacAddressString::create(mac);
    if(!m_mac) {
        LOG_DEBUG << TAG_BLUEZHOSTCONTROLLER << "reason: invalidMAC";
        return false;
    }

    // Atempt to get the friendlyName of the adapter.
    if(!m_adapterProperties->getStringProperty(
        BlueZConstants::BLUEZ_ADAPTER_INTERFACE, BlueZConstants::BLUEZ_DEVICE_INTERFACE_ALIAS, &m_friendlyName)) {
        //TODO: failed to obtain a friendly name. But it doesn't necessary.
        std::string truncatedMac;
        m_friendlyName = truncate(m_mac, &truncatedMac) ? truncatedMac : DEFAULT_NAME;
    }

    return true;
}

std::string BlueZHostController::getMac() const {
    return m_mac->getString();
}

std::string BlueZHostController::getFriendlyName() const {
    return m_friendlyName;
}

// Discovery
std::future<bool> BlueZHostController::enterDiscoverableMode() {
    return setDiscoverable(true);
}

std::future<bool> BlueZHostController::exitDiscoverableMode() {
    return setDiscoverable(false);
}

std::future<bool> BlueZHostController::setDiscoverable(bool discoverable) {

    // synchronous operation for us
    std::promise<bool> promise;
    bool success = false;

    {
        std::lock_guard<std::mutex> lock(m_adapterMutex);
        success = m_adapterProperties->setProperty(
            BlueZConstants::BLUEZ_ADAPTER_INTERFACE, DISCOVERABLE_PROPERTY, g_variant_new_boolean(discoverable));
    }

    promise.set_value(success);

    if(!success) {
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: setAdapterPropertyFailed, discoverable: " << discoverable;
    }

    return promise.get_future();
}

bool BlueZHostController::isDiscoverable() const {
    bool result = false;
    std::lock_guard<std::mutex> lock(m_adapterMutex);
    m_adapterProperties->getBooleanProperty(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, DISCOVERABLE_PROPERTY, &result);
    return result;
}

// Scanning
std::future<bool> BlueZHostController::startScan() {
    return changeScanState(true);
}

std::future<bool> BlueZHostController::stopScan() {
    return changeScanState(false);
}

std::future<bool> BlueZHostController::changeScanState(bool scanning) {
    std::promise<bool> promise;
    ManagedGError error;

    {
        std::lock_guard<std::mutex> lock(m_adapterMutex);
        ManagedGVariant result = 
            m_adapter->callMethod(scanning ? START_SCAN : STOP_SCAN, nullptr, error.toOutputParameter());
    }
    if(error.hasError()) {
        LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: callScanMethodFailed, error: " << error.getMessage();
        promise.set_value(false);
    } else {
        promise.set_value(true);
    }

    return promise.get_future();
}

bool BlueZHostController::isScanning() const {
    bool result = false;
    std::lock_guard<std::mutex> lock(m_adapterMutex);
    m_adapterProperties->getBooleanProperty(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, SCANNING_PROPERTY, &result);
    return result;
}

void BlueZHostController::onPropertyChanged(const GVariantMapReader& changesMap) {
    char* alias =  nullptr;
    bool aliasChanged = changesMap.getCString(ALIAS_PROPERTY.c_str(), &alias);

    std::lock_guard<std::mutex> lock(m_adapterMutex);
    if(aliasChanged) {
        // this should never happen.
        if(!alias) {
            LOG_ERROR << TAG_BLUEZHOSTCONTROLLER << "reason: nullAlias";
        } else {
            LOG_DEBUG << TAG_BLUEZHOSTCONTROLLER << "nameChanged. oldName: " << m_friendlyName
                                                 << "\tnewName: " << alias;
            m_friendlyName = alias;
        }
    }
}

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK