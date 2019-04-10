#ifndef DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_MEDIAENDPOINT_H_
#define DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_MEDIAENDPOINT_H_

#include <atomic>
#include <condition_variable>
#include <vector>

#include <Common/Utils/AudioInputStream.h>
#include <Common/SDKInterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <Common/Utils/Bluetooth/FormattedAudioStreamAdapter.h>

#include "BlueZ/BlueZDeviceManager.h"
#include "BlueZ/BlueZUtils.h"
#include "BlueZ/MediaContext.h"

#include <gio/gio.h>
#include <sbc/sbc.h>

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

class MediaEndpoint : public DBusObject<MediaEndpoint> {
public:
    // Constructor.
    MediaEndpoint(std::shared_ptr<DBusConnection> connection, const std::string& endpointPath);

    // Destructor.
    ~MediaEndpoint();

    // A callback called by BlueZ to notify MediaEndpoint of the stream codec configuration selected by both sides.
    void onSetConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation);

    // A callback called by BlueZ to asking MediaEndpoint to select the audio code configuration to use for the audio streaming.
    void onSelectConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation);

    // A callback called by BlueZ to notify MediaEndpoint that it should reset the codec configuration.
    void onClearConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation);

    // A callback called by BlueZ to notify MediaEndpoint that it is being release and will not be used anymore.
    void onRelease(GVariant* arguments, GDBusMethodInvocation* invocation);

    // A callback called by @c DeviceManager to notify MediaEndpoint that BlueZ reported a streaming state change.
    void onMediaTransportStateChanged(
        common::utils::bluetooth::MediaStreamingState newState,
        const std::string& devicePath);

    /**
     * Get DBus object path of the media endpoint
     *
     * @return Object path of the media endpoint
     */
    std::string getEndpointPath() const;

    /**
     * Get DBus object path of the device the BlueZ currently uses this media endpoint with for streaming.
     *
     * @return Device object path.
     */
    std::string getStreamingDevicePath() const;

    /**
     * Get the @c FormattedAudioStreamAdapter for the audio stream being received from the remote bluetooth device over
     * A2DP. It is safe to call this method early.
     *
     * @return An @c FormattedAudioStreamAdapter object used by @c MediaEndpoint.
     */
    std::shared_ptr<common::utils::bluetooth::FormattedAudioStreamAdapter> getAudioStream();

private:   
    /**
     * Operating mode of the @c MediaEndpoint and its media stream
     */
    enum class OperatingMode {
        /**
         * There is no streaming currently active. Media streaming thread is waiting for the new mode.
         */
        INACTIVE,

        /**
         * The @c MediaEnpoint is working in SINK mode, receiving audio stream from the remote bluetooth device. Media
         * streaming thread is reading the data from the file descriptor provided by BlueZ.
         */
        SINK,

        /**
         * Reserved for future use.
         */
        SOURCE,

        /**
         * The @c MediaEndpoint has been released and any operation on it should fail. Media streaming thread will
         * stop processing the data and exit as soon as returns from any active blocking operation such as poll() or
         * read().
         */
        RELEASED
    };

    // Returns a string representation of @c OperatingMode enum class
    std::string operatingModeToString(OperatingMode mode);

    // A media streaming thread main function.
    void mediaThread();

    // Set the current operating mode for the media endpoint.
    void setOperatingMode(OperatingMode mode);

    // Disconnects the device and enters INACTIVE state.
    void abortStreaming();

    /**
     * An object path where media endpoint is/should be registered.
     */
    std::string m_endpointPath;

    /**
     * An object path of the device that is currently being used to stream from using this media endpoint.
     */
    std::string m_streamingDevicePath;

    /**
     * Flag signalling that operating mode has changed
     */
    bool m_operatingModeChanged;

    /**
     * Current @c OperatingMode.
     */
    std::atomic<OperatingMode> m_operatingMode;

    /**
     * Buffer used to decode SBC data to. Contains raw PCM data after the decoding.
     */
    std::vector<uint8_t> m_sbcBuffer;

    /**
     * Mutex synchronizing the state of media thread.
     */
    std::mutex m_mutex;

    /**
     * Mutex synchronizing the creation/querying of the audio @c FormattedAudioStreamAdapter object.
     */
    std::mutex m_streamMutex;

    /**
     * A condition variable to listen for operating state changes.
     */
    std::condition_variable m_modeChangeSignal;

    /**
     * @c FormattedAudioStreamAdapter object exposed to the clients and used to send decoded audio data to.
     */
    std::shared_ptr<common::utils::bluetooth::FormattedAudioStreamAdapter> m_ioStream;

    /**
     * Buffer for receiving encoded data from BlueZ. This buffer contains RTP packets with SBC packets payload.
     */
    std::vector<uint8_t> m_ioBuffer;

    /**
     * The @c AudioFormat associated with the stream.
     */
    common::utils::AudioFormat m_audioFormat;

    /**
     * A @c MediaContext instance used to hold the streaming configuration before the actual stream starts. As soon
     * as stream starts this configuration is cleared and will be created again, when streaming configuration changes.
     */
    std::shared_ptr<MediaContext> m_currentMediaContext;

    /*
     * Dedicated thread for I/O.
     */
    std::thread m_thread;
};

} // namespace blueZ
} // namespace bluetoothDevice
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_MEDIAENDPOINT_H_