#include <iostream>
#include <csignal>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <gio/gio.h>

#include <Common/Utils/Logger/Log.h>
#include <BlueZ/BlueZBluetoothDeviceManager.h>
#include <Common/SDKInterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>

#ifdef RASPBERRYPI_CONFIG
#include <wiringPi.h>
#define RASP_BUTTON_PIN 0
#else
#include "libsoc_gpio.h"
#include "libsoc_debug.h"
#define BBB_GPIO_INPUT   7
#endif

using namespace std;
using namespace deviceClientSDK;
using namespace deviceClientSDK::common::utils::logger;


#ifdef BLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER
#include <BlueZ/PulseAudioBluetoothInitializer.h>

// Initializer object to reload PulseAudio Bluetooth modules.
std::shared_ptr<bluetoothDevice::blueZ::PulseAudioBluetoothInitializer> m_pulseAudioInitializer;

#endif

static GMainLoop *mainloop = NULL;

//create the BluetoothDeviceManager to communicate with the Bluetooth stack.
std::unique_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager;

int countNumber = 0;

//The interrupt handler
#ifdef RASPBERRYPI_CONFIG
void raspberryInterrupt(void) {
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
#else
int beagleboneInterrupt(void* arg)
{
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
#endif

void signalHandler(int signum) {
    // cleanup and close up stuff here  
    // terminate program  
    auto hostController = bluetoothDeviceManager->getHostController();
    hostController->exitDiscoverableMode();

    auto devices = bluetoothDeviceManager->getDiscoveredDevices();
    for(const auto& device : devices) {
        if(device->isConnected()) {
                LOG_INFO << "Disconnect Command";
                device->disconnect();
        }
    }
    LOG_INFO << "Exiting the main loop";
    g_main_loop_quit(mainloop);
    exit(signum);  
}


int main() {
    // register signal SIGINT and signal handler  
    signal(SIGINT, signalHandler);

#ifdef RASPBERRYPI_CONFIG
    //setup the wiring libary
    if(wiringPiSetup() < 0) {
        LOG_ERROR << "Unable to setup Wiring Pi";
        return 1;
    }

    // Set Pin 17/0 generate an interrupt on hight to low transitions
    if(wiringPiISR(RASP_BUTTON_PIN, INT_EDGE_FALLING, &raspberryInterrupt) < 0) {
        LOG_ERROR << "Unable to setup ISR";
        return 1;
    }
#else
    // Create both gpio pointers
    gpio *gpio_input;

    // Enable debug output
    libsoc_set_debug(1);

    // Request gpios
    gpio_input = libsoc_gpio_request(BBB_GPIO_INPUT, LS_GPIO_SHARED);

     // Set direction to INPUT
    libsoc_gpio_set_direction(gpio_input, INPUT);

    // Set edge to RISING
    libsoc_gpio_set_edge(gpio_input, RISING);
    
    // Setup callback
    libsoc_gpio_callback_interrupt(gpio_input, &beagleboneInterrupt, nullptr);

#endif

    //create the event Bus.
    auto eventBus = std::make_shared<common::utils::bluetooth::BluetoothEventBus>();

#ifdef BLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER
    // Create PulseAudio initializer object before  we create the BT Device manager.
    m_pulseAudioInitializer = bluetoothDevice::blueZ::PulseAudioBluetoothInitializer::create(eventBus);

#endif

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

    LOG_INFO << "Starting main dispatching loop";
    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    LOG_INFO << "Closed Apps";

    return 0;
}