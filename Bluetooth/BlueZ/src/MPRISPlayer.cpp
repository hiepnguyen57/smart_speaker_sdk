#include <string>

#include <Common/Utils/Logger/Log.h>
#include <Common/Utils/Bluetooth/BluetoothEvents.h>

#include "BlueZ/BlueZConstants.h"
#include "BlueZ/BlueZDeviceManager.h"
#include "BlueZ/MPRISPlayer.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

using namespace common::sdkInterfaces::bluetooth::services;
using namespace common::utils::bluetooth;
using namespace common::utils::logger;

#define TAG_MPRISPLAYER         "MPRISPlayer\t"

/// Property indicating whether this player supports the Next functionality.
static const std::string CAN_GO_NEXT = "CanGoNext";

/// Property indicating whether this player supports the Previous functionality.
static const std::string CAN_GO_PREVIOUS = "CanGoPrevious";

/// Property indicating whether this player supports the Play functionality.
static const std::string CAN_PLAY = "CanPlay";

/// Property indicating whether this player supports the Pause functionality.
static const std::string CAN_PAUSE = "CanPause";

/// Property indicating whether this player supports the Seek functionality.
static const std::string CAN_SEEK = "CanSeek";

/// Property indicating whether this player supports the Next functionality.
static const std::string CAN_CONTROL = "CanControl";

/// The Play method.
static const std::string PLAY = "Play";

/// The Pause method.
static const std::string PAUSE = "Pause";

/// The Next method.
static const std::string NEXT = "Next";

/// The Previous method.
static const std::string PREVIOUS = "Previous";

/// The PlayPause method.
static const std::string PLAY_PAUSE = "PlayPause";

/// The Stop method.
static const std::string STOP = "Stop";

/// The Seek method.
static const std::string SEEK = "Seek";

/// The SetPosition method.
static const std::string SET_POSITION = "SetPosition";

/// The OpenUri method.
static const std::string OPEN_URI = "OpenUri";

/// The RegisterPlayer method on the org.bluez.Media1 API.
static const std::string REGISTER_PLAYER = "RegisterPlayer";

/// The UnregisterPlayer method on the org.bluez.Media1 API.
static const std::string UNREGISTER_PLAYER = "UnregisterPlayer";

/// The introspect XML we use to create the DBus object.
// clang-format off
const char INTROSPECT_XML[] = R"(
<!DOCTYPE node PUBLIC -//freedesktop//DTD D-BUS Object Introspection 1.0//EN
    "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd";>
<node>
<interface name="org.mpris.MediaPlayer2.Player">
    <method name="Next"/>
    <method name="Previous"/>
    <method name="Pause"/>
    <method name="PlayPause"/>
    <method name="Stop"/>
    <method name="Play"/>
    <method name="Seek">
        <arg type="x" direction="in"/>
    </method>
    <method name="SetPosition">
        <arg type="o" direction="in"/>
        <arg type="x" direction="in"/>
    </method>
    <method name="OpenUri">
        <arg type="s" direction="in"/>
    </method>
</interface>
</node>)";

const std::string MPRISPlayer::MPRIS_OBJECT_PATH = "/org/mpris/MediaPlayer2";

std::unique_ptr<MPRISPlayer> MPRISPlayer::create(
    std::shared_ptr<DBusConnection> connection,
    std::shared_ptr<DBusProxy> media,
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus,
    const std::string& playerPath) {
    if (!connection) {

        LOG_ERROR << TAG_MPRISPLAYER << "reason: nullConnection";
        return nullptr;
    } else if (!media) {
        LOG_ERROR << TAG_MPRISPLAYER << "reason: nullMediaManager";
        return nullptr;
    } else if (!eventBus) {
        LOG_ERROR << TAG_MPRISPLAYER << "reason: nullEventBus";
        return nullptr;
    }

    auto mediaPlayer = std::unique_ptr<MPRISPlayer>(new MPRISPlayer(connection, media, eventBus, playerPath));
    if (!mediaPlayer->init()) {
        LOG_ERROR << TAG_MPRISPLAYER << "reason: initFailed";
        return nullptr;
    }

    return mediaPlayer;
}

