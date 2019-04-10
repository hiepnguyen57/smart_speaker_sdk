#ifndef DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_DBUSOBJECT_H_
#define DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_DBUSOBJECT_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "BlueZ/DBusObjectBase.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

/**
 * Typed version of the base class for objects exposed to DBus. This class simplifies the handling of DBus method calls
 * by binding them to class member functions
 *
 * @tparam T Name of the derived class to be used to imlpement the object. The derived class declaration would look
 * like this:
 * @code
 * class MyDBusObjectClass : public DBusObject<MyDBusObjectClass> {
 * };
 * @endcode
 */
template <class T>
class DBusObject : public DBusObjectBase {
public:
    virtual ~DBusObject() = default;

    //Callback type to be used by member functions handling DBus method calls.
    using commandHandler_t = void (T::*)(GVariant* parameters, GDBusMethodInvocation* invocation);

protected:
    /**
     * Constructor.
     */
    DBusObject(
        std::shared_ptr<DBusConnection> connection,
        std::string xmlInterfaceIntrospection,
        std::string objectPath,
        std::unordered_map<std::string, commandHandler_t> methodMap) : 
        DBusObjectBase(connection, xmlInterfaceIntrospection, objectPath, &onMethodCallStatic),
        m_commands{methodMap} {
    }

private:
    // static method used as a callback to subcribe to DBus method calls for the object.
    static void onMethodCallStatic(
        GDBusConnection* conn,
        const gchar* sender_name,
        const gchar* object_path,
        const gchar* inteface_name,
        const gchar* method_name,
        GVariant* parameters,
        GDBusMethodInvocation* invocation,
        gpointer data) {
            auto thisPtr = static_cast<T*>(data);
            thisPtr->onMethodCall(method_name, parameters, invocation);
    }

    // Internal method routing the DBus method call to proper implementation member function of the derived class.
    void onMethodCall(const gchar* method_name, GVariant* parameters, GDBusMethodInvocation* invocation);

    // Map between method names and member functions implement them
    std::unordered_map<std::string, commandHandler_t> m_commands;
};

template<class T>
void DBusObject<T>::onMethodCall(const gchar* method_name, GVariant* parameters, GDBusMethodInvocation* invocation) {
    onMethodCalledInternal(method_name);

    auto iter = m_commands.find(method_name);
    if(iter != m_commands.end()) {
        commandHandler_t handler = iter->second;
        (static_cast<T*>(this)->*handler)(parameters, invocation);
    } else {
        g_dbus_method_invocation_return_error(
            invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD, "Unknown method: "
        );
    }
}

} // namespace blueZ
} // namespace bluetoothDevice
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_DBUSOBJECT_H_
