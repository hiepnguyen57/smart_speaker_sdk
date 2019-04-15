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

#ifdef BLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER
#include <BlueZ/PulseAudioBluetoothInitializer.h>

// Initializer object to reload PulseAudio Bluetooth modules.
std::shared_ptr<bluetoothDevice::blueZ::PulseAudioBluetoothInitializer> m_pulseAudioInitializer;

#endif

// Use GPIO Pin 17 
#define BUTTON_PIN 0

//create the BluetoothDeviceManager to communicate with the Bluetooth stack.
std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager;
/**
 * If you want to pair with device, you should generate a pulse on GPIO Pin 17
 *
 * Modify here, pair a device with address XX:XX:XX:XX:XX:XX
 * For Example, Name: LG Speaker, MacAddress as 30:22:11:51:6B:59
 */ 
// The interrupt handler
void myInterrupt(void) {
    auto devices = bluetoothDeviceManager->getDiscoveredDevices();
    for(const auto& device : devices) {
        if(device->getMac() == "30:22:11:51:6B:59") {
            LOG_INFO << "Pairing device with MacAddres " << device->getMac();
            if(!device->isPaired()) {
                //add device on BlueZ dbus
                device->pair();
            }
            device->connect();
        }
    }
}

// using namespace deviceClientSDK::common::utils::bluetooth;
void signalHandler(int signum) {
    // cleanup and close up stuff here  
    // terminate program  
    auto hostController = bluetoothDeviceManager->getHostController();
    hostController->stopScan();
    auto devices = bluetoothDeviceManager->getDiscoveredDevices();
    for(const auto& device : devices) {
        if(device->getMac() == "30:22:11:51:6B:59") {
            device->unpair();
        }
    }
    exit(signum);  
}

int main() {
    // register signal SIGINT and signal handler  
    signal(SIGINT, signalHandler);

    //setup the wiring libary
    if(wiringPiSetup() < 0) {
        LOG_ERROR << "Unable to setup Wiring Pi";
        return 1;
    }

    // Set Pin 17/0 generate an interrupt on hight to low transitions
    if(wiringPiISR(BUTTON_PIN, INT_EDGE_FALLING, &myInterrupt) < 0) {
        LOG_ERROR << "Unable to setup ISR";
        return 1;
    }

    auto eventBus = std::make_shared<common::utils::bluetooth::BluetoothEventBus>();
    bluetoothDeviceManager = bluetooth::blueZ::BlueZBluetoothDeviceManager::create(eventBus);

#ifdef BLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER
    // Create PulseAudio initializer object before  we create the BT Device manager.
    m_pulseAudioInitializer = bluetoothDevice::blueZ::PulseAudioBluetoothInitializer::create(eventBus);

#endif

    auto hostController = bluetoothDeviceManager->getHostController();
    if(!hostController) {
        LOG_ERROR << "nullHostController";
        return -1;
    }

    auto future = hostController->startScan();
    if(!future.valid()) {
        LOG_ERROR << "invalidFuture";
        return -1;
    }

    bool success = future.get();
    if(success) {
        auto devices = bluetoothDeviceManager->getDiscoveredDevices();
        LOG_INFO << "Scanning";
        for(const auto& device : devices) {
            LOG_INFO << "Name: " << device->getFriendlyName() << ", MacAddress as " << device->getMac();
        }
    }

    while(1) {

    }
    return 0;
}