cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set project information
project(pairingTest)

#add_definitions(-DFILE_LOGGER)
#add_definitions(-DNDEBUG)

#Bring the headers into the project
include_directories(../../include 
                    ../../../BlueZ/include
                    ../../../../Common/Utils/include
                    ../../../../Common/SDKInterfaces/include
                    /usr/include
)

find_package(PkgConfig)
pkg_check_modules(GLIB glib-2.0)
pkg_check_modules(GIO gio-unix-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_LIBRARY_DIRS})

link_directories(${GLIB_LIBRARY_DIRS})
link_directories(${GIO_LIBRARY_DIRS})

add_definitions(${GLIB_CFLAGS_OTHER})
add_definitions(${GIO_CFLAGS})

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
            ../../../../Bluetooth/BlueZ/src/BlueZA2DPSink.cpp
            ../../../../Bluetooth/BlueZ/src/BlueZA2DPSource.cpp
            ../../../../Bluetooth/BlueZ/src/BlueZAVRCPController.cpp
            ../../../../Bluetooth/BlueZ/src/BlueZAVRCPTarget.cpp
            ../../../../Bluetooth/BlueZ/src/BlueZBluetoothDevice.cpp
            ../../../../Bluetooth/BlueZ/src/BlueZBluetoothDeviceManager.cpp
            ../../../../Bluetooth/BlueZ/src/BlueZDeviceManager.cpp
            ../../../../Bluetooth/BlueZ/src/BlueZHostController.cpp
            ../../../../Bluetooth/BlueZ/src/DBusConnection.cpp
            ../../../../Bluetooth/BlueZ/src/DBusObjectBase.cpp
            ../../../../Bluetooth/BlueZ/src/DBusPropertiesProxy.cpp
            ../../../../Bluetooth/BlueZ/src/DBusProxy.cpp
            ../../../../Bluetooth/BlueZ/src/GVariantMapReader.cpp
            ../../../../Bluetooth/BlueZ/src/GVariantTupleReader.cpp
            ../../../../Bluetooth/BlueZ/src/MPRISPlayer.cpp
            ../../../../Bluetooth/BlueZ/src/PairingAgent.cpp
            PairingTest.cpp
)

find_package(Threads)
add_executable(pairingTest ${SOURCES}) 
target_link_libraries(pairingTest ${CMAKE_THREAD_LIBS_INIT} ${GIO_LDFLAGS} ${Glib_LIBRARY} -lwiringPi)