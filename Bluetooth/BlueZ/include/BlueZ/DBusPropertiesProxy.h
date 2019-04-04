#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_DBUSPROPERTIESPROXY_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_DBUSPROPERTIESPROXY_H_

#include <memory>
#include <string>

#include "DBusProxy.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

/**
 * A special case of DBus proxy that binds to properties of a specific object.
 * 
 * See <a href="https://developer.gnome.org/glib/stable/gvariant-format-strings.html">GVariant Format Strings</a>
 * for type strings explanation.
 */
class DBusPropertiesProxy: public DBusProxy {
public:
    /**
     * A factory method to create a new instance.
     * 
     * @param objectPath
     * @return New instance of @c DBusPropertiesProxy on success, nullptr or otherwise.
     */
    static std::shared_ptr<DBusPropertiesProxy> create(const std::string& objectPath);

    // Get value of the interface's boolean property.
    bool getBooleanProperty(const std::string& interface, const std::string& property, bool* result);

    // Get value of the interface's string property.
    bool getStringProperty(const std::string& interface, const std::string& property, std::string* result);

    // Get value of the interface's variant property.
    bool getVariantProperty(const std::string& interface, const std::string& property, ManagedGVariant* result);

    // Set value of the interface's property.
    bool setProperty(const std::string& interface, const std::string& property, GVariant* value);

private:
    /**
     * Private constructor.
     */
    explicit DBusPropertiesProxy(GDBusProxy* proxy, const std::string& objectPath);
};

} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_DBUSPROPERTIESPROXY_H_
