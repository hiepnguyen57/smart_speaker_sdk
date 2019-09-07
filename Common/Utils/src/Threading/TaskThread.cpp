#include "Common/Utils/Logger/Log.h"
#include "Common/Utils/Threading/ThreadMoniker.h"
#include "Common/Utils/Threading/TaskThread.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace threading {

static const std::string TAG_TASKTHREAD = "TaskThread\t";

using namespace logger;

TaskThread::TaskThread() : m_alreadyStarting{false}, m_moniker{ThreadMoniker::generateMoniker()} {
}

TaskThread::~TaskThread() {
    m_stop = true;
    if(m_thread.joinable()) {
        m_thread.join();
    }
}

bool TaskThread::start(std::function<bool()> jobRunner) {
    if (!jobRunner) {
        LOG_ERROR << TAG_TASKTHREAD << "startFailed, reason: invalidFunction";
        return false;
    }

    bool notRunning = false;
    if (!m_alreadyStarting.compare_exchange_strong(notRunning, true)) {
        LOG_ERROR << TAG_TASKTHREAD << "startFailed, reason: tooManyThreads";
        return false;
    }

    m_oldThread = std::move(m_thread);
    m_thread = std::thread{std::bind(&TaskThread::run, this, std::move(jobRunner))};
    return true;
}

void TaskThread::run(std::function<bool()> jobRunner) {
    if (m_oldThread.joinable()) {
        m_stop = true;
        m_oldThread.join();
    }

    // Reset stop flag and already starting flag.
    m_stop = false;
    m_alreadyStarting = false;
    ThreadMoniker::setThisThreadMoniker(m_moniker);

    while (!m_stop && jobRunner())
        ;
}
}  // namespace threading
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK
