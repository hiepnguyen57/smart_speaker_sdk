#include <chrono>
#include <string>

#include "BlueZ/PulseAudioBluetoothInitializer.h"
#include <Common/Utils/Logger/Log.h>

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

#define TAG_PULSEAUDIO          "PulseAudioBluetooth\t"

using namespace common::utils::bluetooth;
using namespace common::utils::logger;

// The PulseAudio module related to device discovery.
static std::string BLUETOOTH_DISCOVER = "module-bluetooth-discover";

// The PulseAudio module related to stack related policies.
static std::string BLUETOOTH_POLICY = "module-bluetooth-policy";

// Return indicating an operation was successful.
static const int PA_CONTEXT_CB_SUCCESS{1};

// Return for a module callback indicating that this is the eol.
static const int PA_MODULE_CB_EOL_EOL{1};

// Return for a module callback indicating that an error occurred.
static const int PA_MODULE_CB_EOL_ERR{-1};

// Timeout for blocking operations.
static const std::chrono::seconds TIMEOUT{2};

/**
 * Converts a pa_context_state_t enum to a string.
 */
static std::string stateToString(pa_context_state_t state) {
    switch (state) {
        case PA_CONTEXT_UNCONNECTED:
            return "PA_CONTEXT_UNCONNECTED";
        case PA_CONTEXT_CONNECTING:
            return "PA_CONTEXT_CONNECTING";
        case PA_CONTEXT_AUTHORIZING:
            return "PA_CONTEXT_AUTHORIZING";
        case PA_CONTEXT_SETTING_NAME:
            return "PA_CONTEXT_SETTING_NAME";
        case PA_CONTEXT_READY:
            return "PA_CONTEXT_READY";
        case PA_CONTEXT_FAILED:
            return "PA_CONTEXT_FAILED";
        case PA_CONTEXT_TERMINATED:
            return "PA_CONTEXT_TERMINATED";
    }

    return "UNKNOWN";
}

std::shared_ptr<PulseAudioBluetoothInitializer> PulseAudioBluetoothInitializer::create(
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus) {

    if (!eventBus) {
        LOG_ERROR << TAG_PULSEAUDIO << "createFailed; reason: nullEventBus";
        return nullptr;
    }

    auto pulseAudio = std::shared_ptr<PulseAudioBluetoothInitializer>(new PulseAudioBluetoothInitializer(eventBus));
    pulseAudio->init();
    return pulseAudio;
}

PulseAudioBluetoothInitializer::PulseAudioBluetoothInitializer(
    std::shared_ptr<common::utils::bluetooth::BluetoothEventBus> eventBus) :
        m_eventBus{eventBus},
        m_paLoop{nullptr},
        m_paLoopStarted{false},
        m_context{nullptr},
        m_policyState{ModuleState::UNKNOWN},
        m_discoverState{ModuleState::UNKNOWN},
        m_connected{false} {
}

void PulseAudioBluetoothInitializer::init() {
    m_eventBus->addListener({BluetoothEventType::BLUETOOTH_DEVICE_MANAGER_INITIALIZED}, shared_from_this());
}

void PulseAudioBluetoothInitializer::onLoadDiscoverResult(pa_context* context, uint32_t index, void* userData) {
    if(!userData) {
        LOG_ERROR << TAG_PULSEAUDIO << "onLoadDiscoverResultFailed; reason: nullUserData";
        return;
    }

    PulseAudioBluetoothInitializer* initializer =  static_cast<PulseAudioBluetoothInitializer*>(userData);
    initializer->handleLoadModuleResult(context, index, BLUETOOTH_DISCOVER);
}

void PulseAudioBluetoothInitializer::onLoadPolicyResult(pa_context* context, uint32_t index, void* userData) {
    if(!userData) {
        LOG_ERROR << TAG_PULSEAUDIO << "onLoadPolicyResultFailed; reason: nullUserData";
        return;
    }  

    PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userData);
    initializer->handleLoadModuleResult(context, index, BLUETOOTH_POLICY);
}

void PulseAudioBluetoothInitializer::handleLoadModuleResult(
    pa_context* context,
    uint32_t index,
    const std::string& moduleName) {
    
    std::unique_lock<std::mutex> lock(m_mutex);

    if(!context) {
        LOG_ERROR << TAG_PULSEAUDIO << "handleLoadModuleResultFailed; reason: nullContext";
        return;
    } else if(index == PA_INVALID_INDEX) {
        LOG_ERROR <<  TAG_PULSEAUDIO << "handleLoadModuleResultFailed; reason: loadFailed";
        return;
    }

    if(updateStateLocked(ModuleState::LOADED_BY_SDK, moduleName)) {
        m_mainThreadCv.notify_one();
    }
}

void PulseAudioBluetoothInitializer::onUnloadPolicyResult(pa_context* context, int success, void* userData) {
    if(!userData) {
        LOG_ERROR << TAG_PULSEAUDIO << "onUnloadPolicyResultFailed; reason: nullUserData";
        return;
    }

    PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userData);
    initializer->handleUnloadModuleResult(context, success, BLUETOOTH_POLICY);    
}

void PulseAudioBluetoothInitializer::onUnloadDiscoverResult(pa_context* context, int success, void* userData) {
    if(!userData) {
        LOG_ERROR << TAG_PULSEAUDIO << "onUnloadDiscoverResultFailed; reason: nullUserData";
        return;
    }

    PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userData);
    initializer->handleUnloadModuleResult(context, success, BLUETOOTH_DISCOVER);    
}

void PulseAudioBluetoothInitializer::handleUnloadModuleResult(
    pa_context* context,
    int success,
    const std::string& moduleName) {
    if(!context) {
        LOG_ERROR << TAG_PULSEAUDIO << "handleUnloadModuleResultFailed; reason: nullContext";
        return;
    } else if(success != PA_CONTEXT_CB_SUCCESS) {
        LOG_ERROR << TAG_PULSEAUDIO << "handleUnloadModuleResultFailed; reason: unloadFailed";
        return;       
    }

    if(updateStateLocked(ModuleState::UNLOADED, moduleName)) {
        m_mainThreadCv.notify_one();
    }
}

void PulseAudioBluetoothInitializer::onModuleFound(
    pa_context* context,
    const pa_module_info* info,
    int eol,
    void* userData) {

    if(!context) {
        LOG_ERROR << TAG_PULSEAUDIO << "onModuleFoundFailed; reason: nullContext";
        return;
    } else if(!userData) {
        LOG_ERROR << TAG_PULSEAUDIO << "onModuleFoundFailed; reason: nullUserData";
        return;       
    } else if(eol == PA_MODULE_CB_EOL_ERR) {
        LOG_ERROR << TAG_PULSEAUDIO << "onModuleFoundFailed; reason: pulseAudioError";
        return;  
    }

    PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userData);

    std::unique_lock<std::mutex> lock(initializer->m_mutex);

    if(eol == PA_MODULE_CB_EOL_EOL) {
        LOG_DEBUG << TAG_PULSEAUDIO << "EndOfList";
        if(initializer->m_policyState != ModuleState::INITIALLY_LOADED) {
            initializer->updateStateLocked(ModuleState::UNLOADED, BLUETOOTH_POLICY);
        }

        if(initializer->m_discoverState != ModuleState::INITIALLY_LOADED) {
            initializer->updateStateLocked(ModuleState::UNLOADED, BLUETOOTH_DISCOVER);
        }

        initializer->m_mainThreadCv.notify_one();
        return;
    } else if(!info || !info->name) {
        LOG_ERROR << TAG_PULSEAUDIO << "moduleFoundFailed; reason: invalidInfo";
        return;
    }

    if(info->name == BLUETOOTH_POLICY) {
        initializer->updateStateLocked(ModuleState::INITIALLY_LOADED, BLUETOOTH_POLICY);
        pa_context_unload_module(context, info->index, &PulseAudioBluetoothInitializer::onUnloadPolicyResult, userData);
    } else if(info->name == BLUETOOTH_DISCOVER) {
        initializer->updateStateLocked(ModuleState::INITIALLY_LOADED, BLUETOOTH_DISCOVER);
        pa_context_unload_module(context, info->index, &PulseAudioBluetoothInitializer::onUnloadDiscoverResult, userData);
    }
}

bool PulseAudioBluetoothInitializer::updateStateLocked(const ModuleState& state, const std::string& module) {
    ModuleState currentState = ModuleState::UNKNOWN;

    if(module == BLUETOOTH_POLICY) {
        currentState = m_policyState;
        m_policyState = state;
    } else if(module == BLUETOOTH_DISCOVER) {
        currentState = m_discoverState;
        m_discoverState = state;
    } else {
        LOG_ERROR << TAG_PULSEAUDIO << "updateStateLockedFailed; reason: invalidModule";
        return false;
    }

    return true;
}
void PulseAudioBluetoothInitializer::onStateChanged(pa_context* context, void* userData) {
    if(!context) {
        LOG_ERROR << TAG_PULSEAUDIO << "onStateChangedFailed; reason: nullContext";
        return;
    } else if(!userData) {
        LOG_ERROR << TAG_PULSEAUDIO << "onStateChangedFailed; reason: nullUserData";
        return;
    }

    pa_context_state_t state;
    state = pa_context_get_state(context);
    
    PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userData);
    initializer->setStateAndNotify(state);
}

void PulseAudioBluetoothInitializer::setStateAndNotify(pa_context_state_t state) {
    std::unique_lock<std::mutex> lock(m_mutex);

    switch(state) {
        // Connected and ready to receive calls
        case PA_CONTEXT_READY:
            m_connected = true;
        // These are failed cases
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            m_mainThreadCv.notify_one();
            break;
        // Intermediate states that can be ignored.
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
    }
}

void PulseAudioBluetoothInitializer::cleanup() {
    if(m_context) {
        pa_context_disconnect(m_context);
        pa_context_unref(m_context);
        m_context = nullptr;
    }

    if(m_paLoop) {
        pa_threaded_mainloop_stop(m_paLoop);
        pa_threaded_mainloop_free(m_paLoop);
        m_paLoop = nullptr;
    }

    LOG_DEBUG << TAG_PULSEAUDIO << "cleanupCompleted";
}

PulseAudioBluetoothInitializer::~PulseAudioBluetoothInitializer() {
    // Ensure there are no references to PA resources being used.
    m_executor.shutdown();

    // cleanup() likely to have been previously called, but call again in case executor is shutdown prematurely.
    cleanup();   
}

void PulseAudioBluetoothInitializer::run() {
    /*
     * pa_threaded_mainloop_new creates a separate thread that PulseAudio uses for callbacks.
     * Do this so we can block and wait on the main thread and terminate early on error conditions.
     */
    m_paLoop = pa_threaded_mainloop_new();
    // Owned by m_paLoop, do not need to free.
    pa_mainloop_api* mainLoopApi = pa_threaded_mainloop_get_api(m_paLoop);
    m_context = pa_context_new(mainLoopApi, "Application to unload and reload Pulse Audio BT modules");

    pa_context_set_state_callback(m_context, &PulseAudioBluetoothInitializer::onStateChanged, this);
    pa_context_connect(m_context, NULL, PA_CONTEXT_NOFLAGS, NULL);

    if(pa_threaded_mainloop_start(m_paLoop) < 0) {
        LOG_ERROR << "runFailed; reason: runningMainLoopFailed";
        cleanup();
        return;
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(std::cv_status::timeout == m_mainThreadCv.wait_for(lock, TIMEOUT) || !m_connected) {
            cleanup();
            return;
        }
    }

    /**
     * Get a list of modules. If we find module-bluetooth-discover and module-bluetooth-policy already loadded, we
     * will unload them.
     */
    pa_context_get_module_info_list(m_context, &PulseAudioBluetoothInitializer::onModuleFound, this);

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_mainThreadCv.wait_for(lock, TIMEOUT, [this] {
            return ModuleState::UNLOADED == m_policyState && ModuleState::UNLOADED == m_discoverState;
        })) {
            LOG_DEBUG << TAG_PULSEAUDIO << "success; bluetoothModulesUnloaded";
        } else {
            LOG_ERROR << TAG_PULSEAUDIO << "runFailed; reason: unloadModulesFailed";
            cleanup();
            return;
        }
    }

    // (Re) load the modules.
    pa_context_load_module(
        m_context, BLUETOOTH_POLICY.c_str(), nullptr, &PulseAudioBluetoothInitializer::onLoadPolicyResult, this);
    pa_context_load_module(
        m_context, BLUETOOTH_DISCOVER.c_str(), nullptr, &PulseAudioBluetoothInitializer::onLoadDiscoverResult, this);

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_mainThreadCv.wait_for(lock, TIMEOUT, [this] {
            return ModuleState::LOADED_BY_SDK == m_policyState && ModuleState::LOADED_BY_SDK == m_discoverState;
        })) {
            LOG_DEBUG << TAG_PULSEAUDIO << "reason: loadModulesSuccesful";
        } else {
            LOG_ERROR << TAG_PULSEAUDIO << "runFailed; reason: loadModulesFailed";
            cleanup();
            return;
        }
    }

    LOG_DEBUG << TAG_PULSEAUDIO << "Reloading PulseAudio Bluetooth Modules Successful";

    cleanup();
}

void PulseAudioBluetoothInitializer::onEventFired(const BluetoothEvent& event) {
    if(BluetoothEventType::BLUETOOTH_DEVICE_MANAGER_INITIALIZED != event.getType()) {
        LOG_ERROR << TAG_PULSEAUDIO << "onEventFiredFailed; reason: unexpectedEventReceived";
        return;
    }

    m_executor.submit([this] {
        if(!m_paLoopStarted) {
            m_paLoopStarted = true;
            run();
        } else {
            LOG_WARN << TAG_PULSEAUDIO << "reason: loopAlreadyStarted";
        }
    });
}


} // namespace blueZ
} // namespce bluetoothDevice
} // namespace deviceClientSDK