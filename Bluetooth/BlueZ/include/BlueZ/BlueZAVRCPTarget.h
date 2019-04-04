#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZAVRCPTARGET_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZAVRCPTARGET_H_

#include <memory>
#include <mutex>

#include <Common/SDKInterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include <Common/Utils/Bluetooth/SDPRecords.h>
#include "BlueZ/BlueZBluetoothDevice.h"
#include "BlueZ/BlueZUtils.h"

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

class BlueZAVRCPTarget : public common::sdkInterfaces::bluetooth::services::AVRCPTargetInterface {
public:
    /**
     * Creates a BlueZAVRCPTarget instance.
     *
     * @param device A pointer to an instance of a @c DBusProxy for an org.bluez.MediaControl1 interface.
     * @return An instance of AVRCPTarget if successful, else a nullptr.
     */
    static std::shared_ptr<BlueZAVRCPTarget> create(std::shared_ptr<DBusProxy> mediaControlProxy);

    /// @name BluetoothServiceInterface methods
    /// @{
    std::shared_ptr<common::sdkInterfaces::bluetooth::services::SDPRecordInterface> getRecord() override;
    void setup() override;
    void cleanup() override;
    /// @}

    /// @name AVRCPTargetInterface methods
    /// @{
    bool play() override;
    bool pause() override;
    bool next() override;
    bool previous() override;
    /// @}

private:
    /// Constructor.
    BlueZAVRCPTarget(std::shared_ptr<DBusProxy> mediaControlProxy);

    /// The @c SDPRecord associated with this Service. We don't currently parse the version.
    std::shared_ptr<common::utils::bluetooth::AVRCPTargetRecord> m_record;

    /// Serialize commands.
    std::mutex m_cmdMutex;

    /// A proxy for the BlueZ MediaControl interface.
    std::shared_ptr<DBusProxy> m_mediaControlProxy;
};

} // namespace blueZ
} // namespce bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_BLUEZAVRCPTARGET_H_
