#include "BlueZ/BlueZBluetoothDeviceManager.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

using namespace common::sdkInterfaces::bluetooth;
using namespace common::utils::bluetooth;

std::shared_ptr<BluetoothHostControllerInterface> BlueZBluetoothDeviceManager::getHostController() {
    return m_deviceManager->getHostController();
}

std::list<std::shared_ptr<BluetoothDeviceInterface>> BlueZBluetoothDeviceManager::getDiscoveredDevices() {
    return m_deviceManager->getDiscoveredDevices();
}

std::unique_ptr<BlueZBluetoothDeviceManager> BlueZBluetoothDeviceManager::create(
    std::shared_ptr<BluetoothEventBus> eventBus) {
    auto deviceManager = BlueZDeviceManager::create(eventBus);
    if(!deviceManager) {
        return nullptr;
    }
    return std::unique_ptr<BlueZBluetoothDeviceManager>(new BlueZBluetoothDeviceManager(deviceManager));
}

BlueZBluetoothDeviceManager::BlueZBluetoothDeviceManager(std::shared_ptr<BlueZDeviceManager> deviceManager) :
        m_deviceManager{deviceManager} {
}

BlueZBluetoothDeviceManager::~BlueZBluetoothDeviceManager() {
    m_deviceManager->shutdown();
};

std::shared_ptr<BluetoothEventBus> BlueZBluetoothDeviceManager::getEventBus() {
    return m_deviceManager->getEventBus();
}

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK