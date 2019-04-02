#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_A2DPROLE_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_A2DPROLE_H_

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace bluetooth {
// An Enum representing the current A2DP role.
enum class A2DPRole {
    // Device acting as an A2DPSink.
    SINK,
    // Device acting as an A2DPSource.
    SOURCE
};

}  // namespace bluetooth
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_BLUETOOTH_A2DPROLE_H_