#include <Common/Utils/Logger/Log.h>

#include "BlueZ/DBusConnection.h"
#include "BlueZ/BlueZUtils.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

using namespace common::utils::logger;

static const std::string TAG_DBUSCONNECTION = "DBusConnection\t";

GDBusConnection* DBusConnection::getGDBusConnection() {
    return m_connection;
}

std::unique_ptr<DBusConnection> DBusConnection::create(GBusType connectionType) {
    ManagedGError error;

    GDBusConnection *connection = g_bus_get_sync(connectionType, nullptr, error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_DBUSCONNECTION << "createNewFailed";
        return nullptr;
    }

    g_dbus_connection_set_exit_on_close(connection, false);

    return std::unique_ptr<DBusConnection>(new DBusConnection(connection));
}

unsigned int DBusConnection::subcribeToSignal(
    const char* serviceName,
    const char* interfaceName,
    const char* member,
    const char* firstArgumentFilter,
    GDBusSignalCallback callback,
    gpointer userData) {
    if(serviceName == nullptr) {
        LOG_ERROR << TAG_DBUSCONNECTION << "subcribeToSignalFailed, reason: serviceName is null";
        return 0;
    }
    
    if(interfaceName == nullptr) {
        LOG_ERROR << TAG_DBUSCONNECTION << "subcribeToSignalFailed, reason: interfaceName is null";
        return 0;
    }

    if(member == nullptr) {
        LOG_ERROR << TAG_DBUSCONNECTION << "subcribeToSignalFailed, reason: member is null";
        return 0;
    }

    if(callback == nullptr) {
        LOG_ERROR << TAG_DBUSCONNECTION << "subcribeToSignalFailed, reason: callback is null";
        return 0;
    }

    guint subId = g_dbus_connection_signal_subscribe(
        m_connection,
        serviceName,
        interfaceName,
        member,
        nullptr,
        firstArgumentFilter,
        G_DBUS_SIGNAL_FLAGS_NONE,
        callback,
        userData,
        nullptr);
    if(subId == 0) {
        LOG_ERROR << TAG_DBUSCONNECTION << "subsribeToSignalFailed, reason: failed to subscribe";
        return 0;
    }

    std::lock_guard<std::mutex> guard(m_subscriptionsMutex);
    m_subscriptions.push_back(subId);
    
    return subId;
}

DBusConnection::DBusConnection(GDBusConnection* connection) : m_connection{connection} {
}

void DBusConnection::close() {
    if(!m_connection) {
        //already close
        return;
    }

    {
        std::lock_guard<std::mutex> guard(m_subscriptionsMutex);

        for(auto subscriptionId : m_subscriptions) {
            g_dbus_connection_signal_unsubscribe(m_connection, subscriptionId);
        }
        m_subscriptions.clear();
    }

    g_dbus_connection_flush_sync(m_connection, nullptr, nullptr);
    g_dbus_connection_close_sync(m_connection, nullptr, nullptr);
    g_object_unref(m_connection);
    m_connection = nullptr;
}

DBusConnection::~DBusConnection() {
    close();
}

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK