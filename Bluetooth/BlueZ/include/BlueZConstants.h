namespace bluetooth {
namespace blueZ {

class BlueZConstants {
public:
    /// Name of the BlueZ service in DBus
    static constexpr auto BLUEZ_SERVICE_NAME = "org.bluez";

    ///Name of the Bluez interface responsible fo media stream delivery
    static constexpr auto BLUEZ_MEDIATRANSPORT_INTERFACE = "org.bluez.MediaTransport1";
}
}   //namespace blueZ
}   //namespce bluetooth