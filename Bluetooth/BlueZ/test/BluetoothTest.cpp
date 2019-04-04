#include <Common/Utils/Logger/Log.h>
#include <BlueZ/BlueZBluetoothDeviceManager.h>
#include <Common/SDKInterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>

using namespace deviceClientSDK;
using namespace deviceClientSDK::common::utils::logger;

// using namespace deviceClientSDK::common::utils::bluetooth;
// using namespace deviceClientSDK::common::sdkInterfaces::bluetooth;

#define TAG_BLUETOOTHTEST           "BluetoothTest\t"

int main() {
    //create the BluetoothDeviceManager to communicate with the Bluetooth stack.
    std::unique_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager;
    std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothHostControllerInterface> hostController;
    auto eventBus = std::make_shared<common::utils::bluetooth::BluetoothEventBus>();

    bluetoothDeviceManager = bluetooth::blueZ::BlueZBluetoothDeviceManager::create(eventBus);


    hostController = bluetoothDeviceManager->getHostController();
    LOG_INFO << "Discoverable On";
    hostController->enterDiscoverableMode();

    while(1) {

    }
    return 0;
}