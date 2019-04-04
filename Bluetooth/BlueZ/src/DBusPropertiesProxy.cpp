#include <Common/Utils/Logger/Log.h>

#include "BlueZ/BlueZConstants.h"
#include "BlueZ/BlueZDeviceManager.h"
#include "BlueZ/DBusPropertiesProxy.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

using namespace common::utils::logger;

#define TAG_DBUSPROPERTIESPROXY                 "DBusPropertiesProxy\t"

DBusPropertiesProxy::DBusPropertiesProxy(GDBusProxy* proxy, const std::string& objectPath) :
    DBusProxy(proxy, objectPath) {
}

std::shared_ptr<DBusPropertiesProxy> DBusPropertiesProxy::create(const std::string& objectPath) {
    GError* error = nullptr;
    GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM,
        G_DBUS_PROXY_FLAGS_NONE,
        nullptr,
        BlueZConstants::BLUEZ_SERVICE_NAME,
        objectPath.c_str(),
        BlueZConstants::PROPERTIES_INTERFACE,
        nullptr,
        &error);
    
    if(error) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "createFailed, error: " << error->message;
        g_error_free(error);
        return nullptr;
    }

    return std::shared_ptr<DBusPropertiesProxy>(new DBusPropertiesProxy(proxy, objectPath));
}

bool DBusPropertiesProxy::getBooleanProperty(const std::string& interface, const std::string& property, bool* result) {
    if(result == nullptr) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "getBooleanPropertyFailed, reason: result is null";
        return false;
    }
    ManagedGError error;
    ManagedGVariant varResult = callMethod("Get", g_variant_new("(ss)", interface.c_str(), property.c_str()), error.toOutputParameter());

    if(error.hasError()) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "failed to get boolean property, " 
                    << "error: " << error.getMessage()
                    << "interface: " << interface
                    << "property: " << property
                    << "path: " << getObjectPath();
        return false;
    }

    GVariantTupleReader tupleReader(varResult);
    ManagedGVariant unboxed = tupleReader.getVariant(0).unbox();
    *result = static_cast<bool>(g_variant_get_boolean(unboxed.get()));
    return true;
}

bool DBusPropertiesProxy::getVariantProperty(
    const std::string& interface,
    const std::string& property,
    ManagedGVariant* result) {
    if(result == nullptr) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "getVariantPropertyFailed, reason: result is null";
        return false;
    }

    ManagedGError error;
    ManagedGVariant varResult = callMethod("Get", g_variant_new("(ss)", interface.c_str(), property.c_str()), error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "getVariantPropertyFailed, " 
                    << "error: " << error.getMessage()
                    << "Failed to get variant property: " << property;
        return false;        
    }
    result->swap(varResult);
    return true;
}

bool DBusPropertiesProxy::getStringProperty(
    const std::string& interface,
    const std::string& property,
    std::string* result) {
    if(result == nullptr) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "getStringPropertyFailed, reason: result is null";
        return false;
    }  

    ManagedGError error;
    ManagedGVariant varResult = callMethod("Get", g_variant_new("(ss)", interface.c_str(), property.c_str()), error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "getStringPropertyFailed, " 
                    << "error: " << error.getMessage()
                    << "Failed to get string property: " << property;
        return false;        
    }
    GVariantTupleReader tupleReader(varResult);
    ManagedGVariant unboxed = tupleReader.getVariant(0).unbox();
    *result = g_variant_get_string(unboxed.get(), nullptr);
    return true;    
}

bool DBusPropertiesProxy::setProperty(const std::string& interface, const std::string& property, GVariant* value) {
    if(value == nullptr) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "setPropertyFailed, reason: result is null";
        return false;
    }
    ManagedGError error;
    ManagedGVariant varResult = callMethod("Set", g_variant_new("(ssv)", interface.c_str(), property.c_str(), value), error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_DBUSPROPERTIESPROXY << "setPropertyFailed, " 
                    << "error: " << error.getMessage()
                    << "Failed to set property value: " << property;
        return false;        
    }
    return true;
}

} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK