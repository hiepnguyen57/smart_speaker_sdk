#ifndef DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_SDPRECORDINTERFACE_H_
#define DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_SDPRECORDINTERFACE_H_

#include <string>

namespace deviceClientSDK {
namespace common {
namespace sdkInterfaces {
namespace bluetooth {
namespace services {

// Used to implement ServiceDiscoveryProtocol records. THis allows identification of the service.
class SDPRecordInterface
{
public:
    /**
     * The base UUID of a Bluetooth service. All service UUIDs are calculated from this.
     * Services have a short uuid assigned that is either uuid16 or uuid32. For example:
     * 
     * AudioSource (A2DP Source)
     * Short UUID:110a
     * UUID: 0000110a-0000-1000-8000-00805f9b34fb
     */
    static const std::string BASE_UUID() {
        return "00000000-0000-1000-8000-00805f9b34fb";
    }

    // Destructor
    virtual ~SDPRecordInterface() = default;

    // @return The name of the service.
    virtual std::string getName() const = 0;

    // @return the UUID of the service.
    virtual std::string getUuid() const = 0;

    // @return the Version of the service.
    virtual std::string getVersion() const = 0;
};


}  // namespace services
}  // namespace bluetooth
}  // namespace sdkInterfaces
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_SDKINTERFACES_BLUETOOTH_SERVICES_SDPRECORDINTERFACE_H_