#pragma once

#include "Core/Logger.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace SAGE {

/// @brief Task для выполнения в ThreadPool
struct Task {
    std::function<void()> Function;
    int Priority = 0;  // Выше = важнее
    
    Task() = default;
    Task(std::function<void()> func, int priority = 0)
        : Function(std::move(func)), Priority(priority) {}
    
    bool operator<(const Task& other) const {
        return Priority < other.Priority;  // std::priority_queue - max heap
    }
};

/// @brief Thread Pool с work stealing для балансировки нагрузки
class ThreadPool {
public:
    /// @brief Создать ThreadPool с указанным количеством потоков
    /// @param threadCount Количество worker threads (0 = hardware_concurrency)
    explicit ThreadPool(size_t threadCount = 0)
        : m_Stop(false)
    {
        if (threadCount == 0) {
            threadCount = std::thread::hardware_concurrency();
            if (threadCount == 0) threadCount = 4;  // Fallback
        }
        
        m_Workers.reserve(threadCount);
        m_WorkQueues.resize(threadCount);
        
        // Создать worker threads
        for (size_t i = 0; i < threadCount; ++i) {
            m_Workers.emplace_back([this, i]() { WorkerThread(i); });
        }
        
        SAGE_INFO("ThreadPool: Started with {} threads", threadCount);
    }
    
    ~ThreadPool() {
        Shutdown();
    }
    
    // Запретить копирование
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    /// @brief Добавить задачу в очередь
    template<typename Func, typename... Args>
    auto Enqueue(Func&& func, Args&&... args) -> std::future<typename std::invoke_result_t<Func, Args...>> {
        using ReturnType = typename std::invoke_result_t<Func, Args...>;
        
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );
        
        std::future<ReturnType> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(m_GlobalQueueMutex);
            
            if (m_Stop) {
                SAGE_ERROR("ThreadPool: Cannot enqueue on stopped pool");
                throw std::runtime_error("Enqueue on stopped ThreadPool");
            }
            
            m_GlobalQueue.emplace([task]() { (*task)(); });
        }
        
        m_Condition.notify_one();
        return result;
    }
    
    /// @brief Добавить задачу с приоритетом
    void EnqueueTask(Task&& task) {
        {
            std::unique_lock<std::mutex> lock(m_GlobalQueueMutex);
            
            if (m_Stop) {
                SAGE_ERROR("ThreadPool: Cannot enqueue on stopped pool");
                return;
            }
            
            m_PriorityQueue.push(std::move(task));
        }
        
        m_Condition.notify_one();
    }
    
    /// @brief Ждать завершения всех задач
    void WaitAll() {
        std::unique_lock<std::mutex> lock(m_GlobalQueueMutex);
        m_AllTasksComplete.wait(lock, [this]() {
            return m_GlobalQueue.empty() && 
                   m_PriorityQueue.empty() && 
                   m_ActiveTasks == 0;
        });
    }
    
    /// @brief Остановить ThreadPool
    void Shutdown() {
        {
            std::unique_lock<std::mutex> lock(m_GlobalQueueMutex);
            if (m_Stop) return;
            m_Stop = true;
        }
        
        m_Condition.notify_all();
        
        for (std::thread& worker : m_Workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        m_Workers.clear();
        SAGE_INFO("ThreadPool: Shutdown complete");
    }
    
    /// @brief Получить количество потоков
    size_t GetThreadCount() const { return m_Workers.size(); }
    
    /// @brief Получить количество активных задач
    size_t GetActiveTasks() const { return m_ActiveTasks.load(); }
    
    /// @brief Получить количество задач в очереди
    size_t GetQueuedTasks() const {
        std::unique_lock<std::mutex> lock(m_GlobalQueueMutex);
        return m_GlobalQueue.size() + m_PriorityQueue.size();
    }

private:
    void WorkerThread(size_t threadIndex) {
        while (true) {
            std::function<void()> task;
            
            // Попытка получить задачу
            {
                std::unique_lock<std::mutex> lock(m_GlobalQueueMutex);
                
                m_Condition.wait(lock, [this]() {
                    return m_Stop || !m_GlobalQueue.empty() || !m_PriorityQueue.empty();
                });
                
                if (m_Stop && m_GlobalQueue.empty() && m_PriorityQueue.empty()) {
                    return;
                }
                
                // Приоритетные задачи сначала
                if (!m_PriorityQueue.empty()) {
                    task = m_PriorityQueue.top().Function;
                    m_PriorityQueue.pop();
                } else if (!m_GlobalQueue.empty()) {
                    task = std::move(m_GlobalQueue.front());
                    m_GlobalQueue.pop_front();
                }
            }
            
            if (task) {
                ++m_ActiveTasks;
                
                try {
                    task();
                } catch (const std::exception& e) {
                    SAGE_ERROR("ThreadPool: Task exception: {}", e.what());
                } catch (...) {
                    SAGE_ERROR("ThreadPool: Unknown task exception");
                }
                
                --m_ActiveTasks;
                m_AllTasksComplete.notify_all();
            }
        }
    }

private:
    std::vector<std::thread> m_Workers;
    std::vector<std::deque<Task>> m_WorkQueues;  // Per-thread queues (для work stealing)
    
    // Global queue
    std::deque<std::function<void()>> m_GlobalQueue;
    std::priority_queue<Task> m_PriorityQueue;
    
    mutable std::mutex m_GlobalQueueMutex;
    std::condition_variable m_Condition;
    std::condition_variable m_AllTasksComplete;
    
    std::atomic<bool> m_Stop;
    std::atomic<size_t> m_ActiveTasks{0};
};

/// @brief Singleton для глобального ThreadPool
class GlobalThreadPool {
public:
    static ThreadPool& Get() {
        static ThreadPool instance;
        return instance;
    }
    
    // Запретить создание копий
    GlobalThreadPool() = delete;
    GlobalThreadPool(const GlobalThreadPool&) = delete;
    GlobalThreadPool& operator=(const GlobalThreadPool&) = delete;
};

/// @brief Parallel for-loop
/// @param start Начальный индекс
/// @param end Конечный индекс (не включительно)
/// @param func Функция принимающая индекс: void(size_t index)
template<typename Func>
void ParallelFor(size_t start, size_t end, Func&& func) {
    if (start >= end) return;
    
    auto& pool = GlobalThreadPool::Get();
    const size_t threadCount = pool.GetThreadCount();
    const size_t range = end - start;
    const size_t chunkSize = std::max<size_t>(1, range / threadCount);
    
    std::vector<std::future<void>> futures;
    futures.reserve(threadCount);
    
    for (size_t i = start; i < end; i += chunkSize) {
        size_t chunkEnd = std::min(i + chunkSize, end);
        
        futures.push_back(pool.Enqueue([&func, i, chunkEnd]() {
            for (size_t idx = i; idx < chunkEnd; ++idx) {
                func(idx);
            }
        }));
    }
    
    // Ждать завершения всех chunks
    for (auto& future : futures) {
        future.wait();
    }
}

/// @brief Parallel for-each
template<typename Container, typename Func>
void ParallelForEach(Container& container, Func&& func) {
    ParallelFor(0, container.size(), [&](size_t i) {
        func(container[i]);
    });
}

} // namespace SAGE
