#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_DBUSOBJECTBASE_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_DBUSOBJECTBASE_H_

#include <memory>
#include <string>

#include "BlueZ/DBusConnection.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

/**
 * Base class for objects exposed to Dbus
 */
class DBusObjectBase {
public:
    /**
     * Destructor
     */
    virtual ~DBusObjectBase();

    /**
     * Register an object with DBus
     * 
     * @return true if succeeded, false otherwise
     */
    bool registerWithDBus();

    /**
     * Unregister an object from DBus
     */
    void unregisterObject();

protected:
    /**
     * A constuctor.
     * 
     * @param connection @c DBusConnection connection to register this object with
     * @param xmlInterfaceIntrospection XML containing the description of the interfaces this object implements. See
     * <a href="https://dbus.freedesktop.org/doc/dbus-api-design.html"/>
     * @param objectPath Path to register the object to
     * @param methodCallFunc @c GDBusInterfaceMethodCallFunc callback that handles the incoming method calls
     */
    DBusObjectBase(
        std::shared_ptr<DBusConnection> connection,
        const std::string& xmlInterfaceIntrospection,
        const std::string& objectPath,
        GDBusInterfaceMethodCallFunc methodCallFunc);
    
    /**
     * Method used solely for logging the call of the object's method.
     * 
     * @param methodName the called method name
     */
    void onMethodCalledInternal(const char* methodName);

private:
    // XML interface description to be used for object registration.
    const std::string m_xmlInterfaceIntrospection;

    // The ID of the object registered to DBus. 0 if object is not register.
    unsigned int m_registrationId;

    // @c GDBusInterfaceVTable object containing the reference to the interface description parsed from the XML
    GDBusInterfaceVTable m_interfaceVtable;

    // A DBus connection to be used to register the object
    std::shared_ptr<DBusConnection> m_connection;

    // A path to be registered object with
    const std::string m_objectPath;
};


} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_DBUSOBJECTBASE_H_