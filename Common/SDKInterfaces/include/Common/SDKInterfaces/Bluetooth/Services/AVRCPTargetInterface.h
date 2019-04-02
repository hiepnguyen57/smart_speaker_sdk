#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPTARGETINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPTARGETINTERFACE_H_

#include <ostream>
#include <string>

#include "Common/SDKInterfaces/Bluetooth/Services/BluetoothServiceInterface.h"

namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {
namespace services {

// TODO: Move to own enum file
// An enum representing AVRCP commands.
enum class AVRCPCommand {
    // A Play command
    PLAY,
    // A Pause command
    PAUSE,
    // A Next command
    NEXT,
    // A previous command.
    PREVIOUS
};

/**
 * Converts the @c AVRCPCommand enum to a string.
 * 
 * @param cmd the @c AVRCPCommand to covert.
 * @return A string representation of the @c AVRCPCommand
 */
inline std::string commandToString(AVRCPCommand cmd) {
    switch(cmd)
    {
        case AVRCPCommand::PLAY:
            return "PLAY";
        case AVRCPCommand::PAUSE:
            return "PAUSE";
        case AVRCPCommand::NEXT:
            return "NEXT";
        case AVRCPCommand::PREVIOUS:
            return "PREVIOUS";
    }

    return "UNKOWN";
}

/**
 * Overload for the @c AVRCPCommand enum. This will write the @c AVRCPCommand as a string to the provided stream.
 *
 * @param stream An ostream to send the DeviceState as a string.
 * @param cmd The @c AVRCPCommand to convert.
 * @return The stream.
 */
inline std::ostream& operator<<(std::ostream& stream, const AVRCPCommand cmd) {
    return stream << commandToString(cmd);
}

/// Used to implement an instance of AVRCPTarget (Audio/Video Remote Control Profile).
class AVRCPTargetInterface : public BluetoothServiceInterface  {
public:
    // The Service UUID
    static constexpr const char* UUID = "0000110c-0000-1000-8000-00805f9b34fb";
    
    // The Service Name
    static constexpr const char* NAME = "A/V_RemoteControlTarget";

    /**
     * Sends a play command to the device supporting AVRCPTarget.
     *
     * @return A boolean indicating the success of the function.
     */
    virtual bool play() = 0;

    /**
     * Sends a pause command to the device supporting AVRCPTarget.
     *
     * @return A boolean indicating the success of the function.
     */
    virtual bool pause() = 0;

    /**
     * Sends a next command to the device supporting AVRCPTarget.
     *
     * @return A boolean indicating the success of the function.
     */
    virtual bool next() = 0;

    /**
     * Sends a previous command to the device supporting AVRCPTarget.
     *
     * @return A boolean indicating the success of the function.
     */
    virtual bool previous() = 0;

    /**
     * Destructor.
     */
    virtual ~AVRCPTargetInterface() = default;
};


}  // namespace services
}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK
#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPTARGETINTERFACE_H_
