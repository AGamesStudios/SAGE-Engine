#include "TestFramework.h"

#include "Engine/Core/EventBus.h"

#include <cstdint>
#include <memory>
#include <thread>
#include <chrono>

namespace {

    struct TestEvent : SAGE::Event {
        static SAGE::EventType GetStaticType() { return SAGE::EventType::Custom; }
        SAGE::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "TestEvent"; }
        int GetCategoryFlags() const override { return SAGE::EventCategoryNone; }
        std::string ToString() const override { return "TestEvent"; }
    };

    struct CountingEvent : SAGE::Event {
        explicit CountingEvent(int deltaValue)
            : Delta(deltaValue) {}

        static SAGE::EventType GetStaticType() { return SAGE::EventType::Custom; }
        SAGE::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "CountingEvent"; }
        int GetCategoryFlags() const override { return SAGE::EventCategoryNone; }
        std::string ToString() const override { return "CountingEvent"; }

        int Delta = 0;
    };

    struct PhysicsEvent : SAGE::Event {
        static SAGE::EventType GetStaticType() { return SAGE::EventType::PhysicsCollision; }
        SAGE::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "PhysicsEvent"; }
        int GetCategoryFlags() const override { return SAGE::EventCategoryPhysics; }
        std::string ToString() const override { return "PhysicsEvent"; }
    };

    struct InputEvent : SAGE::Event {
        static SAGE::EventType GetStaticType() { return SAGE::EventType::AppUpdate; }
        SAGE::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "InputEvent"; }
        int GetCategoryFlags() const override { return SAGE::EventCategoryInput; }
        std::string ToString() const override { return "InputEvent"; }
    };

    struct CoalescingEvent : SAGE::Event {
        explicit CoalescingEvent(int val) : Value(val) {}

        static SAGE::EventType GetStaticType() { return SAGE::EventType::Custom; }
        SAGE::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "CoalescingEvent"; }
        int GetCategoryFlags() const override { return SAGE::EventCategoryNone; }
        std::string ToString() const override { return "CoalescingEvent"; }

        bool CanCoalesce() const override { return true; }
        std::size_t GetCoalescingKey() const override {
            return std::hash<std::string>{}(GetName());
        }

        int Value = 0;
    };

} // namespace

TEST_CASE(EventBus_SubscribeScoped_AutoCleanup) {
    SAGE::EventBus bus;
    bus.EnableTracing(true);
    ASSERT_TRUE(bus.IsTracingEnabled());

    int dispatchCount = 0;
    {
        auto handle = bus.SubscribeScoped<TestEvent>([&](TestEvent& evt) {
            ++dispatchCount;
            evt.Handled = true;
        });
        ASSERT_TRUE(handle.IsActive());

        TestEvent evt{};
        bus.Publish(evt);

        ASSERT_EQ(1, dispatchCount);
        auto stats = bus.GetStatistics();
        ASSERT_EQ(1ull, stats.TotalPublished);
        ASSERT_EQ(1ull, stats.HandlersInvoked);
    }

    bus.EnableTracing(false);
    ASSERT_FALSE(bus.IsTracingEnabled());

    TestEvent evt{};
    bus.Publish(evt);
    ASSERT_EQ(1, dispatchCount);

    auto stats = bus.GetStatistics();
    ASSERT_EQ(2ull, stats.TotalPublished);
    ASSERT_EQ(1ull, stats.HandlersInvoked);
}

TEST_CASE(EventBus_UnsubscribeGroup_RemovesHandlers) {
    SAGE::EventBus bus;

    constexpr std::uint32_t groupA = 101u;
    constexpr std::uint32_t groupB = 202u;

    int groupACalls = 0;
    int groupBCalls = 0;

    auto handlerA1 = bus.Subscribe<TestEvent>([&](TestEvent&) { ++groupACalls; }, groupA);
    auto handlerA2 = bus.Subscribe<TestEvent>([&](TestEvent&) { ++groupACalls; }, groupA);
    auto handlerB1 = bus.Subscribe<TestEvent>([&](TestEvent&) { ++groupBCalls; }, groupB);

    ASSERT_NE(handlerA1, handlerA2);
    ASSERT_NE(handlerA1, handlerB1);

    TestEvent evt{};
    bus.Publish(evt);
    ASSERT_EQ(2, groupACalls);
    ASSERT_EQ(1, groupBCalls);

    auto stats = bus.GetStatistics();
    ASSERT_EQ(1ull, stats.TotalPublished);
    ASSERT_EQ(3ull, stats.HandlersInvoked);

    bus.UnsubscribeGroup(groupA);

    TestEvent evtAfterGroup{};
    bus.Publish(evtAfterGroup);
    ASSERT_EQ(2, groupACalls);
    ASSERT_EQ(2, groupBCalls);

    stats = bus.GetStatistics();
    ASSERT_EQ(2ull, stats.TotalPublished);
    ASSERT_EQ(4ull, stats.HandlersInvoked);

    bus.Unsubscribe(handlerB1);

    TestEvent evtFinal{};
    bus.Publish(evtFinal);

    stats = bus.GetStatistics();
    ASSERT_EQ(3ull, stats.TotalPublished);
    ASSERT_EQ(4ull, stats.HandlersInvoked);
}

TEST_CASE(EventBus_EnqueueFlush_DeliversDeferredEvents) {
    SAGE::EventBus bus;

    int total = 0;
    bus.Subscribe<CountingEvent>([&](CountingEvent& evt) {
        total += evt.Delta;
    });

    bus.Enqueue<CountingEvent>(5);
    ASSERT_EQ(0, total);

    bus.Flush();
    ASSERT_EQ(5, total);

    auto stats = bus.GetStatistics();
    ASSERT_EQ(1ull, stats.TotalPublished);
    ASSERT_EQ(1ull, stats.HandlersInvoked);

    bus.Enqueue(std::make_unique<CountingEvent>(3));
    bus.Enqueue<CountingEvent>(2);

    bus.Flush();
    ASSERT_EQ(10, total);

    stats = bus.GetStatistics();
    ASSERT_EQ(3ull, stats.TotalPublished);
    ASSERT_EQ(3ull, stats.HandlersInvoked);
}

TEST_CASE(EventBus_CategoryFiltering_SuppressesDisabledCategories) {
    SAGE::EventBus bus;

    int physicsCalls = 0;
    int inputCalls = 0;

    bus.SetEnabledCategories(SAGE::EventCategoryPhysics);

    bus.Subscribe<PhysicsEvent>([&](PhysicsEvent&) {
        ++physicsCalls;
    });
    bus.Subscribe<InputEvent>([&](InputEvent&) {
        ++inputCalls;
    });

    PhysicsEvent physicsEvent{};
    InputEvent inputEvent{};

    bus.Publish(physicsEvent);
    bus.Publish(inputEvent);

    ASSERT_EQ(1, physicsCalls);
    ASSERT_EQ(0, inputCalls);

    auto stats = bus.GetStatistics();
    ASSERT_EQ(2ull, stats.TotalPublished);
    ASSERT_EQ(1ull, stats.HandlersInvoked);

    bus.EnableCategories(SAGE::EventCategoryInput);
    bus.Publish(inputEvent);

    ASSERT_EQ(1, physicsCalls);
    ASSERT_EQ(1, inputCalls);

    stats = bus.GetStatistics();
    ASSERT_EQ(3ull, stats.TotalPublished);
    ASSERT_EQ(2ull, stats.HandlersInvoked);

    bus.DisableCategories(SAGE::EventCategoryPhysics);
    bus.Publish(physicsEvent);

    ASSERT_EQ(1, physicsCalls);
    ASSERT_EQ(1, inputCalls);

    stats = bus.GetStatistics();
    ASSERT_EQ(4ull, stats.TotalPublished);
    ASSERT_EQ(2ull, stats.HandlersInvoked);
}

TEST_CASE(EventBus_HandlerException_ContinuesOtherHandlers) {
    SAGE::EventBus bus;

    struct ExceptionEvent : SAGE::Event {
        static SAGE::EventType GetStaticType() { return SAGE::EventType::Custom; }
        SAGE::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "ExceptionEvent"; }
        int GetCategoryFlags() const override { return SAGE::EventCategoryNone; }
        std::string ToString() const override { return "ExceptionEvent"; }
    };

    int goodHandlerCalls = 0;
    int secondGoodHandlerCalls = 0;

    // First handler throws; must not block subsequent handlers.
    bus.Subscribe<ExceptionEvent>([](ExceptionEvent&) {
        throw std::runtime_error("Test exception from handler");
    });
    bus.Subscribe<ExceptionEvent>([&](ExceptionEvent&) {
        ++goodHandlerCalls;
    });
    bus.Subscribe<ExceptionEvent>([&](ExceptionEvent&) {
        ++secondGoodHandlerCalls;
    });

    ExceptionEvent evt{};
    // Should not propagate exception to test framework.
    bus.Publish(evt);

    ASSERT_EQ(1, goodHandlerCalls);
    ASSERT_EQ(1, secondGoodHandlerCalls);

    auto stats = bus.GetStatistics();
    // 3 handlers attempted (one throws, two succeed)
    ASSERT_EQ(1ull, stats.TotalPublished);
    ASSERT_EQ(3ull, stats.HandlersInvoked);
}

TEST_CASE(EventBus_BackgroundWorker_AutoFlush) {
    SAGE::EventBus bus;

    int total = 0;
    bus.Subscribe<CountingEvent>([&](CountingEvent& evt) {
        total += evt.Delta;
    });

    ASSERT_FALSE(bus.IsWorkerRunning());

    // Start worker with fast interval
    bus.StartWorker(std::chrono::milliseconds(50));
    ASSERT_TRUE(bus.IsWorkerRunning());

    // Enqueue events
    bus.Enqueue<CountingEvent>(10);
    bus.Enqueue<CountingEvent>(20);

    // Wait for worker to flush (2x interval + margin)
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    ASSERT_EQ(30, total);

    auto stats = bus.GetStatistics();
    ASSERT_EQ(2ull, stats.TotalPublished);
    ASSERT_EQ(2ull, stats.HandlersInvoked);

    bus.StopWorker();
    ASSERT_FALSE(bus.IsWorkerRunning());
}

TEST_CASE(EventBus_PriorityQueuing_DeliversHighFirst) {
    SAGE::EventBus bus;

    std::vector<int> deliveryOrder;

    bus.Subscribe<CountingEvent>([&](CountingEvent& evt) {
        deliveryOrder.push_back(evt.Delta);
    });

    // Enqueue in reverse priority order: Low=1, Normal=2, High=3
    bus.Enqueue<CountingEvent>(SAGE::EventPriority::Low, 1);
    bus.Enqueue<CountingEvent>(SAGE::EventPriority::Normal, 2);
    bus.Enqueue<CountingEvent>(SAGE::EventPriority::High, 3);

    ASSERT_EQ(0u, deliveryOrder.size());

    bus.Flush();

    // Expected order: High(3), Normal(2), Low(1)
    ASSERT_EQ(3u, deliveryOrder.size());
    ASSERT_EQ(3, deliveryOrder[0]);
    ASSERT_EQ(2, deliveryOrder[1]);
    ASSERT_EQ(1, deliveryOrder[2]);

    auto stats = bus.GetStatistics();
    ASSERT_EQ(3ull, stats.TotalPublished);
    ASSERT_EQ(3ull, stats.HandlersInvoked);
}

TEST_CASE(EventBus_Coalescing_CollapsesDuplicates) {
    SAGE::EventBus bus;

    std::vector<int> receivedValues;

    bus.Subscribe<CoalescingEvent>([&](CoalescingEvent& evt) {
        receivedValues.push_back(evt.Value);
    });

    // Enqueue 5 coalescing events with same key
    bus.Enqueue<CoalescingEvent>(SAGE::EventPriority::Normal, 10);
    bus.Enqueue<CoalescingEvent>(SAGE::EventPriority::Normal, 20);
    bus.Enqueue<CoalescingEvent>(SAGE::EventPriority::Normal, 30);
    bus.Enqueue<CoalescingEvent>(SAGE::EventPriority::Normal, 40);
    bus.Enqueue<CoalescingEvent>(SAGE::EventPriority::Normal, 50);

    ASSERT_EQ(0u, receivedValues.size());

    bus.Flush();

    // Only last event should be delivered
    ASSERT_EQ(1u, receivedValues.size());
    ASSERT_EQ(50, receivedValues[0]);

    auto stats = bus.GetStatistics();
    ASSERT_EQ(1ull, stats.TotalPublished);
    ASSERT_EQ(1ull, stats.HandlersInvoked);
}

TEST_CASE(EventBus_WeakLifetime_AutoExpires) {
    SAGE::EventBus bus;

    int callCount = 0;

    // Create owner and subscribe with weak reference
    {
        auto owner = std::make_shared<int>(42);
        
        bus.SubscribeWeak<TestEvent>(owner, [&](TestEvent&) {
            ++callCount;
        });

        // Owner is alive, handler should be invoked
        TestEvent evt;
        bus.Publish(evt);
        ASSERT_EQ(1, callCount);

        // Owner goes out of scope here
    }

    // Owner destroyed, handler should NOT be invoked
    TestEvent evt2;
    bus.Publish(evt2);
    ASSERT_EQ(1, callCount); // Still 1, not incremented

    auto stats = bus.GetStatistics();
    ASSERT_EQ(2ull, stats.TotalPublished);
    ASSERT_EQ(1ull, stats.HandlersInvoked); // Only first invocation counted
}

TEST_CASE(EventBus_Benchmark_ImmediatePublish) {
    SAGE::EventBus bus;

    int handlerCount = 0;
    bus.Subscribe<TestEvent>([&](TestEvent&) {
        ++handlerCount;
    });

    const int iterations = 100000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        TestEvent evt;
        bus.Publish(evt);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    ASSERT_EQ(iterations, handlerCount);
    
    // Print performance metric (not an assertion, just informational)
    double avgMicroseconds = static_cast<double>(duration.count()) / iterations;
    SAGE_INFO("EventBus immediate publish: {} events in {} us (avg {} us/event)", 
              iterations, duration.count(), avgMicroseconds);
}

TEST_CASE(EventBus_Benchmark_QueuedPublish) {
    SAGE::EventBus bus;

    int handlerCount = 0;
    bus.Subscribe<CountingEvent>([&](CountingEvent&) {
        ++handlerCount;
    });

    const int iterations = 100000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        bus.Enqueue<CountingEvent>(i);
    }
    bus.Flush();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    ASSERT_EQ(iterations, handlerCount);
    
    double avgMicroseconds = static_cast<double>(duration.count()) / iterations;
    SAGE_INFO("EventBus queued publish: {} events in {} us (avg {} us/event)", 
              iterations, duration.count(), avgMicroseconds);
}

TEST_CASE(EventBus_Benchmark_CategoryFiltering) {
    SAGE::EventBus bus;

    int physicsCount = 0;
    int inputCount = 0;

    bus.Subscribe<PhysicsEvent>([&](PhysicsEvent&) {
        ++physicsCount;
    });

    bus.Subscribe<InputEvent>([&](InputEvent&) {
        ++inputCount;
    });

    // Disable physics category
    bus.DisableCategories(SAGE::EventCategoryPhysics);

    const int iterations = 50000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        PhysicsEvent pEvt;
        bus.Publish(pEvt);
        InputEvent iEvt;
        bus.Publish(iEvt);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    ASSERT_EQ(0, physicsCount); // Physics filtered out
    ASSERT_EQ(iterations, inputCount); // Input delivered

    double avgMicroseconds = static_cast<double>(duration.count()) / (iterations * 2);
    SAGE_INFO("EventBus category filtering: {} events in {} us (avg {} us/event)", 
              iterations * 2, duration.count(), avgMicroseconds);
}

TEST_CASE(EventBus_Benchmark_MultipleHandlers) {
    SAGE::EventBus bus;

    std::atomic<int> totalInvocations{0};

    // Subscribe 10 handlers
    for (int h = 0; h < 10; ++h) {
        bus.Subscribe<TestEvent>([&](TestEvent&) {
            totalInvocations.fetch_add(1, std::memory_order_relaxed);
        });
    }

    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        TestEvent evt;
        bus.Publish(evt);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    ASSERT_EQ(iterations * 10, totalInvocations.load());
    
    double avgMicroseconds = static_cast<double>(duration.count()) / iterations;
    SAGE_INFO("EventBus 10 handlers: {} events in {} us (avg {} us/event, {} us/handler)", 
              iterations, duration.count(), avgMicroseconds, avgMicroseconds / 10.0);
}