MPRISPlayer::MPRISPlayer(
    std::shared_ptr<DBusConnection> connection,
    std::shared_ptr<DBusProxy> media,
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus,
    const std::string& playerPath) :
        DBusObject(
            connection,
            INTROSPECT_XML,
            playerPath,
            {
                {NEXT, &MPRISPlayer::toAVRCPCommand},
                {PREVIOUS, &MPRISPlayer::toAVRCPCommand},
                {PAUSE, &MPRISPlayer::toAVRCPCommand},
                {PLAY, &MPRISPlayer::toAVRCPCommand},
                {PLAY_PAUSE, &MPRISPlayer::unsupportedMethod},
                {STOP, &MPRISPlayer::unsupportedMethod},
                {SEEK, &MPRISPlayer::unsupportedMethod},
                {SET_POSITION, &MPRISPlayer::unsupportedMethod},
                {OPEN_URI, &MPRISPlayer::unsupportedMethod},
            }),
        m_playerPath{playerPath},
        m_media{media},
        m_eventBus{eventBus} {
}

bool MPRISPlayer::init() {
    if (!registerWithDBus()) {
        LOG_ERROR << TAG_MPRISPLAYER << "reason: createDBusObjectFailed";
        return false;
    }

    return registerPlayer();
}

MPRISPlayer::~MPRISPlayer() {
    unregisterPlayer();
}

void MPRISPlayer::unsupportedMethod(GVariant* arguments, GDBusMethodInvocation* invocation) {
    LOG_WARN << TAG_MPRISPLAYER << "methodName: " << g_dbus_method_invocation_get_method_name(invocation);
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void MPRISPlayer::toAVRCPCommand(GVariant* arguments, GDBusMethodInvocation* invocation) {
    const char* method = g_dbus_method_invocation_get_method_name(invocation);

    if (PLAY == method) {
        sendEvent(AVRCPCommand::PLAY);
    } else if (PAUSE == method) {
        sendEvent(AVRCPCommand::PAUSE);
    } else if (NEXT == method) {
        sendEvent(AVRCPCommand::NEXT);
    } else if (PREVIOUS == method) {
        sendEvent(AVRCPCommand::PREVIOUS);
    } else {
        LOG_ERROR << TAG_MPRISPLAYER << "reason: unsupported, method: " << method;
    }

    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void MPRISPlayer::sendEvent(const AVRCPCommand& command) {

    AVRCPCommandReceivedEvent event(command);
    m_eventBus->sendEvent(event);
}

bool MPRISPlayer::registerPlayer() {

    // Build Dict of properties.
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    // Subset of properties used for AVRCP Commands.
    g_variant_builder_add(&builder, "{sv}", CAN_GO_NEXT.c_str(), g_variant_new("b", TRUE));
    g_variant_builder_add(&builder, "{sv}", CAN_GO_PREVIOUS.c_str(), g_variant_new("b", TRUE));
    g_variant_builder_add(&builder, "{sv}", CAN_PLAY.c_str(), g_variant_new("b", TRUE));
    g_variant_builder_add(&builder, "{sv}", CAN_PAUSE.c_str(), g_variant_new("b", TRUE));
    g_variant_builder_add(&builder, "{sv}", CAN_SEEK.c_str(), g_variant_new("b", FALSE));
    g_variant_builder_add(&builder, "{sv}", CAN_CONTROL.c_str(), g_variant_new("b", TRUE));

    GVariant* properties = g_variant_builder_end(&builder);

    // OBJECT_PATH and ARRAY of STRING:VARIANT (Dictionary).
    auto parameters = g_variant_new("(o@a{sv})", m_playerPath.c_str(), properties);

    ManagedGError error;
    m_media->callMethod(REGISTER_PLAYER, parameters, error.toOutputParameter());

    if (error.hasError()) {
        LOG_ERROR << TAG_MPRISPLAYER << "reason: registerPlayerFailed, error: " << error.getMessage();
        return false;
    }

    LOG_DEBUG << TAG_MPRISPLAYER << "registerPlayerSucceeded, path: " << m_playerPath;

    return true;
}

bool MPRISPlayer::unregisterPlayer() {
    ManagedGError error;

    // OBJECT_PATH type.
    auto parameters = g_variant_new("(o)", m_playerPath.c_str());
    m_media->callMethod(UNREGISTER_PLAYER, parameters, error.toOutputParameter());

    if (error.hasError()) {
        LOG_ERROR << TAG_MPRISPLAYER << "reason: unregisterPlayerFailed, error: " << error.getMessage();
        return false;
    }

    LOG_DEBUG << TAG_MPRISPLAYER << "unregisterPlayerSucceeded";

    return true;
}

} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK

