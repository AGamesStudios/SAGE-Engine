#include "TestFramework.h"

#include <SAGE.h>

#include <cmath>
#include <memory>

using namespace SAGE;

TEST_CASE(EventBus_PublishesAndUnsubscribes) {
    EventBus bus;
    bool invoked = false;

    auto token = bus.Subscribe<AppTickEvent>([&](AppTickEvent&) {
        invoked = true;
    });

    AppTickEvent tickEvent;
    bus.Publish(tickEvent);
    CHECK(invoked);

    invoked = false;
    bus.Unsubscribe<AppTickEvent>(token);
    bus.Publish(tickEvent);
    CHECK(!invoked);
}

namespace {
    struct DummySceneState {
        bool attached = false;
        bool detached = false;
        bool eventHandled = false;
        int updateCount = 0;
        float lastDelta = 0.0f;
    };

    class DummyScene : public Scene {
    public:
        explicit DummyScene(const std::shared_ptr<DummySceneState>& state)
            : Scene("Dummy"), m_State(state) {}

        void OnAttach() override {
            m_State->attached = true;
        }

        void OnDetach() override {
            m_State->detached = true;
        }

        void OnUpdate(float deltaTime) override {
            ++m_State->updateCount;
            m_State->lastDelta = deltaTime;
        }

        void OnEvent(Event& event) override {
            EventDispatcher dispatcher(event);
            dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent&) {
                m_State->eventHandled = true;
                return true;
            });
        }

    private:
        std::shared_ptr<DummySceneState> m_State;
    };

    struct TrackingData {
        int attached = 0;
        int detached = 0;
        int paused = 0;
        int resumed = 0;
    };

    struct StatefulTrackingData : TrackingData {
        int lastCounterOnLoad = -1;
    };

    class TrackingScene : public Scene {
    public:
        TrackingScene(std::string name, std::shared_ptr<TrackingData> data)
            : Scene(std::move(name)), m_Data(std::move(data)) {}

        void OnAttach() override { ++m_Data->attached; }
        void OnDetach() override { ++m_Data->detached; }
        void OnPause() override { ++m_Data->paused; }
        void OnResume() override { ++m_Data->resumed; }

    protected:
        std::shared_ptr<TrackingData> m_Data;
    };

    class StatefulScene : public TrackingScene {
    public:
        explicit StatefulScene(const std::shared_ptr<StatefulTrackingData>& data)
            : TrackingScene("Stateful", data), m_StatefulData(data) {}

        void OnUpdate(float) override { ++m_Counter; }

        void SaveState(SceneState& outState) const override {
            outState.Set("counter", m_Counter);
        }

        void LoadState(const SceneState& state) override {
            if (auto value = state.Get<int>("counter")) {
                m_Counter = *value;
                if (m_StatefulData) {
                    m_StatefulData->lastCounterOnLoad = m_Counter;
                }
            }
        }

        [[nodiscard]] int GetCounter() const { return m_Counter; }

    private:
        int m_Counter = 0;
        std::shared_ptr<StatefulTrackingData> m_StatefulData;
    };
}

TEST_CASE(SceneStack_ManagesLifecycle) {
    SceneStack stack;
    auto state = std::make_shared<DummySceneState>();

    stack.PushScene(CreateScope<DummyScene>(state));
    CHECK(state->attached);
    CHECK(stack.Size() == 1);

    const float delta = 0.25f;
    stack.OnUpdate(delta);
    CHECK(state->updateCount == 1);
    CHECK(std::abs(state->lastDelta - delta) < 0.0001f);

    WindowCloseEvent closeEvent;
    stack.OnEvent(closeEvent);
    CHECK(closeEvent.Handled);
    CHECK(state->eventHandled);

    stack.PopTopScene();
    CHECK(state->detached);
    CHECK(stack.Size() == 0);
}

TEST_CASE(SceneManager_QueuesTransitionsAndRestoresState) {
    SceneStack stack;
    SceneManager manager;

    auto statefulData = std::make_shared<StatefulTrackingData>();
    auto overlayData = std::make_shared<TrackingData>();

    manager.RegisterScene("Stateful", [statefulData]() {
        return CreateScope<StatefulScene>(statefulData);
    });

    manager.RegisterScene("Overlay", [overlayData]() {
        return CreateScope<TrackingScene>("Overlay", overlayData);
    });

    manager.QueuePush("Stateful");
    manager.ProcessTransitions(stack);

    CHECK(stack.Size() == 1);
    auto* top = dynamic_cast<StatefulScene*>(stack.GetTopScene());
    REQUIRE(top != nullptr);
    CHECK(statefulData->attached == 1);
    CHECK(statefulData->resumed == 1);

    stack.OnUpdate(0.016f);
    stack.OnUpdate(0.016f);
    stack.OnUpdate(0.016f);
    CHECK(top->GetCounter() == 3);

    manager.QueuePush("Overlay");
    manager.ProcessTransitions(stack);
    CHECK(stack.Size() == 2);
    CHECK(statefulData->paused == 1);
    CHECK(overlayData->attached == 1);
    CHECK(overlayData->resumed == 1);

    manager.QueuePop(true);
    manager.ProcessTransitions(stack);
    CHECK(stack.Size() == 1);
    CHECK(overlayData->detached == 1);
    CHECK(statefulData->resumed == 2);

    manager.QueueReplace("Stateful", true, true);
    manager.ProcessTransitions(stack);

    auto* replacement = dynamic_cast<StatefulScene*>(stack.GetTopScene());
    REQUIRE(replacement != nullptr);
    CHECK(statefulData->attached == 2);
    CHECK(statefulData->detached == 1);
    CHECK(manager.HasSavedState("Stateful"));
    CHECK(replacement->GetCounter() == 3);
    CHECK(statefulData->lastCounterOnLoad == 3);
}
