#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_BUFFERLAYOUT_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_BUFFERLAYOUT_H_

#include <cstdint>
#include <cstddef>
#include <mutex>
#include <string>
#include <vector>


#include "Common/Utils/Logger/Log.h"
#include "SharedDataStream.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace sds {

using namespace logger;

template <typename T>
class SharedDataStream<T>::BufferLayout {
public:
    // Magic number used to identify a valid Header in memory.
    static const uint32_t MAGIC_NUMBER = 0x53445348;

    // Version of this header layout.
    static const uint32_t VERSION = 2;

    // A constructor.
    BufferLayout(std::shared_ptr<Buffer> buffer);

    // A Destructor.
    ~BufferLayout();

    struct Header {
        uint32_t magic;
        uint8_t version;
        uint32_t traitsNameHash;
        uint16_t wordSize;
        uint8_t maxReaders;
        ConditionVariable dataAvailableConditionVariable;
        Mutex dataAvailableMutex;
        ConditionVariable spaceAvailableConditionVariable;
        Mutex backwardSeekMutex;
        AtomicBool isWriterEnabled;
        AtomicBool hasWriterBeenClosed;
        Mutex writerEnableMutex;
        AtomicIndex writeStartCursor;
        AtomicIndex writeEndCursor;
        AtomicIndex oldestUnconsumedCursor;
        uint32_t referenceCount;
        Mutex attachMutex;
        Mutex readerEnableMutex;      
    };

    Header* getHeader() const;
    AtomicBool* getReaderEnabledArray() const;
    AtomicIndex* getReaderCursorArray() const;
    AtomicIndex* getReaderCloseIndexArray() const;
    Index getDataSize() const;
    uint8_t* getData(Index at = 0) const;
    bool init(size_t wordSize, size_t maxReaders);
    bool attach();
    void detach();
    bool isReaderEnabled(size_t id) const;
    void enableReaderLocked(size_t id);
    void disableReaderLocked(size_t id);
    Index wordsUntilWrap(Index after) const;
    static size_t calculateDataOffset(size_t wordSize, size_t maxReaders);
    void updateOldestUnconsumedCursor();
    void updateOldestUnconsumedCursorLocked();

private:
    static uint32_t stableHash(const char* string);
    static size_t alignSizeTo(size_t size, size_t align);
    static size_t calculateReaderEnabledArrayOffset();
    static size_t calculateReaderCursorArrayOffset(size_t maxReaders);
    static size_t calculateReaderCloseIndexArrayOffset(size_t maxReaders);
    void calculateAndCacheConstants(size_t wordSize, size_t maxReaders);
    static const std::string TAG;
    bool isAttached() const;
    std::shared_ptr<Buffer> m_buffer;
    AtomicBool* m_readerEnabledArray;
    AtomicIndex* m_readerCursorArray;
    AtomicIndex* m_readerCloseIndexArray;
    Index m_dataSize;
    uint8_t* m_data;
};

template <typename T>
SharedDataStream<T>::BufferLayout::BufferLayout(std::shared_ptr<Buffer> buffer) : 
        m_buffer{buffer},
        m_readerEnabledArray{nullptr},
        m_readerCursorArray{nullptr},
        m_readerCloseIndexArray{nullptr},
        m_dataSize{0},
        m_data{nullptr} {
}

template <typename T>
SharedDataStream<T>::BufferLayout::~BufferLayout() {
    detach();
}

template <typename T>
typename SharedDataStream<T>::BufferLayout::Header* SharedDataStream<T>::BufferLayout::getHeader() const {
    return reinterpret_cast<Header*>(m_buffer->data());
}

template <typename T>
typename SharedDataStream<T>::AtomicBool* SharedDataStream<T>::BufferLayout::getReaderEnabledArray() const {
    return m_readerEnabledArray;
}

template <typename T>
typename SharedDataStream<T>::AtomicIndex* SharedDataStream<T>::BufferLayout::getReaderCursorArray() const {
    return m_readerCursorArray;
}

template <typename T>
typename SharedDataStream<T>::AtomicIndex* SharedDataStream<T>::BufferLayout::getReaderCloseIndexArray() const {
    return m_readerCloseIndexArray;
}

template <typename T>
typename SharedDataStream<T>::Index SharedDataStream<T>::BufferLayout::getDataSize() const {
    return m_dataSize;
}

template <typename T>
uint8_t* SharedDataStream<T>::BufferLayout::getData(Index at) const {
    return m_data + (at % getDataSize()) * getHeader()->wordSize;
}

