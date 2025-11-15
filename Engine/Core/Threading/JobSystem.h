#pragma once

#include "Core/Threading/ThreadPool.h"
#include "Core/Logger.h"

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace SAGE {

/// @brief Job handle для отслеживания зависимостей
class JobHandle {
public:
    JobHandle() : m_Counter(std::make_shared<std::atomic<int>>(1)) {}
    
    /// @brief Проверить завершен ли job
    bool IsComplete() const {
        return m_Counter->load() == 0;
    }
    
    /// @brief Ждать завершения job
    void Wait() const {
        while (!IsComplete()) {
            std::this_thread::yield();
        }
    }
    
    /// @brief Добавить dependency
    void AddDependency() {
        m_Counter->fetch_add(1);
    }
    
    /// @brief Отметить завершение
    void Complete() {
        m_Counter->fetch_sub(1);
    }
    
private:
    std::shared_ptr<std::atomic<int>> m_Counter;
};

/// @brief Job System с dependency tracking
class JobSystem {
public:
    using JobFunc = std::function<void()>;
    
    /// @brief Инициализация Job System
    static void Initialize(size_t threadCount = 0) {
        Get().InitializeInternal(threadCount);
    }
    
    /// @brief Остановка Job System
    static void Shutdown() {
        Get().ShutdownInternal();
    }
    
    /// @brief Создать и запустить job
    /// @param func Функция для выполнения
    /// @param priority Приоритет (выше = важнее)
    /// @param dependencies Зависимости от других jobs
    static JobHandle Schedule(JobFunc func, int priority = 0, const std::vector<JobHandle>& dependencies = {}) {
        return Get().ScheduleInternal(std::move(func), priority, dependencies);
    }
    
    /// @brief Parallel-for с автоматическим разделением работы
    template<typename Func>
    static JobHandle ParallelFor(size_t start, size_t end, size_t batchSize, Func&& func) {
        if (start >= end) {
            JobHandle handle;
            handle.Complete();
            return handle;
        }
        
        JobHandle handle;
        const size_t range = end - start;
        const size_t numBatches = (range + batchSize - 1) / batchSize;
        
        for (size_t batch = 0; batch < numBatches; ++batch) {
            handle.AddDependency();
            
            size_t batchStart = start + batch * batchSize;
            size_t batchEnd = std::min(batchStart + batchSize, end);
            
            Get().m_ThreadPool.EnqueueTask(Task([&func, batchStart, batchEnd, handle]() mutable {
                for (size_t i = batchStart; i < batchEnd; ++i) {
                    func(i);
                }
                handle.Complete();
            }, 0));
        }
        
        handle.Complete();  // Убрать начальный счетчик
        return handle;
    }
    
    /// @brief Ждать завершения всех jobs
    static void WaitAll() {
        Get().m_ThreadPool.WaitAll();
    }
    
    /// @brief Получить количество потоков
    static size_t GetThreadCount() {
        return Get().m_ThreadPool.GetThreadCount();
    }
    
    /// @brief Получить статистику
    static void LogStats() {
        auto& instance = Get();
        SAGE_INFO("JobSystem Stats:");
        SAGE_INFO("  Thread Count: {}", instance.m_ThreadPool.GetThreadCount());
        SAGE_INFO("  Active Tasks: {}", instance.m_ThreadPool.GetActiveTasks());
        SAGE_INFO("  Queued Tasks: {}", instance.m_ThreadPool.GetQueuedTasks());
    }

private:
    JobSystem() = default;
    
    static JobSystem& Get() {
        static JobSystem instance;
        return instance;
    }
    
    void InitializeInternal(size_t threadCount) {
        if (m_Initialized) {
            SAGE_WARN("JobSystem already initialized");
            return;
        }
        
        m_ThreadPool = ThreadPool(threadCount);
        m_Initialized = true;
        
        SAGE_INFO("JobSystem: Initialized with {} threads", m_ThreadPool.GetThreadCount());
    }
    
    void ShutdownInternal() {
        if (!m_Initialized) return;
        
        m_ThreadPool.Shutdown();
        m_Initialized = false;
        
        SAGE_INFO("JobSystem: Shutdown complete");
    }
    
    JobHandle ScheduleInternal(JobFunc func, int priority, const std::vector<JobHandle>& dependencies) {
        JobHandle handle;
        
        // Создать задачу с проверкой зависимостей
        auto wrappedFunc = [func = std::move(func), dependencies, handle]() mutable {
            // Ждать завершения всех зависимостей
            for (const auto& dep : dependencies) {
                dep.Wait();
            }
            
            // Выполнить функцию
            func();
            
            // Отметить завершение
            handle.Complete();
        };
        
        m_ThreadPool.EnqueueTask(Task(std::move(wrappedFunc), priority));
        
        return handle;
    }

private:
    ThreadPool m_ThreadPool;
    bool m_Initialized = false;
};

/// @brief RAII wrapper для автоматической инициализации/деинициализации
class JobSystemGuard {
public:
    explicit JobSystemGuard(size_t threadCount = 0) {
        JobSystem::Initialize(threadCount);
    }
    
    ~JobSystemGuard() {
        JobSystem::Shutdown();
    }
    
    // Запретить копирование
    JobSystemGuard(const JobSystemGuard&) = delete;
    JobSystemGuard& operator=(const JobSystemGuard&) = delete;
};

} // namespace SAGE
