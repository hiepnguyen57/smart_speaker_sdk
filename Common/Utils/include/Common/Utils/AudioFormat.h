#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_AUDIOFORMAT_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_AUDIOFORMAT_H_

#include <ostream>

namespace deviceClientSDK {
namespace common {
namespace utils {

/**
 * The format of audio data
 */
struct AudioFormat
{
    /**
     * An enum used to represent the encoding of audio data.
     */
    enum class Encoding {
        // Represents LPCM encoding.
        LPCM,
        // Represents OPUS encoding.
        OPUS
    };

    /**
     * An enum class to represent layout of audio files for streams with more than one channel.
     */
    enum class Layout {
        // None-Interleaved: The L and R are separated in different streams.
        NON_INTERLEAVED,
        // Interleaved: The L and R sides of a stereo recording are interleaved.
        INTERLEAVED
    };

    /**
     * An enum class used to represent the endianness of audio data.
     */
    enum class Endianness {
        // Represent little endianness.
        LITTLE,
        // Represent big endianness.
        BIG
    };

    // The encoding of the data.
    Encoding encoding;

    // The endianness of the data.
    Endianness endianness;

    // The number of sample recorded or played per second.
    unsigned int sampleRateHz;

    // The bits per sample.
    unsigned int sampleSizeInBits;

    // The number of channels.
    unsigned int numChannels;

    // @c true if the data is signed @c false otherwise.
    bool dataSigned;

    // The layout of format for case where numberChannesl > 1.
    Layout layout;
};

/**
 * Write an @c Encoding value to an @c ostream as a string.
 * 
 * @param stream The stream write the value to.
 * @param encoding The encoding value to write to the @c ostream as a string.
 * return The @c ostream that was passed in and written to.
 */
inline std::ostream& operator<<(std::ostream& stream, const AudioFormat::Encoding& encoding) {
    switch (encoding)
    {
        case AudioFormat::Encoding::LPCM:
            stream << "LPCM";
            break;
        case AudioFormat::Encoding::OPUS:
            stream << "OPUS";
            break;
    }
    return stream;
}

/**
 * Write an @c Endianness value to an @c ostream as a string.
 *
 * @param stream The stream to write the value to.
 * @param endianness The endianness value to write to the @c ostream as a string.
 * @return The @c ostream that was passed in and written to.
 */
inline std::ostream& operator<<(std::ostream& stream, const AudioFormat::Endianness& endianness) {
    switch (endianness) {
        case AudioFormat::Endianness::LITTLE:
            stream << "LITTLE";
            break;
        case AudioFormat::Endianness::BIG:
            stream << "BIG";
            break;
    }
    return stream;
}


}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_AUDIOFORMAT_H_