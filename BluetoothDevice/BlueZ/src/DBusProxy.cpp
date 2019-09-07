#include <gio/gunixfdlist.h>
#include <Common/Utils/Logger/Log.h>

#include "BlueZ/BlueZConstants.h"
#include "BlueZ/DBusProxy.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

using namespace common::utils::logger;

static const std::string TAG_DBUSPROXY = "DBusProxy\t";

static const int PROXY_DEFAULT_TIMEOUT = -1;

DBusProxy::DBusProxy(GDBusProxy *proxy, const std::string& objectPath) : m_proxy{proxy}, m_objectPath{objectPath} {
}

DBusProxy::~DBusProxy() {
    if(m_proxy) {
        g_object_unref(m_proxy);
    }
}

std::shared_ptr<DBusProxy> DBusProxy::create(const std::string& interfaceName, 
        const std::string& objectPath) {
    GError *error = nullptr;

    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM,
        G_DBUS_PROXY_FLAGS_NONE,
        nullptr,
        BlueZConstants::BLUEZ_SERVICE_NAME,
        objectPath.c_str(),
        interfaceName.c_str(),
        nullptr,
        &error);

    if(!proxy) {
        LOG_ERROR << TAG_DBUSPROXY << "createFailed, error" << error->message;
        g_error_free(error);
        return nullptr;
    }

    return std::shared_ptr<DBusProxy>(new DBusProxy(proxy, objectPath));
}

ManagedGVariant DBusProxy::callMethod(const std::string& methodName, 
        GVariant* parameters, GError** error) {
    GVariant *tempResult = g_dbus_proxy_call_sync(
        m_proxy, methodName.c_str(), parameters, G_DBUS_CALL_FLAGS_NONE,
        PROXY_DEFAULT_TIMEOUT, nullptr, error);

    return ManagedGVariant(tempResult);
}

ManagedGVariant DBusProxy::callMethodWithFDList(
    const std::string& methodName,
    GVariant* parameters,
    GUnixFDList** outlist,
    GError** error) {
    GVariant* tempResult = g_dbus_proxy_call_with_unix_fd_list_sync(
        m_proxy,
        methodName.c_str(),
        parameters,
        G_DBUS_CALL_FLAGS_NONE,
        PROXY_DEFAULT_TIMEOUT,
        nullptr,
        outlist,
        nullptr,
        error);
    return ManagedGVariant(tempResult);
}

std::string DBusProxy::getObjectPath() const {
    return m_objectPath;
}

GDBusProxy* DBusProxy::get() {
    return m_proxy;
}

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK