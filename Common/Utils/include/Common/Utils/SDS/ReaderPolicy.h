#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_READERPOLICY_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_READERPOLICY_H_

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace sds {
/// Specifies the policy to use for reading from the stream.
enum class ReaderPolicy {
    /**
     * A @c NONBLOCKING @c Reader will return any available data (up to the amount requested) immediately, without
     * waiting for more data to be written to the stream.  If no data is available, a @c NONBLOCKING @c Reader will
     * return @c Error::WOULDBLOCK.
     */
    NONBLOCKING,
    /**
     * A @c BLOCKING @c Reader will wait for up to the specified timeout (or forever if `(timeout == 0)`) for data
     * to become available.  As soon as at least one word is available, the @c Reader will return up to the
     * requested amount of data.  If no data becomes available in the specified timeout, a @c BLOCKING @c Reader
     * will return @c Error::TIMEDOUT.
     */
    BLOCKING
};

} // namespace sds
} // namespace utils
} // namespace common
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_READERPOLICY_H_
