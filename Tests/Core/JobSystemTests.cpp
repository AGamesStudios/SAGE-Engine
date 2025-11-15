// Tests/Core/JobSystemTests.cpp

#include "../TestFramework.h"
#include "../../Engine/Core/Threading/ThreadPool.h"
#include "../../Engine/Core/Threading/JobSystem.h"
#include <thread>
<parameter name="chrono">
#include <atomic>
#include <vector>

using namespace SAGE;

// ============= ThreadPool Tests =============

TEST_CASE(ThreadPool_BasicTaskExecution) {
    GlobalThreadPool::Get().Initialize(2);
    
    std::atomic<bool> executed{false};
    
    GlobalThreadPool::Get().Enqueue([&executed]() {
        executed = true;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT(executed.load());
}

TEST_CASE(ThreadPool_MultipleTasks) {
    GlobalThreadPool::Get().Initialize(4);
    
    std::atomic<int> counter{0};
    const int taskCount = 100;
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < taskCount; ++i) {
        futures.push_back(GlobalThreadPool::Get().Enqueue([&counter]() {
            counter++;
        }));
    }
    
    // Wait for all tasks
    for (auto& future : futures) {
        future.wait();
    }
    
    ASSERT(counter.load() == taskCount);
}

TEST_CASE(ThreadPool_TaskReturnValues) {
    GlobalThreadPool::Get().Initialize(2);
    
    auto future = GlobalThreadPool::Get().Enqueue([]() {
        return 42;
    });
    
    int result = future.get();
    ASSERT(result == 42);
}

TEST_CASE(ThreadPool_WaitAll) {
    GlobalThreadPool::Get().Initialize(4);
    
    std::vector<std::future<void>> futures;
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 10; ++i) {
        futures.push_back(GlobalThreadPool::Get().Enqueue([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
        }));
    }
    
    GlobalThreadPool::Get().WaitAll(futures);
    
    ASSERT(counter.load() == 10);
}

TEST_CASE(ThreadPool_ParallelFor) {
    GlobalThreadPool::Get().Initialize(4);
    
    std::vector<int> data(1000, 0);
    
    ParallelFor(0, static_cast<int>(data.size()), [&data](int i) {
        data[i] = i * 2;
    });
    
    // Verify results
    bool allCorrect = true;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] != static_cast<int>(i * 2)) {
            allCorrect = false;
            break;
        }
    }
    
    ASSERT(allCorrect);
}

TEST_CASE(ThreadPool_ThreadSafety) {
    GlobalThreadPool::Get().Initialize(8);
    
    std::atomic<int> counter{0};
    std::mutex mutex;
    std::vector<int> vec;
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 100; ++i) {
        futures.push_back(GlobalThreadPool::Get().Enqueue([&, i]() {
            counter++;
            std::lock_guard<std::mutex> lock(mutex);
            vec.push_back(i);
        }));
    }
    
    GlobalThreadPool::Get().WaitAll(futures);
    
    ASSERT(counter.load() == 100);
    ASSERT(vec.size() == 100);
}

// ============= JobSystem Tests =============

TEST_CASE(JobSystem_BasicJobExecution) {
    JobSystemGuard guard;
    
    std::atomic<bool> executed{false};
    
    auto job = JobSystem::Schedule([&executed]() {
        executed = true;
    });
    
    job.Wait();
    
    ASSERT(job.IsComplete());
}

TEST_CASE(JobSystem_JobDependencies) {
    JobSystemGuard guard;
    
    std::atomic<int> executionOrder{0};
    std::atomic<int> job1Order{0};
    std::atomic<int> job2Order{0};
    
    auto job1 = JobSystem::Schedule([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        job1Order = ++executionOrder;
    });
    
    auto job2 = JobSystem::Schedule([&]() {
        job2Order = ++executionOrder;
    }, {job1});
    
    job2.Wait();
    
    ASSERT(job1Order.load() < job2Order.load());
}

TEST_CASE(JobSystem_ParallelForBatchProcessing) {
    JobSystemGuard guard;
    
    std::vector<int> data(1000, 0);
    
    auto job = JobSystem::ParallelFor(0, static_cast<int>(data.size()), [&data](int i) {
        data[i] = i * 3;
    }, 64);
    
    job.Wait();
    
    // Verify
    bool allCorrect = true;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] != static_cast<int>(i * 3)) {
            allCorrect = false;
            break;
        }
    }
    
    ASSERT(allCorrect);
}

TEST_CASE(JobSystem_MultipleDependencies) {
    JobSystemGuard guard;
    
    std::atomic<int> executionOrder{0};
    std::atomic<int> job1Order{0};
    std::atomic<int> job2Order{0};
    std::atomic<int> job3Order{0};
    std::atomic<int> job4Order{0};
    
    auto job1 = JobSystem::Schedule([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        job1Order = ++executionOrder;
    });
    
    auto job2 = JobSystem::Schedule([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        job2Order = ++executionOrder;
    });
    
    auto job3 = JobSystem::Schedule([&]() {
        job3Order = ++executionOrder;
    }, {job1, job2});
    
    auto job4 = JobSystem::Schedule([&]() {
        job4Order = ++executionOrder;
    }, {job3});
    
    job4.Wait();
    
    ASSERT(job1Order.load() > 0 && job2Order.load() > 0);
    ASSERT(job3Order.load() > job1Order.load() && job3Order.load() > job2Order.load());
    ASSERT(job4Order.load() > job3Order.load());
}
