#include <iostream>
#include <csignal>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>

#include <Common/Utils/Logger/Log.h>
#include <BlueZ/BlueZBluetoothDeviceManager.h>
#include <Common/SDKInterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>
//#include <BlueZ/PulseAudioBluetoothInitializer.h>

using namespace std;
using namespace deviceClientSDK;
using namespace deviceClientSDK::common::utils::logger;

// Use GPIO Pin 17 
#define BUTTON_PIN 0

// Initializer object to reload PulseAudio Bluetooth modules.
//std::shared_ptr<bluetoothDevice::blueZ::PulseAudioBluetoothInitializer> m_pulseAudioInitializer;

//create the BluetoothDeviceManager to communicate with the Bluetooth stack.
std::unique_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager;

int countNumber = 0;
/**
 * If you want to disconnect with device, you should generate a pulse on GPIO Pin 17
 */
// The interrupt handler
void myInterrupt(void) {
    auto devices = bluetoothDeviceManager->getDiscoveredDevices();
    for(const auto& device : devices) {
        if(device->isConnected()) {
            auto avrcpTarget = device->getAVRCPTarget();
            if(!avrcpTarget) {
                LOG_ERROR << "AVRCP not supported";
                return;
            }
            switch(countNumber) {
                case 0:
                    LOG_INFO << "Pause Command";
                    avrcpTarget->pause();
                    break;
                case 1:
                    LOG_INFO << "Play Command";
                    avrcpTarget->play();
                    break;
                case 2:
                    LOG_INFO << "Next Command";
                    avrcpTarget->next();
                    break;
                case 3:
                    LOG_INFO << "Previous Command";
                    avrcpTarget->previous();
                    break;
                case 4:
                    LOG_INFO << "Disconnect Command";
                    device->disconnect();
                    break;
            }

        }
    }
    countNumber++;
    if(countNumber > 4) {
        countNumber = 0;
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

    //create the event Bus.
    auto eventBus = std::make_shared<common::utils::bluetooth::BluetoothEventBus>();

    // Create PulseAudio initializer object before  we create the BT Device manager.
    //m_pulseAudioInitializer = bluetoothDevice::blueZ::PulseAudioBluetoothInitializer::create(eventBus);
    
    bluetoothDeviceManager = bluetoothDevice::blueZ::BlueZBluetoothDeviceManager::create(eventBus);

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