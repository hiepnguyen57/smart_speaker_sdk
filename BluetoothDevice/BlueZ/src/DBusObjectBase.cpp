#include <Common/Utils/Logger/Log.h>

#include "BlueZ/DBusObjectBase.h"
#include "BlueZ/BlueZUtils.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

using namespace common::utils::logger;

static const std::string TAG_DBUSOBJECTBASE = "DBusObjectBase\t";

DBusObjectBase::~DBusObjectBase() {
    unregisterObject();
}

DBusObjectBase::DBusObjectBase(
    std::shared_ptr<DBusConnection> connection,
    const std::string& xmlInterfaceIntrospection,
    const std::string& objectPath,
    GDBusInterfaceMethodCallFunc methodCallFunc) :
        m_xmlInterfaceIntrospection{xmlInterfaceIntrospection},
        m_registrationId{0},
        m_interfaceVtable{methodCallFunc, nullptr, nullptr},
        m_connection{connection},
        m_objectPath{objectPath} {
}

void DBusObjectBase::onMethodCalledInternal(const char* methodName) {

}

void DBusObjectBase::unregisterObject() {
    if(m_registrationId > 0) {
        g_dbus_connection_unregister_object(m_connection->getGDBusConnection(), m_registrationId);
        m_registrationId = 0;
    }
}

bool DBusObjectBase::registerWithDBus() {
    if(m_registrationId > 0) {
        return true;
    }

    ManagedGError error;
    GDBusNodeInfo* data = g_dbus_node_info_new_for_xml(m_xmlInterfaceIntrospection.c_str(), error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_DBUSOBJECTBASE << "Failed to register object, error: " <<  error.getMessage();
        return false;
    }

    GDBusInterfaceInfo* interfaceInfo =  data->interfaces[0];

    m_registrationId = g_dbus_connection_register_object(
        m_connection->getGDBusConnection(),
        m_objectPath.c_str(),
        interfaceInfo,
        &m_interfaceVtable,
        this,
        nullptr,
        nullptr);
    
    return m_registrationId > 0;
}

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK