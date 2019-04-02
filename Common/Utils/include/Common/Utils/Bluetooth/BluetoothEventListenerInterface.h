#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTLISTENERINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTLISTENERINTERFACE_H_

#include "Common/Utils/Bluetooth/BluetoothEvents.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {

/**
 * Base interface for the objects listening to events.
 */
class BluetoothEventListenerInterface {
public:
    /**
     * Destructor.
     */
    virtual ~BluetoothEventListenerInterface() = default;

    /**
     * Method called to process an event of the specific type.
     * 
     * @return event Event to be processed.
     */
    virtual void onEventFired(const BluetoothEvent& event) = 0;
};

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTLISTENERINTERFACE_H_