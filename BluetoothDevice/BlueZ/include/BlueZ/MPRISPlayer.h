#ifndef DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_MPRISPLAYER_H_
#define DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_MPRISPLAYER_H_

#include <memory>
#include <gio/gio.h>

#include <Common/Utils/Bluetooth/BluetoothEventBus.h>
#include "BlueZ/DBusObject.h"
#include "BlueZ/DBusProxy.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

class MPRISPlayer : public DBusObject<MPRISPlayer> {
public:
    // Destructor.
    virtual ~MPRISPlayer();

    // The default MPRIS object path for players.
    static const std::string MPRIS_OBJECT_PATH;

    // Create an instance of the MPRISPlayer.
    static std::unique_ptr<MPRISPlayer> create(
        std::shared_ptr<DBusConnection> connection,
        std::shared_ptr<DBusProxy> media,
        std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus,
        const std::string& playerPath = MPRIS_OBJECT_PATH);

private:
    // Constructor.
    MPRISPlayer(
        std::shared_ptr<DBusConnection> connection,
        std::shared_ptr<DBusProxy> media,
        std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus,
        const std::string& playerPath);
    /**
     * Performs initialization.
     *
     * @return Whether the operation was succesful or not.
     */
    bool init();

    /**
     * Register this @c MPRISPlayer with the BlueZ Media object.
     *
     * @return Whether the operation was successful.
     */
    bool registerPlayer();

    /**
     * Unregister this @c MPRISPlayer with the BlueZ Media object.
     *
     * @return Whether the operation was successful.
     */
    bool unregisterPlayer();

    /**
     * The default unsupported method.
     *
     * @param arguments The arguments which this DBus method was called with.
     * @invocation A struct containing data about the method invocation.
     */
    void unsupportedMethod(GVariant* arguments, GDBusMethodInvocation* invocation);

    /**
     * Converts the callback to the corresponding @c AVRCPCommandReceived event.
     *
     * @param arguments The arguments which this DBus method was called with.
     * @invocation A struct containing data about the method invocation.
     */
    void toAVRCPCommand(GVariant* arguments, GDBusMethodInvocation* invocation);

    /**
     * Sends the @c AVRCPCommandRecieved event to @c m_eventBus.
     *
     * @param command The command to send.
     */
    void sendEvent(const common::sdkInterfaces::bluetooth::services::AVRCPCommand& command);

    /// The DBus object path of the player.
    const std::string m_playerPath;

    /// A Proxy for the Media object.
    std::shared_ptr<DBusProxy> m_media;

    /// The event bus on which to send the @c AVRCPCommand.
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> m_eventBus;    
};

} // namespace blueZ
} // namespace bluetoothDevice
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_MPRISPLAYER_H_