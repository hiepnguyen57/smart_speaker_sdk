#Device SDK for smart speaker device
##Dependencies
This is list of requirement and dependencies for the Device SDK for C++.

##Requirements
The device SDK runs on Raspberry Pi, Beaglebone. It requires C++11 or later.

##Bluetooth
Building with Bluetooth is optional and is currently limited to Raspberry Pi and Beaglebone. `A2DP-SINK`,
`A2DP-SOURCE`, `AVRCPTarget` and `AVRCPController` profiles are supported.

If you choose to build with Bluetooth, these libraries and modules, and their dependencies, must be installed:

`Library`                                                   `Minimum version`
`SBC library`                                                     1.3
`BlueZ 5.37`                                                      5.37
`libpulse-dev`. Only required if enabling `Cmake variable`:       8.0
    `BLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER`

`Module`                                                    `Minimum version`
`Pulseaudio` and `Pulseaudio Bluetooth` - Used to handle        12.2 or earlier
    audio routing.