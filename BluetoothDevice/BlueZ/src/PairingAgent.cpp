#include <iostream>
#include <string>

#include <Common/Utils/Logger/Log.h>

#include "BlueZ/BlueZConstants.h"
#include "BlueZ/BlueZDeviceManager.h"
#include "BlueZ/PairingAgent.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

using namespace common::utils::logger;

#define TAG_PAIRINGAGENT "PairingAgent\t"

// The path we register the PairingAgent object under.
static const std::string AGENT_OBJECT_PATH = "/tmp/Agent";

// See https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/agent-api.txt 
// for more details.
static const std::string CAPABILITY = "NoInputNoOutput";
static const std::string RELEASE = "Release";
static const std::string REQUEST_PIN_CODE = "RequestPinCode";
static const std::string DISPLAY_PIN_CODE = "DisplayPinCode";
static const std::string REQUEST_PASSKEY = "RequestPasskey";
static const std::string DISPLAY_PASSKEY = "DisplayPasskey";
static const std::string REQUEST_CONFIRMATION = "RequestConfirmation";
static const std::string REQUEST_AUTHORIZATION = "RequestAuthorization";
static const std::string AUTHORIZE_SERVICE = "AuthorizeService";
static const std::string CANCEL = "Cancel";

// Bluez Identifier.
static const std::string BLUEZ_OBJECT_PATH = "/org/bluez";

// Default keypass
const uint32_t DEFAULT_KEYPASS = 0;

// Default pincode
const char* DEFAULT_PINCODE = "0000";

// The introspect XML we ue to create the DBus object.
const char INTROSPECT_XML[] = R"(
<!DOCTYPE node PUBLIC -//freedesktop//DTD D-BUS Object Introspection 1.0//EN
    "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd";>
<node>
<interface name="org.bluez.Agent1">
    <method name="Release">
    </method>
    <method name="RequestPinCode">
        <arg type="s" direction="out"/>
        <arg name="device" type="o" direction="in"/>
        </method>
    <method name="DisplayPinCode">
        <arg type="s" direction="out"/>
        <arg name="device" type="o" direction="in"/>
        <arg name="pincode" type="s" direction="in"/>
    </method>
    <method name="RequestPasskey">
        <arg type="u" direction="out"/>
        <arg name="device" type="o" direction="in"/>
    </method>
    <method name="DisplayPasskey">
        <arg name="device" type="o" direction="in"/>
        <arg name="passkey" type="u" direction="in"/>
        <arg name="entered" type="q" direction="in"/>
    </method>
    <method name="RequestConfirmation">
        <arg name="device" type="o" direction="in"/>
        <arg name="passkey" type="u" direction="in"/>
    </method>
    <method name="RequestAuthorization">
        <arg name="device" type="o" direction="in"/>
    </method>
    <method name="AuthorizeService">
        <arg name="device" type="o" direction="in"/>
        <arg name="uuid" type="s" direction="in"/>
    </method>
    <method name="Cancel">
    </method>
</interface>
</node>)";

std::unique_ptr<PairingAgent> PairingAgent::create(std::shared_ptr<DBusConnection> connection) {
    if(!connection) {
        LOG_ERROR << TAG_PAIRINGAGENT << "paringAgentFailed, reason: nullConnection";
        return nullptr;
    }

    auto pairingAgent = std::unique_ptr<PairingAgent>(new PairingAgent(connection));
    if(!pairingAgent->init()) {
        LOG_ERROR << TAG_PAIRINGAGENT << "reason: initFailed";
        return nullptr;
    }

    return pairingAgent;
}

PairingAgent::PairingAgent(std::shared_ptr<DBusConnection> connection) : 
        DBusObject(
            connection,
            INTROSPECT_XML,
            AGENT_OBJECT_PATH,
            {
                {RELEASE, &PairingAgent::release},
                {REQUEST_PIN_CODE, &PairingAgent::requestPinCode},
                {DISPLAY_PIN_CODE, &PairingAgent::displayPinCode},
                {REQUEST_PASSKEY, &PairingAgent::requestPasskey},
                {DISPLAY_PASSKEY, &PairingAgent::displayPasskey},
                {REQUEST_CONFIRMATION, &PairingAgent::requestConfirmation},
                {REQUEST_AUTHORIZATION, &PairingAgent::requestAuthorization},
                {AUTHORIZE_SERVICE, &PairingAgent::authorizeService},
                {CANCEL, &PairingAgent::cancel},
            }) {
}

bool PairingAgent::init() {
    if(!registerWithDBus()) {
        return false;
    }

    m_agentManager = DBusProxy::create(BlueZConstants::BLUEZ_AGENTMANAGER_INTERFACE, BLUEZ_OBJECT_PATH);
    if(!m_agentManager) {
        LOG_ERROR << TAG_PAIRINGAGENT << "reason: nullAgentManager";
        return false;
    }

    return registerAgent() && requestDefaultAgent();
}

PairingAgent::~PairingAgent() {
    unregisterAgent();
}

void PairingAgent::release(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void PairingAgent::requestPinCode(GVariant* arguments, GDBusMethodInvocation* invocation) {
    LOG_INFO << TAG_PAIRINGAGENT << "Pincode: " << DEFAULT_PINCODE;

    auto parameters = g_variant_new("(s)", DEFAULT_PINCODE);
    g_dbus_method_invocation_return_value(invocation, parameters);
}

void PairingAgent::displayPinCode(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void PairingAgent::requestPasskey(GVariant* arguments, GDBusMethodInvocation* invocation) {
    LOG_INFO << TAG_PAIRINGAGENT << "Passkey: " << DEFAULT_KEYPASS;

    auto parameters = g_variant_new("(u)", DEFAULT_KEYPASS);
    g_dbus_method_invocation_return_value(invocation, parameters);
}

void PairingAgent::displayPasskey(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void PairingAgent::requestConfirmation(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void PairingAgent::requestAuthorization(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void PairingAgent::authorizeService(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void PairingAgent::cancel(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

bool PairingAgent::requestDefaultAgent() {
    ManagedGError error;

    auto parameters = g_variant_new("(o)", AGENT_OBJECT_PATH.c_str());
    m_agentManager->callMethod("RequestDefaultAgent", parameters, error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_PAIRINGAGENT << "requestDefaultAgentFailed, reason: " << error.getMessage();
        return false;
    }
    return true;
}

bool PairingAgent::registerAgent() {
    ManagedGError error;

    auto parameters = g_variant_new("(os)", AGENT_OBJECT_PATH.c_str(), CAPABILITY.c_str());
    m_agentManager->callMethod("RegisterAgent", parameters, error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_PAIRINGAGENT << "registerAgentFailed, reason: " << error.getMessage();
        return false;        
    }
    return true;
}

bool PairingAgent::unregisterAgent() {
    ManagedGError error;

    auto parameters = g_variant_new("(o)", AGENT_OBJECT_PATH.c_str());
    m_agentManager->callMethod("UnregisterAgent", parameters, error.toOutputParameter());
    if(error.hasError()) {
        LOG_ERROR << TAG_PAIRINGAGENT << "unregisterAgentFailed, reason: " << error.getMessage();
        return false;
    }
    return true;   
}

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK