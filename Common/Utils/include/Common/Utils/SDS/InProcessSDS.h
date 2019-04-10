#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_INPROCESSSDS_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_INPROCESSSDS_H_

#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>

#include "SharedDataStream.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace sds {

/// Structure for specifying the traits of a SharedDataStream which works between threads in a single process.
struct InProcessSDSTraits {
    /// C++11 std::atomic is sufficient for in-process atomic variables.
    using AtomicIndex = std::atomic<uint64_t>;

    /// C++11 std::atomic is sufficient for in-process atomic variables.
    using AtomicBool = std::atomic<bool>;

    /// A std::vector provides a simple container to hold a buffer for in-process usage.
    using Buffer = std::vector<uint8_t>;

    /// A std::mutex provides a lock which will work for in-process usage.
    using Mutex = std::mutex;

    /// A std::condition_variable provides a condition variable which will work for in-process usage.
    using ConditionVariable = std::condition_variable;

    /// A unique identifier representing this combination of traits.
    static constexpr const char* traitsName = "deviceClientSDK::common::utils::sds::InProcessSDSTraits";
};

/// Type alias for a SharedDataStream which works between threads in a single process.
using InProcessSDS = SharedDataStream<InProcessSDSTraits>;

} // namespace sds
} // namespace utils
} // namespace common
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_INPROCESSSDS_H_
