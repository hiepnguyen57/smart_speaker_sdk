cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set project information
project(BluetoothStreamFromDevice)

option(BLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER "Override Pulseaudio BlueZ Endpoints with SDK ones" OFF)
#add_definitions(-DFILE_LOGGER)
#add_definitions(-DNDEBUG)
add_definitions(${GLIB_CFLAGS_OTHER})
find_package(Threads)

find_package(PkgConfig)
pkg_check_modules(GIO REQUIRED gio-2.0>=2.4)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0>=2.4)
pkg_check_modules(SBC REQUIRED sbc)
pkg_check_modules(GLIB glib-2.0)

include_directories(${GIO_LIBRARY_DIRS})
include_directories(${GIO_UNIX_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})

link_directories(${GIO_LIBRARY_DIRS})
link_directories(${GIO_UNIX_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

add_definitions(${GIO_CFLAGS})

#Bring the headers into the project
include_directories(../../include 
                    ../../../BlueZ/include
                    ../../../../Common/Utils/include
                    ../../../../Common/SDKInterfaces/include
)

#add the sources using the set command as follows:
set(SOURCES ../../../../Common/Utils/src/Logger/Level.cpp
            ../../../../Common/Utils/src/Bluetooth/SDPRecords.cpp
            ../../../../Common/Utils/src/Threading/Executor.cpp
            ../../../../Common/Utils/src/Threading/TaskThread.cpp
            ../../../../Common/Utils/src/Threading/ThreadMoniker.cpp
            ../../../../Common/Utils/src/BluetoothEventBus.cpp
            ../../../../Common/Utils/src/MacAddressString.cpp
            ../../../../Common/Utils/src/RequiresShutdown.cpp
            ../../../../Common/Utils/src/UUIDGeneration.cpp
            ../../../../Common/Utils/src/FormattedAudioStreamAdapter.cpp
            ../../../../BluetoothDevice/BlueZ/src/BlueZA2DPSink.cpp
            ../../../../BluetoothDevice/BlueZ/src/BlueZA2DPSource.cpp
            ../../../../BluetoothDevice/BlueZ/src/BlueZAVRCPController.cpp
            ../../../../BluetoothDevice/BlueZ/src/BlueZAVRCPTarget.cpp
            ../../../../BluetoothDevice/BlueZ/src/BlueZBluetoothDevice.cpp
            ../../../../BluetoothDevice/BlueZ/src/BlueZBluetoothDeviceManager.cpp
            ../../../../BluetoothDevice/BlueZ/src/BlueZDeviceManager.cpp
            ../../../../BluetoothDevice/BlueZ/src/BlueZHostController.cpp
            ../../../../BluetoothDevice/BlueZ/src/DBusConnection.cpp
            ../../../../BluetoothDevice/BlueZ/src/DBusObjectBase.cpp
            ../../../../BluetoothDevice/BlueZ/src/DBusPropertiesProxy.cpp
            ../../../../BluetoothDevice/BlueZ/src/DBusProxy.cpp
            ../../../../BluetoothDevice/BlueZ/src/GVariantMapReader.cpp
            ../../../../BluetoothDevice/BlueZ/src/GVariantTupleReader.cpp
            ../../../../BluetoothDevice/BlueZ/src/MPRISPlayer.cpp
            ../../../../BluetoothDevice/BlueZ/src/PairingAgent.cpp
            ../../../../BluetoothDevice/BlueZ/src/MediaContext.cpp
            ../../../../BluetoothDevice/BlueZ/src/MediaEndpoint.cpp
            BluetoothStreamFromDevice.cpp
)

if(BLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER)
    pkg_check_modules(PULSEAUDIO REQUIRED libpulse>=8.0)
    include_directories(${PULSEAUDIO_INCLUDE_DIRS})
    add_definitions(-DBLUETOOTH_BLUEZ_PULSEAUDIOINITIALIZER)
    list(APPEND SOURCES ../../../../BluetoothDevice/BlueZ/src/PulseAudioBluetoothInitializer.cpp)
    target_link_libraries(BluetoothStreamFromDevice ${PULSEAUDIO_LDFLAGS})

endif()


add_executable(BluetoothStreamFromDevice ${SOURCES}) 
target_link_libraries(BluetoothStreamFromDevice ${CMAKE_THREAD_LIBS_INIT} ${GIO_LDFLAGS} ${Glib_LIBRARY} "sbc" -lwiringPi)