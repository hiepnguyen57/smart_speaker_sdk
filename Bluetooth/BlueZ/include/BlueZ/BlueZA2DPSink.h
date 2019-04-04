#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZA2DPSINK_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZA2DPSINK_H_

#include <memory>

#include <Common/SDKInterfaces/Bluetooth/Services/A2DPSinkInterface.h>
#include <Common/Utils/Bluetooth/BluetoothEventBus.h>
#include <Common/Utils/Bluetooth/SDPRecords.h>

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

/**
 * BlueZ implementation of @c A2DPSinkInterface interface
 */
class BlueZA2DPSink : public common::sdkInterfaces::bluetooth::services::A2DPSinkInterface {
public:
    // factory method to create new instace of @c BlueZA2DPSink
    static std::shared_ptr<BlueZA2DPSink> create();

    // name BluetoothServiceInterface functions.
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::SDPRecordInterface> getRecord() override;
    void setup() override;
    void cleanup() override;

private:
    // Private constructor.
    BlueZA2DPSink();

    // Bluetooth service's SDP record containing the common service information.
    std::shared_ptr<common::utils::bluetooth::A2DPSinkRecord> m_record;
};

} // namespace blueZ
} // namespce bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZA2DPSINK_H_