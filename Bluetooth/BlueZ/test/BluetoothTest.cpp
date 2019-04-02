#include <Common/Utils/Logger/Log.h>
#include "BlueZ/BlueZDeviceManager.h"

using namespace common::utils::logger;
using namespace common::utils::bluetooth;
using namespace common::sdkInterfaces::bluetooth;

#define TAG_BLUETOOTHTEST           "BluetoothTest\t"

int main() {
    LOG_INFO << TAG_BLUETOOTHTEST;
    return 0;
}