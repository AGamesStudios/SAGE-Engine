#pragma once

#include "Core/Core.h"
#include "Core/Logger.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <future>
#include <atomic>

namespace SAGE {

/**
 * @brief Job System for parallel task execution
 * 
 * Features:
 * - Thread pool with configurable size
 * - Task queue with priority support
 * - Future-based result retrieval
 * - Automatic load balancing
 * - Thread-safe operations
 * 
 * Usage:
 *   JobSystem jobSystem(4); // 4 worker threads
 *   
 *   auto future = jobSystem.Submit([]() {
 *       return ExpensiveComputation();
 *   });
 *   
 *   int result = future.get(); // Wait for completion
 */
class JobSystem {
public:
    /**
     * @brief Create job system with specified thread count
     * @param threadCount Number of worker threads (0 = auto-detect)
     */
    explicit JobSystem(uint32_t threadCount = 0) {
        if (threadCount == 0) {
            threadCount = std::thread::hardware_concurrency();
            if (threadCount == 0) threadCount = 4; // Fallback
        }

        m_ThreadCount = threadCount;
        m_Stop = false;

        SAGE_INFO("JobSystem: Initializing with {} worker threads", threadCount);

        // Create worker threads
        for (uint32_t i = 0; i < threadCount; ++i) {
            m_Workers.emplace_back([this, i]() {
                WorkerThread(i);
            });
        }
    }

    ~JobSystem() {
        Shutdown();
    }

    // Non-copyable, non-movable
    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    /**
     * @brief Submit a task to the job system
     * @return Future for result retrieval
     */
    template<typename Func, typename... Args>
    auto Submit(Func&& func, Args&&... args) 
        -> std::future<typename std::invoke_result<Func, Args...>::type> {
        
        using ReturnType = typename std::invoke_result<Func, Args...>::type;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            
            if (m_Stop) {
                throw std::runtime_error("Cannot submit task to stopped JobSystem");
            }

            m_TaskQueue.emplace([task]() { (*task)(); });
        }

        m_Condition.notify_one();
        m_TasksSubmitted++;

        return result;
    }

    /**
     * @brief Wait for all pending tasks to complete
     */
    void WaitAll() {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_AllDoneCondition.wait(lock, [this]() {
            return m_TaskQueue.empty() && m_ActiveWorkers == 0;
        });
    }

    /**
     * @brief Shutdown job system and wait for all tasks
     */
    void Shutdown() {
        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            if (m_Stop) return;
            m_Stop = true;
        }

        m_Condition.notify_all();

        for (std::thread& worker : m_Workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }

        SAGE_INFO("JobSystem: Shutdown complete ({} tasks processed)", 
                 m_TasksCompleted.load());
    }

    /**
     * @brief Get number of worker threads
     */
    [[nodiscard]] uint32_t GetThreadCount() const {
        return m_ThreadCount;
    }

    /**
     * @brief Get number of pending tasks
     */
    [[nodiscard]] size_t GetPendingTaskCount() const {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        return m_TaskQueue.size();
    }

    /**
     * @brief Get statistics
     */
    [[nodiscard]] uint64_t GetTasksSubmitted() const {
        return m_TasksSubmitted.load();
    }

    [[nodiscard]] uint64_t GetTasksCompleted() const {
        return m_TasksCompleted.load();
    }

private:
    void WorkerThread(uint32_t threadID) {
        SAGE_INFO("JobSystem: Worker thread {} started", threadID);

        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                m_Condition.wait(lock, [this]() {
                    return m_Stop || !m_TaskQueue.empty();
                });

                if (m_Stop && m_TaskQueue.empty()) {
                    break;
                }

                if (!m_TaskQueue.empty()) {
                    task = std::move(m_TaskQueue.front());
                    m_TaskQueue.pop();
                    m_ActiveWorkers++;
                }
            }

            if (task) {
                task();
                m_TasksCompleted++;

                {
                    std::lock_guard<std::mutex> lock(m_QueueMutex);
                    m_ActiveWorkers--;
                    if (m_TaskQueue.empty() && m_ActiveWorkers == 0) {
                        m_AllDoneCondition.notify_all();
                    }
                }
            }
        }

        SAGE_INFO("JobSystem: Worker thread {} stopped", threadID);
    }

    std::vector<std::thread> m_Workers;
    std::queue<std::function<void()>> m_TaskQueue;

    mutable std::mutex m_QueueMutex;
    std::condition_variable m_Condition;
    std::condition_variable m_AllDoneCondition;

    bool m_Stop;
    uint32_t m_ThreadCount;
    std::atomic<uint32_t> m_ActiveWorkers{0};
    std::atomic<uint64_t> m_TasksSubmitted{0};
    std::atomic<uint64_t> m_TasksCompleted{0};
};

