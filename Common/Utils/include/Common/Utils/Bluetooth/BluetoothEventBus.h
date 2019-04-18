#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTBUS_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTBUS_H_

#include <mutex>
#include <memory>
#include <unordered_map>
#include <list>
#include <string>
#include <vector>

#include "Common/Utils/Bluetooth/BluetoothEvents.h"
#include "Common/Utils/Bluetooth/BluetoothEventListenerInterface.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {

/**
 * Event bus class for Bluetooth CA. Publishes Bluetooth events to all listeners.
 */
class BluetoothEventBus {
public:
    /**
     * Constructor.
     */
    BluetoothEventBus();

    /**
     * A type representing a collection of @c EventLister objects.
     */
    using ListenerList = std::list<std::weak_ptr<BluetoothEventListenerInterface>>;

    /**
     * Send the event to @c EventBus. Method block untils all the listeners process the event. The method is thread safe.
     *
     * @param event Event to be sent to @c EventBus.
     */
    void sendEvent(const BluetoothEvent& event);

    /**
     * Add a listener to the bus.
     */
    void addListener(
        const std::vector<BluetoothEventType>& eventTypes,
        std::shared_ptr<BluetoothEventListenerInterface> listener
    );

    /**
     * Remove a listener from the @c EventBus.
     */
    void removeListener(
        const std::vector<BluetoothEventType>& eventTypes,
        std::shared_ptr<BluetoothEventListenerInterface> listener
    );

private:
    // Mutex used to synchronize access to subcribed listen list.
    std::mutex m_mutex;

    // A collection of @c EventListener<EventT> objects grouped by event type Id. Each listener may be subcribed to any
    // number of event types.
    std::unordered_map<BluetoothEventType, ListenerList, BluetoothEventTypeHash> m_listenerMap;
};

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTBUS_H_
