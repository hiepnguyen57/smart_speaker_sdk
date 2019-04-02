#include <Common/Utils/MacAddressString.h>

#include <memory>
#include <string>

namespace deviceClientSDK {
namespace common {
namespace utils {

std::unique_ptr<MacAddressString> MacAddressString::create(const std::string& macAddress) {
    enum class State { FIRST_OCTET_BYTE, SECOND_OCTET_BYTE, DIVIDER };

    State parseState = State::FIRST_OCTET_BYTE;
    int numOctets = 0;
    for(const auto& c : macAddress) {
        switch (parseState)
        {
            case State::FIRST_OCTET_BYTE:
                if(!std::isxdigit(c)) {
                    return nullptr;
                }
                parseState = State::SECOND_OCTET_BYTE;
                break;
            case State::SECOND_OCTET_BYTE:
                if(!std::isxdigit(c)) {
                    return nullptr;
                }
                parseState = State::DIVIDER;
                ++numOctets;
                break;
            case State::DIVIDER:
                if ((':' != c) && ('-' != c)) {
                    return nullptr;
                }
                parseState = State::FIRST_OCTET_BYTE;
                break;                           
        }
    }

    if ((6 != numOctets) || (parseState != State::DIVIDER)) {
        return nullptr;
    }

    return std::unique_ptr<MacAddressString>(new MacAddressString(macAddress));   
}

std::string MacAddressString::getString() const {
    return m_macAddress;
}

MacAddressString::MacAddressString(const std::string& macAddress) : m_macAddress(macAddress) {
}

}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK