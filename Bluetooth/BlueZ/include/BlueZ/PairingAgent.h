#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_PAIRINGAGENT_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_PAIRINGAGENT_H_

#include <memory>
#include <gio/gio.h>

#include "BlueZ/DBusProxy.h"
#include "BlueZ/DBusObject.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

/**
 * A DBusObject to handle pairing requests from BlueZ.
 */
class PairingAgent : public DBusObject<PairingAgent> {
public:
    // Destructor.
    virtual ~PairingAgent();

    // Create an instance of the PairingAgent.
    static std::unique_ptr<PairingAgent> create(std::shared_ptr<DBusConnection> connection);

private:
    // Constructor.
    PairingAgent(std::shared_ptr<DBusConnection> connection);

    // Initialization.
    bool init();

    // Register this PairingAgent.
    bool registerAgent();

    // Unregister this PairingAgent.
    bool unregisterAgent();

    // Register this PairingAgent as default.
    bool requestDefaultAgent();

    // Part of the org.bluez.PairingAgent1 inteface.
    void release(GVariant* arguments, GDBusMethodInvocation* invocation);
    void requestPinCode(GVariant* arguments, GDBusMethodInvocation* invocation);
    void displayPinCode(GVariant* arguments, GDBusMethodInvocation* invocation);
    void requestPasskey(GVariant* arguments, GDBusMethodInvocation* invocation);
    void displayPasskey(GVariant* arguments, GDBusMethodInvocation* invocation);
    void requestConfirmation(GVariant* arguments, GDBusMethodInvocation* invocation);
    void requestAuthorization(GVariant* arguments, GDBusMethodInvocation* invocation);
    void authorizeService(GVariant* arguments, GDBusMethodInvocation* invocation);
    void cancel(GVariant* arguments, GDBusMethodInvocation* invocation);

    // A proxy for the AgentManager.
    std::shared_ptr<DBusProxy> m_agentManager;
};

} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_PAIRINGAGENT_H_