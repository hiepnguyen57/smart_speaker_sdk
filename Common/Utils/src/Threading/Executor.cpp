#include "Common/Utils/Memory/Memory.h"
#include "Common/Utils/Threading/Executor.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace threading {

Executor::~Executor() {
    shutdown();
}

Executor::Executor(const std::chrono::milliseconds& delayExit) :
        m_threadRunning{false},
        m_timeout{delayExit},
        m_shutdown{false} {
}

void Executor::waitForSubmittedTasks() {
    std::unique_lock<std::mutex> lock{m_queueMutex};
    if (m_threadRunning) {
        // wait for thread to exit.
        std::promise<void> flushedPromise;
        auto flushedFuture = flushedPromise.get_future();
        m_queue.emplace_back([&flushedPromise]() { flushedPromise.set_value(); });

        lock.unlock();
        m_delayedCondition.notify_one();
        flushedFuture.wait();
    }
}

std::function<void()> Executor::pop() {
    std::lock_guard<std::mutex> lock{m_queueMutex};
    if (!m_queue.empty()) {
        auto task = std::move(m_queue.front());
        m_queue.pop_front();
        return task;
    }
    return std::function<void()>();
}

bool Executor::hasNext() {
    std::unique_lock<std::mutex> lock{m_queueMutex};
    m_delayedCondition.wait_for(lock, m_timeout, [this] { return !m_queue.empty() || m_shutdown; });
    m_threadRunning = !m_queue.empty();
    return m_threadRunning;
}

bool Executor::runNext() {
    auto task = pop();
    if (task) {
        task();
    }

    return hasNext();
}

void Executor::shutdown() {
    std::unique_lock<std::mutex> lock{m_queueMutex};
    m_queue.clear();
    m_shutdown = true;
    lock.unlock();
    waitForSubmittedTasks();
}

bool Executor::isShutdown() {
    return m_shutdown;
}

}  // namespace threading
}  // namespace utils
}  // namespace common
}  // namespace deviceClientSDK