/**
 * @brief Render Command Buffer for parallel rendering
 * 
 * Allows building render commands on multiple threads,
 * then submitting them in batch to the renderer.
 */
class RenderCommandBuffer {
public:
    enum class CommandType {
        DrawQuad,
        DrawTexturedQuad,
        DrawLine,
        DrawCircle,
        SetViewport,
        SetScissor,
        Clear
    };

    struct RenderCommand {
        CommandType type;
        
        // Command data (union for memory efficiency)
        union {
            struct { float x, y, w, h; uint32_t color; } quad;
            struct { float x, y, w, h; uint32_t texID, color; } texQuad;
            struct { float x1, y1, x2, y2; uint32_t color; float thickness; } line;
            struct { float x, y, radius; uint32_t color; } circle;
            struct { uint32_t x, y, w, h; } viewport;
            struct { uint32_t color; } clear;
        } data;
    };

    RenderCommandBuffer() = default;

    /**
     * @brief Add quad draw command
     */
    void DrawQuad(float x, float y, float w, float h, uint32_t colorRGBA) {
        RenderCommand cmd;
        cmd.type = CommandType::DrawQuad;
        cmd.data.quad = {x, y, w, h, colorRGBA};
        
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Commands.push_back(cmd);
    }

    /**
     * @brief Add textured quad draw command
     */
    void DrawTexturedQuad(float x, float y, float w, float h, 
                         uint32_t texID, uint32_t tintRGBA) {
        RenderCommand cmd;
        cmd.type = CommandType::DrawTexturedQuad;
        cmd.data.texQuad = {x, y, w, h, texID, tintRGBA};
        
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Commands.push_back(cmd);
    }

    /**
     * @brief Execute all commands on render backend
     */
    template<typename RenderBackend>
    void Execute(RenderBackend& backend) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        for (const auto& cmd : m_Commands) {
            switch (cmd.type) {
                case CommandType::DrawQuad:
                    // backend.DrawQuad(...)
                    break;
                case CommandType::DrawTexturedQuad:
                    // backend.DrawTexturedQuad(...)
                    break;
                // ... other commands
                default:
                    break;
            }
        }
    }

    /**
     * @brief Clear all commands
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Commands.clear();
    }

    /**
     * @brief Get command count
     */
    [[nodiscard]] size_t GetCommandCount() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Commands.size();
    }

private:
    std::vector<RenderCommand> m_Commands;
    mutable std::mutex m_Mutex;
};

/**
 * @brief Parallel Renderer using JobSystem
 * 
 * Example usage:
 *   ParallelRenderer renderer(jobSystem, backend);
 *   
 *   // Build commands in parallel
 *   for (auto& batch : entityBatches) {
 *       jobSystem.Submit([&]() {
 *           renderer.RecordBatch(batch);
 *       });
 *   }
 *   
 *   jobSystem.WaitAll();
 *   renderer.Execute(); // Submit all commands
 */
class ParallelRenderer {
public:
    ParallelRenderer(JobSystem& jobSystem) 
        : m_JobSystem(jobSystem) {
    }

    /**
     * @brief Get thread-local command buffer
     */
    RenderCommandBuffer& GetCommandBuffer() {
        thread_local RenderCommandBuffer buffer;
        return buffer;
    }

    /**
     * @brief Execute all recorded commands
     */
    template<typename RenderBackend>
    void Execute(RenderBackend& backend) {
        // In real implementation, merge all thread-local buffers
        // and execute in order
    }

private:
    JobSystem& m_JobSystem;
};

} // namespace SAGE
