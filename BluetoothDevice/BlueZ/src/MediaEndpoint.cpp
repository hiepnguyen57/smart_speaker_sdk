#include <Common/Utils/Logger/Log.h>
#include "BlueZ/MediaEndpoint.h"
#include "BlueZ/BlueZConstants.h"

// https://github.com/Arkq/bluez-alsa
// Version 1.2.0
#include <Common/Utils/bluez-alsa/a2dp-rtp.h>
#include <poll.h>

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

using namespace common::utils::logger;

#define TAG_MEDIAENDPOINT           "MediaEndpoint\t"

// General error for DBus methods.
constexpr const char* DBUS_ERROR_FAILED = "org.bluez.Error.Rejected";

// Timeout to wait for new data coming frome BlueZ audio stream.
static const std::chrono::milliseconds POLL_TIMEOUT_MS(100);

// Name of the BlueZ MediaEndpoint1::SetConfiguration method.
constexpr const char* MEDIAENDPOINT1_SETCONFIGURATION_METHOD_NAME = "SetConfiguration";

// Name of the BlueZ MediaEndpoint1::SelectConfiguration method.
constexpr const char* MEDIAENDPOINT1_SELECTCONFIGURATION_METHOD_NAME = "SelectConfiguration";

// Name of the BlueZ MediaEndpoint1::ClearConfiguration method.
constexpr const char* MEDIAENDPOINT1_CLEARCONFIGURATION_METHOD_NAME = "ClearConfiguration";

// Name of the BlueZ MediaEndpoint1::Release method.
constexpr const char* MEDIAENDPOINT1_RELEASE_METHOD_NAME = "Release";

// selectCompatibleConfig() should check 4 options
constexpr int CHECK_4_OPTIONS = 4;

// @c selectCompatibleConfig() should check 2 options
constexpr int CHECK_2_OPTIONS = 2;

// First frequency mask to be used in @c selectCompatibleConfig().
constexpr uint8_t FIRST_FREQUENCY = 0x10;

// First channel mode mask to be used in @c selectCompatibleConfig().
constexpr uint8_t FIRST_CHANNEL_MODE = 0x01;

// First block length mask to be used in @c selectCompatibleConfig().
constexpr uint8_t FIRST_BLOCK_LENGTH = 0x10;

// First subbands mask to be used in @c selectCompatibleConfig().
constexpr uint8_t FIRST_SUBBANDS = 0x04;

// First allocation method mask to be used in @c selectCompatibleConfig().
constexpr uint8_t FIRST_ALLOCATION_METHOD = 0x01;

// Sampling rate 16000
constexpr int SAMPLING_RATE_16000 = 16000;

// Sampling rate 32000
constexpr int SAMPLING_RATE_32000 = 32000;

// Sampling rate 44100
constexpr int SAMPLING_RATE_44100 = 44100;

// Sampling rate 48000
constexpr int SAMPLING_RATE_48000 = 48000;

// Max sane frame length for SBC codec
constexpr size_t MAX_SANE_FRAME_LENGTH = 200;

// Min sane frame length for SBC codec
constexpr size_t MIN_SANE_FRAME_LENGTH = 1;

// Max sane code size for SBC codec. Max compression ratio for 8-band settings
constexpr size_t MAX_SANE_CODE_SIZE = MAX_SANE_FRAME_LENGTH * 32;

// Min sane code size for SBC codec
constexpr size_t MIN_SANE_CODE_SIZE = 1;

/**
 * XML description of the MediaEndpoint1 interface to be implemented by this obejct. The format is defined by DBus.
 * This data is used during the registration of the media endpoint object.
 */
static const gchar mediaEndpointIntrospectionXml[] =
    "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection "
    "1.0//EN\"\n"
    "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\";>\n"
    "<node>"
    " <interface name=\"org.bluez.MediaEndpoint1\">"
    "  <method name=\"SetConfiguration\">"
    "   <arg name=\"transport\" direction=\"in\" type=\"o\"/>"
    "   <arg name=\"properties\" direction=\"in\" type=\"a{sv}\"/>"
    "  </method>"
    "  <method name=\"SelectConfiguration\">"
    "   <arg name=\"capabilities\" direction=\"in\" type=\"ay\"/>"
    "   <arg name=\"configuration\" direction=\"out\" type=\"ay\"/>"
    "  </method>"
    "  <method name=\"ClearConfiguration\">"
    "  <arg name=\"transport\" direction=\"in\" type=\"o\"/>"
    "  </method>"
    "  <method name=\"Release\">"
    "  </method>"
    " </interface>"
    "</node>";

MediaEndpoint::MediaEndpoint(std::shared_ptr<DBusConnection> connection, const std::string& endpointPath) : 
        DBusObject(
            connection,
            mediaEndpointIntrospectionXml,
            endpointPath,
            {{MEDIAENDPOINT1_SETCONFIGURATION_METHOD_NAME, &MediaEndpoint::onSetConfiguration},
             {MEDIAENDPOINT1_SELECTCONFIGURATION_METHOD_NAME, &MediaEndpoint::onSelectConfiguration},
             {MEDIAENDPOINT1_CLEARCONFIGURATION_METHOD_NAME, &MediaEndpoint::onClearConfiguration},
             {MEDIAENDPOINT1_RELEASE_METHOD_NAME, &MediaEndpoint::onRelease}}),
        m_endpointPath{endpointPath},
        m_operatingModeChanged{false},
        m_operatingMode{OperatingMode::INACTIVE} {
    m_thread = std::thread(&MediaEndpoint::mediaThread, this);
}

MediaEndpoint::~MediaEndpoint() {
    setOperatingMode(OperatingMode::RELEASED);
    if(m_thread.joinable()) {
        m_thread.join();
    }
}

void MediaEndpoint::abortStreaming() {
    setOperatingMode(OperatingMode::INACTIVE);
    std::shared_ptr<DBusProxy> deviceProxy = 
        DBusProxy::create(BlueZConstants::BLUEZ_DEVICE_INTERFACE, m_streamingDevicePath);
    if(deviceProxy) {
        deviceProxy->callMethod("Disconnect");
    }
}

