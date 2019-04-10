#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_WRITERPOLICY_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_WRITERPOLICY_H_

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace sds {
/// Specifies the policy to use for writing to the stream.
enum class WriterPolicy {
    /**
     * A @c NONBLOCKABLE @c Writer will always write all the data provided without waiting for @c Readers to move
     * out of the way.
     *
     * @note: This policy causes the @c Writer to notify @c BLOCKING @c Readers about new data being available
     *     without holding a mutex.  This means that a @c read() call may miss a notification and block when data
     *     is actually available.  The assumption here is that a @c NONBLOCKABLE @c Writer will be frequently
     *     writing data, and a subsequent @c write() will again notify the @c Reader and wake it up.
     */
    NONBLOCKABLE,
    /**
     * An @c ALL_OR_NOTHING @c Writer will either write all the data provided if it can do so without overwriting
     * unconsumed data, or it will return @c Error::WOULDBLOCK without writing any data at all.
     *
     * @note: If there are no @c Readers attached, data can be written to a @c ShardDataStream until it fills up,
     *     but it will then stop accepting data from an @c ALL_OR_NOTHING @c Writer until a @c Reader attaches and
     *     consumes some of the data.  However, there is a corner case where a @c Reader is attached, but has
     *     @c seek()ed into the future.  In this scenario, all data written until the @c Writer catches up with the
     *     @c Reader will never be consumed, so the @c SharedDataStream will allow an @c ALL_OR_NOTHING @c Writer to
     *     continue writing new data (and discarding old data) until it reaches the index the @c Reader is waiting
     *     for.
     */
    ALL_OR_NOTHING,
    /**
     * A @c BLOCKING @c Writer will wait for up to the specified timeout (or forever if `(timeout == 0)`) for space
     * to become available.  As soon as at least one word can be written, the @c Writer will write as many words as
     * it can without overwriting unconsumed data, and return the number of words written.  If no space becomes
     * available in the specified timeout, a @c BLOCKING @c Writer will return @c Error::TIMEDOUT.
     */
    BLOCKING
};

} // namespace sds
} // namespace utils
} // namespace common
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_WRITERPOLICY_H_