template <typename T>
bool SharedDataStream<T>::BufferLayout::init(size_t wordSize, size_t maxReaders) {
    // Make sure parameters are not too large to store.
    if (wordSize > std::numeric_limits<decltype(Header::wordSize)>::max()) {
        LOG_ERROR << "BufferLayout, initFailed, wordSize wrong";
        return false;
    }
    if (maxReaders > std::numeric_limits<decltype(Header::maxReaders)>::max()) {
        LOG_ERROR << "BufferLayout, initFailed, maxReaders";
        return false;
    }

    // Pre-calculate some pointers and sizes that are frequently accessed.
    calculateAndCacheConstants(wordSize, maxReaders);

    // Default construction of the Header.
    auto header = new (getHeader()) Header;

    // Default construction of the reader arrays.
    size_t id;
    for (id = 0; id < maxReaders; ++id) {
        new (m_readerEnabledArray + id) AtomicBool;
        new (m_readerCursorArray + id) AtomicIndex;
        new (m_readerCloseIndexArray + id) AtomicIndex;
    }

    // Header field initialization.
    header->magic = MAGIC_NUMBER;
    header->version = VERSION;
    header->traitsNameHash = stableHash(T::traitsName);
    header->wordSize = wordSize;
    header->maxReaders = maxReaders;
    header->isWriterEnabled = false;
    header->hasWriterBeenClosed = false;
    header->writeStartCursor = 0;
    header->writeEndCursor = 0;
    header->oldestUnconsumedCursor = 0;
    header->referenceCount = 1;

    // Reader arrays initialization.
    for (id = 0; id < maxReaders; ++id) {
        m_readerEnabledArray[id] = false;
        m_readerCursorArray[id] = 0;
        m_readerCloseIndexArray[id] = 0;
    }

    return true;
}

template <typename T>
bool SharedDataStream<T>::BufferLayout::attach() {
    // Verify compatibility.
    auto header = getHeader();
    if (header->magic != MAGIC_NUMBER) {
        LOG_ERROR <<  "attachFailed, reason: magicnumber";
        return false;
    }
    if (header->version != VERSION) {
        LOG_ERROR <<  "attachFailed, reason: incompatibleVersion";
        return false;
    }
    if (header->traitsNameHash != stableHash(T::traitsName)) {
        LOG_ERROR <<  "attachFailed, reason: traitsNameHashMismatch";
        return false;
    }

    // Attach.
    std::lock_guard<Mutex> lock(header->attachMutex);
    if (0 == header->referenceCount) {
        LOG_ERROR <<  "attachFailed, reason: zeroUsers";
        return false;
    }
    if (std::numeric_limits<decltype(header->referenceCount)>::max() == header->referenceCount) {
        LOG_ERROR <<  "attachFailed, reason: bufferMaxUsersExceeded";
        return false;
    }
    ++header->referenceCount;

    // Pre-calculate some pointers and sizes that are frequently accessed.
    calculateAndCacheConstants(header->wordSize, header->maxReaders);

    return true;
}

template <typename T>
void SharedDataStream<T>::BufferLayout::detach() {
    if (!isAttached()) {
        return;
    }

    auto header = getHeader();
    std::lock_guard<Mutex> lock(header->attachMutex);
    --header->referenceCount;
    if (header->referenceCount > 0) {
        return;
    }

    // Destruction of reader arrays.
    for (size_t id = 0; id < header->maxReaders; ++id) {
        m_readerCloseIndexArray[id].~AtomicIndex();
        m_readerCursorArray[id].~AtomicIndex();
        m_readerEnabledArray[id].~AtomicBool();
    }

    // Destruction of the Header.
    header->~Header();
}

template <typename T>
bool SharedDataStream<T>::BufferLayout::isReaderEnabled(size_t id) const {
    return m_readerEnabledArray[id];
}

template <typename T>
void SharedDataStream<T>::BufferLayout::enableReaderLocked(size_t id) {
    m_readerEnabledArray[id] = true;
}

template <typename T>
void SharedDataStream<T>::BufferLayout::disableReaderLocked(size_t id) {
    m_readerEnabledArray[id] = false;
}

template <typename T>
typename SharedDataStream<T>::Index SharedDataStream<T>::BufferLayout::wordsUntilWrap(Index after) const {
    return alignSizeTo(after, getDataSize()) - after;
}

template <typename T>
size_t SharedDataStream<T>::BufferLayout::calculateDataOffset(size_t wordSize, size_t maxReaders) {
    return alignSizeTo(calculateReaderCloseIndexArrayOffset(maxReaders) + (maxReaders * sizeof(AtomicIndex)), wordSize);
}

template <typename T>
void SharedDataStream<T>::BufferLayout::updateOldestUnconsumedCursor() {
    // Note: as an optimization, we could skip this function if Writer policy is nonblockable (ACSDK-251).
    std::lock_guard<Mutex> backwardSeekLock(getHeader()->backwardSeekMutex);
    updateOldestUnconsumedCursorLocked();
}

