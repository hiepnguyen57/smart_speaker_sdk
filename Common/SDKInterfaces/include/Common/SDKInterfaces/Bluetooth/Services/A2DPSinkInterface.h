#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSINKINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSINKINTERFACE_H_

#include "Common/SDKInterfaces/Bluetooth/Services/BluetoothServiceInterface.h"

namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {
namespace services {

/**
 * Interface to support A2DP streaming from SDK to bluetooth device.
 */
class A2DPSinkInterface : public BluetoothServiceInterface {
public:
    /// The Service UUID.
    static constexpr const char* UUID = "0000110b-0000-1000-8000-00805f9b34fb";

    /// The Service Name.
    static constexpr const char* NAME = "AudioSink";
};

}  // namespace services
}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif  //  DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSINKINTERFACE_H_
