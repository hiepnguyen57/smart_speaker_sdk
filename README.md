# Device SDK for smart speaker device
### Dependencies
This is list of requirement and dependencies for the Device SDK for C++.

### Requirements
The device SDK runs on Raspberry Pi, Beaglebone. It requires C++11 or later.

### Bluetooth
Building with Bluetooth is optional and is currently limited to Raspberry Pi and Beaglebone. `A2DP-SINK`,
`A2DP-SOURCE`, `AVRCPTarget` and `AVRCPController` profiles are supported.

If you choose to build with Bluetooth, these libraries and modules, and their dependencies, must be installed:

| Library | Minimum version |
| ------ | ------ |
| `SBC` library | 1.3 |
| `BlueZ` 5.37 | 5.37 |
| `libpulse-dev`. Only required if enabling Cmake variable `BLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER` | 8.0 |

| Module | Minimum Version |
| ------- | ------ |
| `Pulseaudio` and `Pulseaudio Bluetooth`. Used to handle audio routing | 12.2 or earlier |

### Build and run example
Go to direction of example
```
cd omni-device-sdk/BluetoothDevice/BlueZ/test/Discoverable
```

Create folder for building
```
mkdir build && cd build
```

Cmake generates Makefiles
```
cmake ..
```

Type command to build
```
make
```

Run example
```
./BluetoothStreamFromDevice
```

In discoverable Mode:
```
pi@raspberrypi:~/omni-device-sdk/BluetoothDevice/BlueZ/test/Discoverable/build $ ./BluetoothStreamFromDevice
[2019-07-02 05:31:56][INFO]     Discoverable On
[2019-07-02 05:31:56][INFO]     Starting main dispatching loop
[2019-07-02 05:32:16][DEBUG]    BlueZBluetoothDevice    connectedChanged: 1
[2019-07-02 05:32:19][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: paused
[2019-07-02 05:32:20][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: stopped
[2019-07-02 05:32:29][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: paused
[2019-07-02 05:32:29][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: playing
[2019-07-02 05:33:00][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: paused
[2019-07-02 05:33:08][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: playing
[2019-07-02 05:33:16][INFO]     Pause Command
[2019-07-02 05:33:16][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: paused
[2019-07-02 05:33:26][INFO]     Play Command
[2019-07-02 05:33:26][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: playing
[2019-07-02 05:33:36][INFO]     Next Command
[2019-07-02 05:33:44][INFO]     Previous Command
[2019-07-02 05:33:44][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: paused
[2019-07-02 05:33:44][DEBUG]    BlueZDeviceManager      onMediaPlayerPropertyChanged, currentStatus: playing
[2019-07-02 05:33:49][INFO]     Disconnect Command
[2019-07-02 05:33:52][DEBUG]    BlueZBluetoothDevice    connectedChanged: 0
[2019-07-02 05:34:53][INFO]     Exiting the main loop
[2019-07-02 05:34:53][INFO]     BlueZDeviceManager      Clean all before exit
[2019-07-02 05:34:53][INFO]     Exiting the main loop
```

And Pairing Mode:

```
pi@raspberrypi:~/omni-device-sdk/BluetoothDevice/BlueZ/test/Pairing/build $ ./BluetoothStreamToDevice
[2019-07-02 05:34:58][INFO]     Scanning
[2019-07-02 05:34:58][INFO]     Name: JBL Clip 2, MacAddress as 04:FE:A1:9E:C1:CB
[2019-07-02 05:34:58][INFO]     Name: Thuan's Mi Phone, MacAddress as 20:47:DA:FB:2F:29
[2019-07-02 05:34:58][INFO]     Name: B13, MacAddress as 30:22:11:51:6B:59
[2019-07-02 05:34:58][INFO]     Name: HTC Desire 12+, MacAddress as 40:4E:36:E2:49:0F
[2019-07-02 05:34:58][INFO]     Name: Olli Comdt05, MacAddress as 88:1F:A1:7A:44:29
[2019-07-02 05:34:58][INFO]     Name: Hiep Nokia 7 Plus, MacAddress as A0:28:ED:AB:59:E1
[2019-07-02 05:34:58][INFO]     Name: F3-E7-91-26-01-0C, MacAddress as F3:E7:91:26:01:0C
[2019-07-02 05:34:58][INFO]     Starting main dispatching loop
[2019-07-02 05:35:04][INFO]     Pairing device with MacAddres 30:22:11:51:6B:59
[2019-07-02 05:35:09][DEBUG]    BlueZBluetoothDevice    connectedChanged: 1
[2019-07-02 05:36:49][INFO]     Exiting the main loop
[2019-07-02 05:36:49][INFO]     BlueZDeviceManager      Clean all before exit
[2019-07-02 05:36:52][DEBUG]    MediaEndpoint   Exiting media thread
```

### Issue
A2DP Support. 
Now let’s check that A2DP streaming is working. We start by checking that PulseAudio is listing the Bluetooth sound card:
```
pacmd list-cards
```

The Bluetooth card will be index #1, you can also see the supported profiles (a2dp, hsp, off…). Set A2DP as active profile:
```
pacmd set-card-profile bluez_card.xx_xx_xx_xx_xx_xx a2dp_sink
```

Set the Bluetooth device as output audio:
```
pacmd set-default-sink bluez_sink.xx_xx_xx_xx_xx_xx.a2dp_sink
```