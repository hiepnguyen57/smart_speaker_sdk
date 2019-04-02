#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZA2DPSOURCE_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZA2DPSOURCE_H_

#include <Common/SDKInterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <Common/Utils/Bluetooth/SDPRecords.h>
#include <Common/Utils/Bluetooth/FormattedAudioStreamAdapter.h>

namespace deviveClientSDK {
namespace bluetooth {
namespace blueZ {

class BlueZDeviceManager;

/**
 * BlueZ implementation of @c A2DPSourceInterface interface
 */
class BlueZA2DPSource : public common::sdkInterfaces::bluetooth::services::A2DPSourceInterface {
public:
    /**
     * Factory method to create a new instance of @c BlueZA2DPSource
     * @param deviceManager A @c BlueZDeviceManager this instance belongs to
     * @return A new instance of @c BlueZA2DPSource, nullptr if there was an error creating it.
     */
    static std::shared_ptr<BlueZA2DPSource> create(std::shared_ptr<BlueZDeviceManager> deviceManager);

    /// @name A2DPSourceInterface functions.
    /// @{

    std::shared_ptr<common::utils::bluetooth::FormattedAudioStreamAdapter> getSourceStream() override;

    /// @}

    /// @name BluetoothServiceInterface functions.
    /// @{

    std::shared_ptr<common::sdkInterfaces::bluetooth::services::SDPRecordInterface> getRecord() override;
    void setup() override;
    void cleanup() override;

    /// @}

private:
    /**
     * Private constructor
     * @param deviceManager A @c BlueZDeviceManager this instance belongs to
     */
    BlueZA2DPSource(std::shared_ptr<BlueZDeviceManager> deviceManager);

    /**
     * Bluetooth service's SDP record containing the common service information.
     */
    std::shared_ptr<common::utils::bluetooth::A2DPSourceRecord> m_record;

    /**
     * A @c BlueZDeviceManager this instance belongs to
     */
    std::shared_ptr<BlueZDeviceManager> m_deviceManager;
};

} // namespace blueZ
} // namespce bluetooth
} // namespace deviveClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZA2DPSOURCE_H_