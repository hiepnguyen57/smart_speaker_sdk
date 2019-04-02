#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTS_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTS_H_

#include "Common/SDKInterfaces/Bluetooth/BluetoothDeviceInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/A2DPSourceInterface.h"
#include "Common/SDKInterfaces/Bluetooth/Services/AVRCPTargetInterface.h"
#include "Common/Utils/Bluetooth/A2DPRole.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {

// The different Bluetooth event types.
enum class BluetoothEventType {
    // Represents when a device is discovered.
    DEVICE_DISCOVERED,
    // Represents when a device is removed.
    DEVICE_REMOVED,
    // Represents when the state of a device changes.
    DEVICE_STATE_CHANGED,
    // Represents when the A2DP streaming state changes.
    STREAMING_STATE_CHANGED,
    // Represents when an AVRCP command has been receviced.
    AVRCP_COMMAND_RECEIVED,
    // When the BluetoothDeviceManager has initialized.
    BLUETOOTH_DEVICE_MANAGER_INITIALIZED
};

// Helper struct allow enum class to be a key in collections.
struct BluetoothEventTypeHash {
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

// An Enum representing the current state of the stream.
enum class MediaStreamingState {
    // Currently not streaming.
    IDLE,
    // Currently acquiring the stream.
    PENDING,
    //Currently streaming.
    ACTIVE
};

class BluetoothEvent
{
public:
    /**
     * Destructor.
     */
    virtual ~BluetoothEvent() = default;

    /**
     * Get Event type.
     * @return Event type.
     */
    BluetoothEventType getType() const;

    /**
     * Get @c BluetoothDeviceInterface associated with the event.
     * @return @c BluetoothDeviceInterface associated with the event or nullptr.
     */
    std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> getDevice() const;

    /**
     * Get @c DeviceState associated with the event.
     */
    sdkInterfaces::bluetooth::DeviceState getDeviceState() const;

    // Get @c MediaStream associated with the event.
    MediaStreamingState getMediaStreamingState() const;

    // Get @c A2DPRole associated with the event.
    std::shared_ptr<A2DPRole> getA2DPRole() const;

    // Get @c AVRCP command associated with the event.
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::AVRCPCommand> getAVRCPCommand() const;

protected:
    /**
     * Constructor
     */
    BluetoothEvent(
        BluetoothEventType type,
        std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> device = nullptr,
        common::sdkInterfaces::bluetooth::DeviceState deviceState = 
            common::sdkInterfaces::bluetooth::DeviceState::IDLE,
        MediaStreamingState mediaStreamingState = MediaStreamingState::IDLE,
        std::shared_ptr<A2DPRole> a2dpRole =  nullptr,
        std::shared_ptr<common::sdkInterfaces::bluetooth::services::AVRCPCommand> avrcpCommand = nullptr
    );

private:
    // Event Type
    BluetoothEventType m_type;

    // @c BluetoothDeviceInterface associated with the event
    std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> m_device;
  
    // @c DeviceState associated with the event
    common::sdkInterfaces::bluetooth::DeviceState m_deviceState;

    // @c MediaStreamingState associated with the event
    MediaStreamingState m_mediaStreamingState;

    // @c A2DPRole associated with the event
    std::shared_ptr<A2DPRole> m_a2dpRole;

    // @C AVRCPCommand that is received
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::AVRCPCommand> m_avrcpCommand;
};

inline BluetoothEvent::BluetoothEvent(
    BluetoothEventType type,
    std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> device,
    common::sdkInterfaces::bluetooth::DeviceState deviceState,
    MediaStreamingState mediaStreamingState,
    std::shared_ptr<A2DPRole> a2dpRole,
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::AVRCPCommand> avrcpCommand) :
        m_type{type},
        m_device{device},
        m_deviceState{deviceState},
        m_mediaStreamingState{mediaStreamingState},
        m_a2dpRole{a2dpRole},
        m_avrcpCommand{avrcpCommand} {

}

inline BluetoothEventType BluetoothEvent::getType() const {
    return m_type;
}

inline std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> BluetoothEvent::getDevice() const {
    return m_device;
}

inline MediaStreamingState BluetoothEvent::getMediaStreamingState() const {
    return m_mediaStreamingState;
}

inline common::sdkInterfaces::bluetooth::DeviceState BluetoothEvent::getDeviceState() const {
    return m_deviceState;
}

inline std::shared_ptr<A2DPRole> BluetoothEvent::getA2DPRole() const {
    return m_a2dpRole;
}

inline std::shared_ptr<common::sdkInterfaces::bluetooth::services::AVRCPCommand> BluetoothEvent::getAVRCPCommand() const {
    return m_avrcpCommand;
}


/**
 * Event indicating that a new device was discovered. This must be sent when
 * a new device is discovered with a reference to the @c BluetoothDeviceInterface.
 */
class DeviceDiscoveredEvent : public BluetoothEvent {
public:
    /**
     * Constructor
     * @param device Device associated with the event.
     */

