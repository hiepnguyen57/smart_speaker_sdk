#include "Common/Utils/Logger/Log.h"
#include "Common/Utils/Bluetooth/BluetoothEventBus.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {

using namespace logger;

#define TAG_BLUETOOTHEVENTBUS       "BluetoothEventBus\t"

BluetoothEventBus::BluetoothEventBus() {

}

void BluetoothEventBus::sendEvent(const BluetoothEvent& event) {
    ListenerList listenerList;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto mapIterator = m_listenerMap.find(event.getType());
        if(mapIterator == m_listenerMap.end()) {
            // No listener registered.
            return;
        }

        listenerList = mapIterator->second;
    }

    for(auto listenerWeakPtr : listenerList) {
        std::shared_ptr<BluetoothEventListenerInterface> listener = listenerWeakPtr.lock();
        if(listener != nullptr) {
            listener->onEventFired(event);
        }
    }
}

void BluetoothEventBus::addListener(
    const std::vector<BluetoothEventType>& eventTypes,
    std::shared_ptr<BluetoothEventListenerInterface> listener) {
    if(listener == nullptr) {
        LOG_ERROR << TAG_BLUETOOTHEVENTBUS << "addListenerFailed, reason: Listener cannot be null";
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    for(BluetoothEventType eventType : eventTypes) {
        ListenerList& listenerList = m_listenerMap[eventType];

        auto iter = listenerList.begin();
        while(iter != listenerList.end()) {
            auto listenerWeakPtr = *iter;
            auto listenerPtr = listenerWeakPtr.lock();
            if(listenerPtr == nullptr) {
                iter = listenerList.erase(iter);
            } else {
                if(listenerPtr == listener) {
                    LOG_ERROR << TAG_BLUETOOTHEVENTBUS << "addListenerFailed, reason: The same listener already exists";
                    break;
                }
                ++iter;
            }
        }

        if(iter == listenerList.end()) {
            listenerList.push_back(listener);
        }
    }
}

void BluetoothEventBus::removeListener(
    const std::vector<BluetoothEventType>& eventTypes,
    std::shared_ptr<BluetoothEventListenerInterface> listener) {
    if(listener == nullptr) {
        LOG_ERROR << TAG_BLUETOOTHEVENTBUS << "removeListenerFailed, reason: Listener cannot be null";
        return;       
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    for(BluetoothEventType eventType : eventTypes) {
        auto mapIterator = m_listenerMap.find(eventType);
        if(mapIterator == m_listenerMap.end()) {
            LOG_ERROR << TAG_BLUETOOTHEVENTBUS << "removeListenerFailed, reason: Listener not subcribed";
        }

        ListenerList& listenerList = mapIterator->second;

        auto iter = listenerList.begin();
        while(iter != listenerList.end()) {
            auto listenerWeakPtr = *iter;
            auto listenerPtr = listenerWeakPtr.lock();
            if(listenerPtr == nullptr) {
                iter = listenerList.erase(iter);
            } else if(listenerPtr == listener) {
                listenerList.erase(iter);
                break;
            }
            else {
                ++iter;
            }
        }
        if(listenerList.empty()) {
            m_listenerMap.erase(mapIterator);
        }
    }
}

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK