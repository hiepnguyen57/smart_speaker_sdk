#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_MACADDRESSSTRING_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_MACADDRESSSTRING_H_

#include <memory>
#include <string>

namespace deviceClientSDK {
namespace common {
namespace utils {
/**
 * A class used to validate a MAC address string before construction.
 */
class MacAddressString {
public:
    // Default copy constructor so object can be passed by value.
    MacAddressString(const MacAddressString&) = default;

    // Factory that validates the MAC address before constructing the actual object.
    static std::unique_ptr<MacAddressString> create(const std::string& macAddress);

    std::string getString() const;

private:
    // Constructor.
    MacAddressString(const std::string& macAddress);

    const std::string m_macAddress;
};

}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_MACADDRESSSTRING_H_