    explicit DeviceDiscoveredEvent(
        const std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>& device);

};

inline DeviceDiscoveredEvent::DeviceDiscoveredEvent(
    const std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>& device) : 
        BluetoothEvent(
            BluetoothEventType::DEVICE_DISCOVERED,
            device,
            common::sdkInterfaces::bluetooth::DeviceState::IDLE,
            MediaStreamingState::IDLE) {

}

/**
 * Event indicating that a device is removed from the underlying stack, if applicable.
 * This signifies that the stack is no longer aware of the device. For example,
 * if the stack forgets an unpaired device. This must be sent with a reference
 * to the @c BluetoothDeviceInterface.
 */
class DeviceRemovedEvent : public BluetoothEvent {
public:
    /**
     * Constructor.
     * @param device Device associated with the event.
     */
    explicit DeviceRemovedEvent(
        const std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>& device);
};

inline DeviceRemovedEvent::DeviceRemovedEvent(
    const std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface>& device) : 
        BluetoothEvent(
            BluetoothEventType::DEVICE_REMOVED,
            device,
            common::sdkInterfaces::bluetooth::DeviceState::IDLE,
            MediaStreamingState::IDLE) {
}

/**
 * Event indicating that a device undergoes a state transition. The available states are
 * dictated by @c DeviceState. This must be sent with a reference to the @c BluetoothDeviceInterface.
 */
class DeviceStateChangedEvent : public BluetoothEvent {
public:
    /**
     * Constructor.
     * @param device Device associated with the event.
     */
    DeviceStateChangedEvent(
        std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> device,
        common::sdkInterfaces::bluetooth::DeviceState newState);
};

inline DeviceStateChangedEvent::DeviceStateChangedEvent(
    std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> device,
    common::sdkInterfaces::bluetooth::DeviceState newState) : 
        BluetoothEvent(
            BluetoothEventType::DEVICE_STATE_CHANGED,
            device,
            newState,
            MediaStreamingState::IDLE) {

}

/**
 * Event indicating that a device's streaming state has changed. This refers to the
 * A2DP profile. This must be sent on transitions between @c MediaStreamingState.
 */
class MediaStreamingStateChangedEvent : public BluetoothEvent {
public:
    /**
     * Constructor.
     */
    explicit MediaStreamingStateChangedEvent(
        MediaStreamingState newState,
        A2DPRole role,
        std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> device
    );
};

inline MediaStreamingStateChangedEvent::MediaStreamingStateChangedEvent(
    MediaStreamingState newState,
    A2DPRole role,
    std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceInterface> device) : 
        BluetoothEvent(
            BluetoothEventType::STREAMING_STATE_CHANGED,
            device,
            common::sdkInterfaces::bluetooth::DeviceState::IDLE,
            newState,
            std::make_shared<A2DPRole>(role)) {

}

/**
 * Event indicating that an AVRCP command was received.
 */
class AVRCPCommandReceivedEvent : public BluetoothEvent {
public:
    explicit AVRCPCommandReceivedEvent(
        common::sdkInterfaces::bluetooth::services::AVRCPCommand command);
};

inline AVRCPCommandReceivedEvent::AVRCPCommandReceivedEvent(
    common::sdkInterfaces::bluetooth::services::AVRCPCommand command) : 
    BluetoothEvent(
        BluetoothEventType::AVRCP_COMMAND_RECEIVED,
        nullptr,
        common::sdkInterfaces::bluetooth::DeviceState::IDLE,
        MediaStreamingState::IDLE,
        nullptr,
        std::make_shared<common::sdkInterfaces::bluetooth::services::AVRCPCommand>(command)) {

}

/**
 * Event indicating that the BluetoothDeviceManager has finished initialization. This should only be sent once.
 */
class BluetoothDeviceManagerInitializedEvent : public BluetoothEvent {
public:
    explicit BluetoothDeviceManagerInitializedEvent();
};

inline BluetoothDeviceManagerInitializedEvent::BluetoothDeviceManagerInitializedEvent() : 
    BluetoothEvent(BluetoothEventType::BLUETOOTH_DEVICE_MANAGER_INITIALIZED) {

}

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTS_H_