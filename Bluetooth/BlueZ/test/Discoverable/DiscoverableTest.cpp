#include <iostream>
#include <csignal>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>

#include <Common/Utils/Logger/Log.h>
#include <BlueZ/BlueZBluetoothDeviceManager.h>
#include <Common/SDKInterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>

using namespace std;
using namespace deviceClientSDK;
using namespace deviceClientSDK::common::utils::logger;

// Use GPIO Pin 17 
#define BUTTON_PIN 0

//create the BluetoothDeviceManager to communicate with the Bluetooth stack.
std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager;

/**
 * If you want to disconnect with device, you should generate a pulse on GPIO Pin 17
 */
// The interrupt handler
void myInterrupt(void) {
    auto devices = bluetoothDeviceManager->getDiscoveredDevices();
    for(const auto& device : devices) {
        if(device->isConnected()) {
            LOG_INFO << "Device: " << device->getMac() << " was disconnected";
            device->disconnect();
        }
    }
}

void signalHandler(int signum) {
    // cleanup and close up stuff here  
    // terminate program  
    auto hostController = bluetoothDeviceManager->getHostController();
    hostController->exitDiscoverableMode();
    exit(signum);  
}

int main() {
    // register signal SIGINT and signal handler  
    signal(SIGINT, signalHandler);

    //setup the wiring libary
    if(wiringPiSetup() < 0) {
        LOG_ERROR << "Unable to setup Wiring Pi";
        return -1;
    }

    // Set Pin 17/0 generate an interrupt on hight to low transitions
    if(wiringPiISR(BUTTON_PIN, INT_EDGE_FALLING, &myInterrupt) < 0) {
        LOG_ERROR << "Unable to setup ISR";
        return -1;
    }

    auto eventBus = std::make_shared<common::utils::bluetooth::BluetoothEventBus>();
    bluetoothDeviceManager = bluetooth::blueZ::BlueZBluetoothDeviceManager::create(eventBus);

    auto hostController = bluetoothDeviceManager->getHostController();
    if(!hostController) {
        LOG_ERROR << "nullHostController";
        return -1;
    }

    // Enable Discoverable Mode
    if(!hostController->isDiscoverable()) {
        LOG_INFO << "Discoverable On";
        hostController->enterDiscoverableMode();
    }

    while(1) {

    }
    return 0;
}