#ifndef DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_PULSEAUDIOBLUETOOTHINITIALIZER_H_
#define DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_PULSEAUDIOBLUETOOTHINITIALIZER_H_

#include <condition_variable>
#include <memory>
#include <mutex>

#include <Common/Utils/Bluetooth/BluetoothEventBus.h>
#include <Common/Utils/Threading/Executor.h>

#include <pulse/pulseaudio.h>

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

/**
 * Application can receive/send A2DP audio data from/to BlueZ by registering local endpoints. BlueZ only
 * supports sending audio to a single endpoint, and will choose the one that was registered first with the supported
 * capabilities.
 */
class PulseAudioBluetoothInitializer
        : public common::utils::bluetooth::BluetoothEventListenerInterface
        , public std::enable_shared_from_this<PulseAudioBluetoothInitializer> {
public:
    //The State of each module.
    enum class ModuleState {
        UNKNOWN,
        INITIALLY_LOADED,
        UNLOADED,
        LOADED_BY_SDK,
    };

    // factory method to create new instace of @c PulseAudioBluetoothInitializer
    static std::shared_ptr<PulseAudioBluetoothInitializer> create(
        std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus);

    // Destructor.
    ~PulseAudioBluetoothInitializer();

protected:
    // @name BluetoothEventBusListenerInterface Functions
    void onEventFired(const common::utils::bluetooth::BluetoothEvent& event) override;

private:
    // Callback for PulseAudio Context state changes.
    static void onStateChanged(pa_context* context, void* userData);

    // A callback which indicates module found
    static void onModuleFound(pa_context* context, const pa_module_info* info, int eol, void* userData);

    // A callback which indicates the result of loading module-blluetooth-policy.
    static void onLoadPolicyResult(pa_context* context, uint32_t index, void* userData);

    // A callback which indicates the result of loading module-bluetooth-discover.
    static void onLoadDiscoverResult(pa_context* context, uint32_t index, void* userData);

    // A callback which indicates the result of unloading module-bluetooth-policy.
    static void onUnloadPolicyResult(pa_context* context, int success, void* userData);

    // A callback which indicates the result of unloading module-bluetooth-discover.
    static void onUnloadDiscoverResult(pa_context* context, int success, void* userData);

    // A helper function to handle the callback of loading a module.
    void handleLoadModuleResult(pa_context* context, uint32_t index, const std::string& moduleName);

    // A helper function to handle the callback of unloading a module.
    void handleUnloadModuleResult(pa_context* context, int success, const std::string& moduleName);

    // Update the variables tracking module state.
    bool updateStateLocked(const ModuleState& state, const std::string& module);

    // Set the state and notifies the condition variable.
    void setStateAndNotify(pa_context_state_t state);

    // Performs internal initialization of the object.
    void init();

    // Entry point to the class.
    void run();

    // Release any PulseAudio resources, stops the main thread, and other cleanup.
    void cleanup();

    // Constructor.
    PulseAudioBluetoothInitializer(std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus);

    // Condition variable for @c m_mainThread to wait for @c m_paLoop.
    std::condition_variable m_mainThreadCv;

    // Mutext protecting variables.
    std::mutex  m_mutex;

    // The eventBus which will we will receive events upon BluetoothDeviceManager initialization.
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> m_eventBus;

    // The main loop that PulseAudio callback occur on.
    pa_threaded_mainloop* m_paLoop;

    // Indicates whether we have started a PulseAudio instance. Do not start multiple instances if we get multiple
    // messages on m_eventBus.
    bool m_paLoopStarted;   

    // The PulseAudio context.
    pa_context* m_context;

    // The state of module-bluetooth-policy.
    ModuleState m_policyState;;

    // The state of module-bluetooth-discover.
    ModuleState m_discoverState;

    // Whether a connection to PulseAudio was succesful.
    bool m_connected;

    // An executor to serialize calls.
    common::utils::threading::Executor m_executor;
};

/**
 * Converts the @c ModuleState enum to a string.
 *
 * @param The @c ModuleState to convert.
 * @return A string representation of the @c ModuleState.
 */
inline std::string moduleStateToString(const PulseAudioBluetoothInitializer::ModuleState& state) {
    switch (state) {
        case PulseAudioBluetoothInitializer::ModuleState::UNKNOWN:
            return "UNKNOWN";
        case PulseAudioBluetoothInitializer::ModuleState::INITIALLY_LOADED:
            return "INITIALLY_LOADED";
        case PulseAudioBluetoothInitializer::ModuleState::UNLOADED:
            return "UNLOADED";
        case PulseAudioBluetoothInitializer::ModuleState::LOADED_BY_SDK:
            return "LOADED_BY_SDK";
    }

    return "INVALID";
}

/**
 * Overload for the @c ModuleState enum. This will write the @c ModuleState as a string to the provided
 * stream.
 *
 * @param An ostream to send the @c ModuleState as a string.
 * @param The @c ModuleState.
 * @return The stream.
 */
inline std::ostream& operator<<(std::ostream& stream, const PulseAudioBluetoothInitializer::ModuleState& state) {
    return stream << moduleStateToString(state);
}

} // namespace blueZ
} // namespace bluetoothDevice
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_PULSEAUDIOBLUETOOTHINITIALIZER_H_