// This code in this method is based on a work of Arkadiusz Bokowy licensed under the terms of the MIT license.
// https://github.com/Arkq/bluez-alsa/blob/88aefeea56b7ea20668796c2c7a8312bf595eef4/src/io.c#L144
void MediaEndpoint::mediaThread() {
   pollfd pollStruct = { /* fd */ 0, /* requested events */ POLLIN, /* return events */ 0};

   std::shared_ptr<MediaContext> mediaContext;

   while(m_operatingMode != OperatingMode::RELEASED) {
       // Reset any media context that could still esist.
        {
            std::unique_lock<std::mutex> modeLock(m_mutex);

            m_modeChangeSignal.wait(modeLock, [this]() {return m_operatingModeChanged;});
            m_operatingModeChanged = false;

            if(m_operatingMode != OperatingMode::SINK) {
                continue;
            }

            if(!m_currentMediaContext || !m_currentMediaContext->isSBCInitialized()) {
                LOG_ERROR << TAG_MEDIAENDPOINT << "mediaThreadFailed; reason: no valid media context, no media streaming started";
                continue;
            }

            mediaContext = m_currentMediaContext;

        }

        pollStruct.fd = mediaContext->getStreamFD();
        m_ioBuffer.resize(static_cast<unsigned long>(mediaContext->getReadMTU()));

        const size_t sbcCodeSize = sbc_get_codesize(mediaContext->getSBCContextPtr());
        const size_t sbcFrameLength = sbc_get_frame_length(mediaContext->getSBCContextPtr());

        if(sbcFrameLength < MIN_SANE_FRAME_LENGTH || sbcFrameLength > MAX_SANE_FRAME_LENGTH) {
            LOG_ERROR << TAG_MEDIAENDPOINT << "mediaThreadFailed; reason: invalid sbcFrameLength";
            abortStreaming();
            continue;
        }

        if(sbcCodeSize < MIN_SANE_CODE_SIZE || sbcCodeSize > MAX_SANE_CODE_SIZE) {
            LOG_ERROR << TAG_MEDIAENDPOINT << "mediaThreadFailed; reason: invalid sbcCodeSize";
            abortStreaming();
            continue;            
        }

        // output buffer size = decoded block size * (number of encoded blocks in the input buffer + 1 to fill possible gap)
        const size_t outBufferSize = sbcCodeSize * (m_ioBuffer.size() / sbcFrameLength + 1);
        m_sbcBuffer.resize(outBufferSize);

        int positionInReadBuf = 0;
        
        // Staying in current mode
        while(m_operatingMode == OperatingMode::SINK) {
            int timeout = poll(&pollStruct, 1, static_cast<int>(POLL_TIMEOUT_MS.count()));

            if(timeout == 0) {
                continue;
            }
            if(timeout < 0) {
                LOG_ERROR << TAG_MEDIAENDPOINT << "mediaThreadFailed; reason: Failed to poll bluetooth media stream";
                abortStreaming();
                break;
            }

            // Check if wew are still in SINK mode
            if(m_operatingMode != OperatingMode::SINK) {
                break;
            }

            ssize_t bytesReadSigned = 
                read(pollStruct.fd, m_ioBuffer.data() + positionInReadBuf, m_ioBuffer.size() - positionInReadBuf);

            if(bytesReadSigned < 0) {
                LOG_ERROR << TAG_MEDIAENDPOINT << "mediaThreadFailed; reason: Failed to read bluetooth media stream";
                abortStreaming();
                break;
            }

            if(bytesReadSigned == 0) {
                // End of stream. switch to inactive mode.
                setOperatingMode(OperatingMode::INACTIVE);
                break;
            }

            size_t bytesRead = static_cast<size_t>(bytesReadSigned);

            if(bytesRead < sizeof(rtp_header)) {
                // Invalid RPT frame. Skip it
                continue;
            }

            // Decode RTP frame and SBC payload.
            rtp_header_t* rtpHeader = reinterpret_cast<rtp_header_t*>(m_ioBuffer.data());
            const rtp_payload_sbc_t* rtpPayload = 
                reinterpret_cast<const rtp_payload_sbc_t*>(&rtpHeader->csrc[rtpHeader->cc]);
            
            const uint8_t* payloadData = reinterpret_cast<const uint8_t*>(rtpPayload + 1);
            uint8_t* output = m_sbcBuffer.data();
            size_t headersSize = reinterpret_cast<size_t>(payloadData) - reinterpret_cast<size_t>(m_ioBuffer.data());
            size_t inputLength = bytesRead - headersSize;
            if (inputLength > m_ioBuffer.size()) {
                // Invalid RTP frame, skip it
                continue;
            }
            size_t outputLength = outBufferSize;
            size_t frameCount = rtpPayload->frame_count;

            while(frameCount-- && inputLength >= sbcFrameLength) {
                ssize_t bytesProcessed = 0;
                size_t bytesDecoded = 0;

                if ((bytesProcessed = sbc_decode(
                         mediaContext->getSBCContextPtr(),
                         payloadData,
                         inputLength,
                         output,
                         outputLength,
                         &bytesDecoded)) < 0) {
                    LOG_ERROR << TAG_MEDIAENDPOINT << "mediaThreadFailed; reason: SBC decoding error";
                    break;
                }

                payloadData += bytesProcessed;
                inputLength -= bytesProcessed;

                output += bytesDecoded;
                outputLength -= bytesDecoded;
            }

            size_t writeSize = output - m_sbcBuffer.data();

            // Check if wea are still in SINK mode
            if(m_operatingMode != OperatingMode::SINK) {
                break;
            }

            m_ioStream->send(m_sbcBuffer.data(), writeSize);
        } // IO loop, continue while still in SINK mode
    }     // while(true) =  thread loop

    mediaContext.reset(); 
}

std::shared_ptr<common::utils::bluetooth::FormattedAudioStreamAdapter> MediaEndpoint::getAudioStream() {
    std::lock_guard<std::mutex> guard(m_streamMutex);

    if(m_ioStream) {
        return m_ioStream;
    }

    m_ioStream = std::make_shared<common::utils::bluetooth::FormattedAudioStreamAdapter>(m_audioFormat);
    return m_ioStream;
}

std::string MediaEndpoint::getEndpointPath() const {
    return m_endpointPath;
}

std::string MediaEndpoint::getStreamingDevicePath() const {
    return m_streamingDevicePath;
}

void MediaEndpoint::setOperatingMode(OperatingMode mode) {
    std::lock_guard<std::mutex> modeLock(m_mutex);

    m_operatingMode = mode;
    m_operatingModeChanged = true;
    m_modeChangeSignal.notify_all();
}

