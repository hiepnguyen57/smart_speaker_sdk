#include "Common/Utils/RequiresShutdown.h"
#include "Common/Utils/Logger/Log.h"

#include <mutex>
#include <unordered_set>

namespace deviceClientSDK {
namespace common {
namespace utils {

using namespace logger;

#define TAG_REQUIRESSHUTDOWN            "RequiresShutdown\t"

// Class used to track whether @c RequiresShutdown objects have been shut down correctly.
class ShutdownMonitor {
public:
    /**
     * Adds a @c RequiresShutdown object to the set of objects being tracked.  This must be called at construction of
     * a @c RequiresShutdown object.
     *
     * @param object A pointer to the object to track.
     */
    void add(const RequiresShutdown* object);

    /**
     * Removes a @c RequiresShutdown object from the set of objects being tracked.  This must be called at destruction
     * of a @c RequiresShutdown object.
     *
     * @param object A pointer to the object to track.
     */
    void remove(const RequiresShutdown* object);

    /// Constructor
    ShutdownMonitor();

    /// Destructor.
    ~ShutdownMonitor();

private:
    // Protects access to @c m_objects.
    std::mutex m_mutex;

    /// Alias to the container type used to hold objects.
    using Objects = std::unordered_set<const RequiresShutdown*>;

    /// The @c RequiredShutdown objects being tracked.
    Objects m_objects;
};

void ShutdownMonitor::add(const RequiresShutdown* object) {
    if(nullptr == object) {
        LOG_ERROR << TAG_REQUIRESSHUTDOWN << "addFailed, reason: nullptrObject";
    }
    bool inserted = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        inserted = m_objects.insert(object).second;
    }
    if(!inserted) {
        LOG_ERROR << TAG_REQUIRESSHUTDOWN << "addFailed, reason: alreadyAdded, name: " << object->name();
    }
}

void ShutdownMonitor::remove(const RequiresShutdown* object) {
    if(nullptr == object) {
        LOG_ERROR << TAG_REQUIRESSHUTDOWN << "removeFailed, reason: nullptrObject";
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_objects.erase(object) == 0) {
        LOG_ERROR << TAG_REQUIRESSHUTDOWN << "removeFailed, reason: notFound, name: " << object->name();
    }
}

ShutdownMonitor::ShutdownMonitor() {

}

ShutdownMonitor::~ShutdownMonitor() {

}

/// The global @c ShutdownMonitor used to track all @c RequiresShutdown objects.
static ShutdownMonitor g_shutdownMonitor;

RequiresShutdown::RequiresShutdown(const std::string& name) : m_name{name}, m_isShutdown{false} {
    g_shutdownMonitor.add(this);
}

RequiresShutdown::~RequiresShutdown() {
    if (!m_isShutdown) {
        LOG_ERROR << TAG_REQUIRESSHUTDOWN << "~RequiresShutdownFailed, reason: notShutdown, name: " << name();
    }
    g_shutdownMonitor.remove(this);
}

const std::string& RequiresShutdown::name() const {
    return m_name;
}

void RequiresShutdown::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) {
        LOG_ERROR << TAG_REQUIRESSHUTDOWN << "shutdownFailed, reason: alreadyShutdown, name: " << name();
        return;
    }
    doShutdown();
    m_isShutdown = true;
}

bool RequiresShutdown::isShutdown() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_isShutdown;
}

}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK