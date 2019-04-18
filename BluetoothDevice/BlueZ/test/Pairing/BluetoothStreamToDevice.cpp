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
std::shared_ptr<common::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager;
/**
 * If you want to pair with device, you should generate a pulse on GPIO Pin 17
 *
 * Modify here, pair a device with address XX:XX:XX:XX:XX:XX
 * For Example, Name: JBL Clip 2, MacAddress as 04:FE:A1:9E:C1:CB
 */ 
//The interrupt handler
#ifdef RASPBERRYPI_CONFIG
void raspberryInterrupt(void) {
    auto devices = bluetoothDeviceManager->getDiscoveredDevices();
    for(const auto& device : devices) {
        if(device->getMac() == "04:FE:A1:9E:C1:CB") {
            LOG_INFO << "Pairing device with MacAddres " << device->getMac();
            if(!device->isPaired()) {
                //add device on BlueZ dbus
                device->pair();
            }
            device->connect();
        }
    }
}
#else
int beagleboneInterrupt(void* arg)
{
    auto devices = bluetoothDeviceManager->getDiscoveredDevices();
    for(const auto& device : devices) {
        if(device->getMac() == "04:FE:A1:9E:C1:CB") {
            LOG_INFO << "Pairing device with MacAddres " << device->getMac();
            if(!device->isPaired()) {
                //add device on BlueZ dbus
                device->pair();
            }
            device->connect();
        }
    }  
    return 0;
}
#endif

// exit signal handler
void signalHandler(int signum) {
    // cleanup and close up stuff here  
    // terminate program

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

    LOG_INFO << "Starting main dispatching loop";
    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    LOG_INFO << "Closed Apps";

    return 0;
}