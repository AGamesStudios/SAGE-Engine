#include "EventBus.h"

#include <typeindex>
#include <limits>

namespace SAGE {

    EventBus::~EventBus() {
        StopWorker();
    }

    void EventBus::Publish(Event& event) {
        PublishInternal(event, std::type_index(typeid(event)));
    }

    void EventBus::Clear() {
        {
            std::unique_lock lock(m_Mutex);
            m_Handlers.clear();
            m_HandlerLookup.clear();
            m_NextId = 1;
        }

        {
            std::lock_guard queueLock(m_QueueMutex);
            m_ActiveQueue.clear();
            m_PendingQueue.clear();
            m_PriorityActiveQueues.clear();
            m_PriorityPendingQueues.clear();
            m_CoalescingMaps.clear();
        }

        m_EnabledCategories.store(std::numeric_limits<std::uint32_t>::max(), std::memory_order_relaxed);
        ResetStatistics();
    }

    void EventBus::StartWorker(std::chrono::milliseconds interval) {
        if (m_WorkerRunning.load(std::memory_order_acquire)) {
            SAGE_WARNING("EventBus::StartWorker called but worker already running");
            return;
        }

        m_WorkerRunning.store(true, std::memory_order_release);
        m_WorkerThread = std::thread([this, interval]() {
            while (m_WorkerRunning.load(std::memory_order_acquire)) {
                {
                    std::unique_lock lock(m_WorkerMutex);
                    m_WorkerCV.wait_for(lock, interval, [this]() {
                        return !m_WorkerRunning.load(std::memory_order_acquire);
                    });
                }

                if (!m_WorkerRunning.load(std::memory_order_acquire)) {
                    break;
                }

                Flush();
            }
        });

        if (ShouldTrace()) {
            SAGE_TRACE("EventBus: background worker started (interval={}ms)", interval.count());
        }
    }

    void EventBus::StopWorker() {
        if (!m_WorkerRunning.load(std::memory_order_acquire)) {
            return;
        }

        m_WorkerRunning.store(false, std::memory_order_release);
        m_WorkerCV.notify_all();

        if (m_WorkerThread.joinable()) {
            m_WorkerThread.join();
        }

        if (ShouldTrace()) {
            SAGE_TRACE("EventBus: background worker stopped");
        }
    }

    bool EventBus::IsWorkerRunning() const {
        return m_WorkerRunning.load(std::memory_order_acquire);
    }

} // namespace SAGE