void MediaEndpoint::onMediaTransportStateChanged(
    common::utils::bluetooth::MediaStreamingState newState,
    const std::string& devicePath) {
    
    if(m_operatingMode == OperatingMode::RELEASED) {
        // Release the media thread already.
        return;
    }

    if(m_streamingDevicePath != devicePath) {
        return;
    }

    if(newState == common::utils::bluetooth::MediaStreamingState::PENDING) {
        // pending: streaming but not acquired.
        std::shared_ptr<DBusProxy> transportProxy = 
            DBusProxy::create(BlueZConstants::BLUEZ_MEDIATRANSPORT_INTERFACE, m_streamingDevicePath);

        if(!transportProxy) {
            LOG_ERROR << TAG_MEDIAENDPOINT << "onMediaTransportStateChangedFailed"
                                            << "reason: Failed to get MediaTransport1 proxy";
            return;
        }

        ManagedGError error;
        GUnixFDList* fdList = nullptr;
        ManagedGVariant transportDetails = 
            transportProxy->callMethodWithFDList("Acquire", nullptr, &fdList, error.toOutputParameter());
        if(error.hasError()) {
            LOG_ERROR << TAG_MEDIAENDPOINT << "onMediaTransportStateChangedFailed; reason: nullFDlist";
            return;
        }

        gint32 streamFDIndex = 0;
        guint16 readMTU = 0;
        guint16 writeMTU = 0;
        g_variant_get(transportDetails.get(), "(hqq)", &streamFDIndex, &readMTU, &writeMTU);

        if (streamFDIndex > g_unix_fd_list_get_length(fdList)) {
            LOG_ERROR << TAG_MEDIAENDPOINT << "onMediaTransportStateChangedFailed; reason: indexOutOfBounds";
            return;
        }

        // g_unix_fd_list_get duplicates the fd
        gint streamFD = g_unix_fd_list_get(fdList, streamFDIndex, error.toOutputParameter());

        if (streamFD < 0) {
            LOG_ERROR << TAG_MEDIAENDPOINT << "onMediaTransportStateChangedFailed; reason: Invalid media stream file descriptor";
            return;
        }

        if (error.hasError()) {
            LOG_ERROR << TAG_MEDIAENDPOINT << "onMediaTransportStateChangedFailed; reason: failedToGetFD " << error.getMessage();
            close(streamFD);
            return;
        }

        {
            std::lock_guard<std::mutex> modeLock(m_mutex);

            if (!m_currentMediaContext) {
                m_currentMediaContext = std::make_shared<MediaContext>();
            }

            m_currentMediaContext->setStreamFD(streamFD);

            m_currentMediaContext->setReadMTU(readMTU);
            m_currentMediaContext->setWriteMTU(writeMTU);
        }

        // Make sure we have stream created
        getAudioStream();

        setOperatingMode(OperatingMode::SINK);
    } else if (common::utils::bluetooth::MediaStreamingState::IDLE == newState) {
        setOperatingMode(OperatingMode::INACTIVE);
    }
}

void MediaEndpoint::onSetConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation) {
    {
        std::lock_guard<std::mutex> modeLock(m_mutex);

        if(!m_currentMediaContext) {
            m_currentMediaContext = std::make_shared<MediaContext>();
        }

        m_currentMediaContext->setSBCInitialized(false);

        const char* path =  nullptr;
        GVariant* configuration = nullptr;
        GVariant* variant = g_dbus_method_invocation_get_parameters(invocation);
        g_variant_get(variant, "(o*)", &path, &configuration);

        GVariantMapReader mapReader(configuration);
        ManagedGVariant sbcConfigVar = mapReader.getVariant("Configuration");
        if(sbcConfigVar.hasValue()) {
            gsize configSize = 0;
            gconstpointer ptr = g_variant_get_fixed_array(sbcConfigVar.get(), &configSize, 1);
            if(ptr) {
                sbc_t* sbcContext = m_currentMediaContext->getSBCContextPtr();
                int sbcError = -sbc_init_a2dp(sbcContext, 0, ptr, configSize);
                if(sbcError != 0) {
                    LOG_ERROR << TAG_MEDIAENDPOINT << "onSetConfigurationFailed; reason: Failed to init SBC decoder";
                    g_dbus_method_invocation_return_dbus_error(
                        invocation, DBUS_ERROR_FAILED, "Failed to init SBC decoder");
                    return;
                } else {
                    m_audioFormat.encoding = common::utils::AudioFormat::Encoding::LPCM;
                    m_audioFormat.endianness = SBC_LE == sbcContext->endian
                                                    ? common::utils::AudioFormat::Endianness::LITTLE
                                                    : common::utils::AudioFormat::Endianness::BIG;
                    m_audioFormat.sampleSizeInBits = 16;
                    m_audioFormat.numChannels = SBC_MODE_MONO == sbcContext->mode ? 1 : 2;
                    switch (sbcContext->frequency) {
                        case SBC_FREQ_16000:
                            m_audioFormat.sampleRateHz = SAMPLING_RATE_16000;
                            break;
                        case SBC_FREQ_32000:
                            m_audioFormat.sampleRateHz = SAMPLING_RATE_32000;
                            break;
                        case SBC_FREQ_44100:
                            m_audioFormat.sampleRateHz = SAMPLING_RATE_44100;
                            break;
                        case SBC_FREQ_48000:
                        default:
                            m_audioFormat.sampleRateHz = SAMPLING_RATE_48000;
                            break;
                    }
                    m_audioFormat.layout =  common::utils::AudioFormat::Layout::INTERLEAVED;
                    m_audioFormat.dataSigned = true;

                    m_currentMediaContext->setSBCInitialized(true);                   
                }
            } else {
                LOG_ERROR << TAG_MEDIAENDPOINT << "onSetConfigurationFailed; reason: Failed to convert sbc configuration to bytestream";
                g_dbus_method_invocation_return_dbus_error(
                    invocation, DBUS_ERROR_FAILED, "Failed to convert sbc configuration to bytestream");
                return;                
            }
        } else {
            LOG_ERROR << TAG_MEDIAENDPOINT << "onSetConfigurationFailed; reason: Failed to read SBC configuration";
            g_dbus_method_invocation_return_dbus_error(
                invocation, DBUS_ERROR_FAILED, "Failed to read SBC configuration");
            return;              
        }

        m_streamingDevicePath = path;     
    }

    g_dbus_method_invocation_return_value(invocation, nullptr);

}

