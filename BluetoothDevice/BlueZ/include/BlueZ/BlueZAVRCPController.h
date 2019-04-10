#ifndef DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZAVRCPCONTROLLER_H_
#define DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZAVRCPCONTROLLER_H_

#include <memory>

#include <Common/SDKInterfaces/Bluetooth/Services/AVRCPControllerInterface.h>
#include <Common/Utils/Bluetooth/SDPRecords.h>

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

/**
 * BlueZ implementation of @c AVRCPControllerInterface interface
 */
class BlueZAVRCPController : public common::sdkInterfaces::bluetooth::services::AVRCPControllerInterface {
public:
    /**
     * Factory method to create a new instance of @c BlueZAVRCPController.
     *
     * @return A new instance of @c BlueZAVRCPController, nullptr if there was an error creating it.
     */
    static std::shared_ptr<BlueZAVRCPController> create();

    /// @name BluetoothServiceInterface functions.
    /// @{
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::SDPRecordInterface> getRecord() override;
    void setup() override;
    void cleanup() override;
    /// @}

private:
    /**
     * Private constructor
     */
    BlueZAVRCPController();

    /// Bluetooth service's SDP record containing the common service information.
    std::shared_ptr<common::utils::bluetooth::AVRCPControllerRecord> m_record;
};

} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_BLUEZAVRCPCONTROLLER_H_