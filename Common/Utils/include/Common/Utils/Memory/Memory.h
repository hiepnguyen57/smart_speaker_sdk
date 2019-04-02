#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_MEMORY_MEMORY_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_MEMORY_MEMORY_H_

#include <memory>
#include <utility>

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace memory {

// START Herb Sutter code sample adaptation
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
// END Herb Sutter code sample adaptation

}  // namespace memory
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_MEMORY_MEMORY_H_