template <typename T>
void SharedDataStream<T>::BufferLayout::updateOldestUnconsumedCursorLocked() {
    auto header = getHeader();

    // Note: as an optimization, we could skip this function if Writer policy is nonblockable (ACSDK-251).

    // The only barrier to a blocking writer overrunning a reader is oldestUnconsumedCursor, so we have to be careful
    // not to ever move it ahead of any readers.  The loop below searches through the readers to find the oldest point,
    // without moving oldestUnconsumedCursor.  Note that readers can continue to read while we are looping; it means
    // oldest may not be completely accurate, but it will always be older than the readers because they are reading
    // away from it.  Also note that backwards seeks (which would break the invariant) are prevented with a mutex which
    // is held while this function is called.  Also note that all read cursors may be in the future, so we start with
    // an unlimited barrier and work back from there.
    Index oldest = std::numeric_limits<Index>::max();
    for (size_t id = 0; id < header->maxReaders; ++id) {
        // Note that this code is calling isReaderEnabled() without holding readerEnableMutex.  On the surface, this
        // appears to be a race condition because a reader may be disabled and/or re-enabled before the subsequent code
        // reads the cursor, but it turns out to be safe because:
        // - if a reader is enabled, its cursor is valid
        // - if a reader becomes disabled, its cursor moves to writeCursor (which will never be the oldest)
        // - if a reader becomes re-enabled, its cursor defaults to writeCursor (which will never be the oldest)
        // - if a reader is created that wants to be at an older index, it gets there by doing a backward seek (which
        //   is locked when this function is called)
        if (isReaderEnabled(id) && getReaderCursorArray()[id] < oldest) {
            oldest = getReaderCursorArray()[id];
        }
    }

    // If no barrier was found, block at the write cursor so that we retain data until a reader comes along to read it.
    if (std::numeric_limits<Index>::max() == oldest) {
        oldest = header->writeStartCursor;
    }

    // Now that we've measured the oldest cursor, we can safely update oldestUnconsumedCursor with no risk of an
    // overrun of any readers.

    // To clarify the logic here, the code above reviewed all of the enabled readers to see where the oldest cursor is
    // at.  This value is captured in the 'oldest' variable.  Now we want to move up our writer barrier
    // ('oldestUnconsumedCursor') if it is older than it needs to be.
    if (oldest > header->oldestUnconsumedCursor) {
        header->oldestUnconsumedCursor = oldest;

        // Notify the writer(s).
        // Note: as an optimization, we could skip this if there are no blocking writers (ACSDK-251).
        header->spaceAvailableConditionVariable.notify_all();
    }
}

template <typename T>
uint32_t SharedDataStream<T>::BufferLayout::stableHash(const char* string) {
    // Simple, stable hash which XORs all bytes of string into the hash value.
    uint32_t hashed = 0;
    size_t pos = 0;
    while (*string) {
        hashed ^= *string << ((pos % sizeof(uint32_t)) * 8);
        ++string;
        ++pos;
    }
    return hashed;
}

template <typename T>
size_t SharedDataStream<T>::BufferLayout::alignSizeTo(size_t size, size_t align) {
    if (size) {
        return (((size - 1) / align) + 1) * align;
    } else {
        return 0;
    }
}

template <typename T>
size_t SharedDataStream<T>::BufferLayout::calculateReaderEnabledArrayOffset() {
    return alignSizeTo(sizeof(Header), alignof(AtomicBool));
}

template <typename T>
size_t SharedDataStream<T>::BufferLayout::calculateReaderCursorArrayOffset(size_t maxReaders) {
    return alignSizeTo(calculateReaderEnabledArrayOffset() + (maxReaders * sizeof(AtomicBool)), alignof(AtomicIndex));
}

template <typename T>
size_t SharedDataStream<T>::BufferLayout::calculateReaderCloseIndexArrayOffset(size_t maxReaders) {
    return calculateReaderCursorArrayOffset(maxReaders) + (maxReaders * sizeof(AtomicIndex));
}

template <typename T>
void SharedDataStream<T>::BufferLayout::calculateAndCacheConstants(size_t wordSize, size_t maxReaders) {
    auto buffer = reinterpret_cast<uint8_t*>(m_buffer->data());
    m_readerEnabledArray = reinterpret_cast<AtomicBool*>(buffer + calculateReaderEnabledArrayOffset());
    m_readerCursorArray = reinterpret_cast<AtomicIndex*>(buffer + calculateReaderCursorArrayOffset(maxReaders));
    m_readerCloseIndexArray = reinterpret_cast<AtomicIndex*>(buffer + calculateReaderCloseIndexArrayOffset(maxReaders));
    m_dataSize = (m_buffer->size() - calculateDataOffset(wordSize, maxReaders)) / wordSize;
    m_data = buffer + calculateDataOffset(wordSize, maxReaders);
}

template <typename T>
bool SharedDataStream<T>::BufferLayout::isAttached() const {
    return m_data != nullptr;
}
} // namespace sds
} // namespace utils
} // namespace common
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_SDS_BUFFERLAYOUT_H_