static uint8_t selectCompatibleConfig(uint8_t caps, uint8_t mask, int width) {
    for (; !(caps & mask) && width > 0; mask <<= 1, width--)
        ;
    return mask;
}

void MediaEndpoint::onSelectConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation) {

    GVariantTupleReader capsTuple(g_dbus_method_invocation_get_parameters(invocation));
    ManagedGVariant streamSupportedCapabilitiesBytes = capsTuple.getVariant(0);

    gsize configSize = 0;
    gconstpointer ptr = g_variant_get_fixed_array(streamSupportedCapabilitiesBytes.get(), &configSize, sizeof(uint8_t));
    if (ptr && configSize >= 4) {
        auto capabilitiesBytes = static_cast<const uint8_t*>(ptr);
        uint8_t byte1 = capabilitiesBytes[0];
        uint8_t byte2 = capabilitiesBytes[1];

        // The following logic heavily depends on the correct format of SBC capabilities
        // Select the best possible configuration
        uint8_t selectedConfig1 = selectCompatibleConfig(byte1, FIRST_FREQUENCY, CHECK_4_OPTIONS);
        selectedConfig1 |= selectCompatibleConfig(byte1, FIRST_CHANNEL_MODE, CHECK_4_OPTIONS);

        uint8_t selectedConfig2 = selectCompatibleConfig(byte2, FIRST_BLOCK_LENGTH, CHECK_4_OPTIONS);
        selectedConfig2 |= selectCompatibleConfig(byte2, FIRST_SUBBANDS, CHECK_2_OPTIONS);
        selectedConfig2 |= selectCompatibleConfig(byte2, FIRST_ALLOCATION_METHOD, CHECK_2_OPTIONS);

        GVariantBuilder* capBuilder = g_variant_builder_new(G_VARIANT_TYPE("ay"));

        g_variant_builder_add(capBuilder, "y", selectedConfig1);
        g_variant_builder_add(capBuilder, "y", selectedConfig2);
        g_variant_builder_add(capBuilder, "y", capabilitiesBytes[2]);
        g_variant_builder_add(capBuilder, "y", capabilitiesBytes[3]);

        g_dbus_method_invocation_return_value(invocation, g_variant_new("(ay)", capBuilder));
    } else {
        LOG_ERROR << TAG_MEDIAENDPOINT << "onSelectConfigurationFailed; reason: Invalid SBC config";
    }
}

void MediaEndpoint::onClearConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

std::string MediaEndpoint::operatingModeToString(OperatingMode mode) {
    switch (mode) {
        case OperatingMode::INACTIVE:
            return "INACTIVE";
        case OperatingMode::SINK:
            return "SINK";
        case OperatingMode::SOURCE:
            return "SOURCE";
        case OperatingMode::RELEASED:
            return "RELEASED";
    }
    return "UNKNOWN";
}

void MediaEndpoint::onRelease(GVariant* arguments, GDBusMethodInvocation* invocation) {
    g_dbus_method_invocation_return_value(invocation, nullptr);
}


} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK