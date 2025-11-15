#include "TestFramework.h"

#include <SAGE.h>

#include <cmath>
#include <memory>
#include <utility>
#include <string>

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
        bool entered = false;
        bool exited = false;
        bool eventHandled = false;
        int updateCount = 0;
        float lastDelta = 0.0f;
    };

    class DummyScene : public Scene {
    public:
        explicit DummyScene(const std::shared_ptr<DummySceneState>& state)
            : Scene("Dummy"), m_State(state) {}

        void OnEnter(const Scene::TransitionContext& /*context*/) override {
            m_State->entered = true;
        }

        void OnExit() override {
            m_State->exited = true;
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
        int entered = 0;
        int exited = 0;
        int paused = 0;
        int resumed = 0;
        int updateCalls = 0;
        int fixedCalls = 0;
        std::string lastEnterPrevious;
        std::string lastResumePrevious;
        SceneParameters lastEnterParams;
        SceneParameters lastResumeParams;
        bool lastEnterRestored = false;
        bool lastResumeRestored = false;
    };

    struct StatefulTrackingData : TrackingData {
        int lastCounterOnLoad = -1;
    };

    class TrackingScene : public Scene {
    public:
        TrackingScene(std::string name, std::shared_ptr<TrackingData> data)
            : Scene(std::move(name)), m_Data(std::move(data)) {}

        void OnEnter(const Scene::TransitionContext& context) override {
            ++m_Data->entered;
            m_Data->lastEnterPrevious = context.PreviousScene ? context.PreviousScene->GetName() : std::string{};
            m_Data->lastEnterParams = context.Parameters;
            m_Data->lastEnterRestored = context.StateRestored;
        }

        void OnExit() override { ++m_Data->exited; }
        void OnPause() override { ++m_Data->paused; }

        void OnResume(const Scene::TransitionContext& context) override {
            ++m_Data->resumed;
            m_Data->lastResumePrevious = context.PreviousScene ? context.PreviousScene->GetName() : std::string{};
            m_Data->lastResumeParams = context.Parameters;
            m_Data->lastResumeRestored = context.StateRestored;
        }

        void OnUpdate(float) override {
            ++m_Data->updateCalls;
        }

        void OnFixedUpdate(float) override {
            ++m_Data->fixedCalls;
        }

    protected:
        std::shared_ptr<TrackingData> m_Data;
    };

    class StatefulScene : public TrackingScene {
    public:
        explicit StatefulScene(const std::shared_ptr<StatefulTrackingData>& data)
            : TrackingScene("Stateful", data), m_StatefulData(data) {}

        void OnUpdate(float deltaTime) override {
            TrackingScene::OnUpdate(deltaTime);
            ++m_Counter;
        }

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

        [[nodiscard]] bool IsPersistent() const override { return true; }
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
    CHECK(state->entered);
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
    CHECK(state->exited);
    CHECK(stack.Size() == 0);
}

TEST_CASE(SceneStack_PauseResumeChain) {
    SceneStack stack;
    auto baseData = std::make_shared<TrackingData>();
    auto overlayData = std::make_shared<TrackingData>();

    stack.PushScene(CreateScope<TrackingScene>("Game", baseData));
    CHECK(baseData->entered == 1);

    stack.OnUpdate(0.016f);
    CHECK(baseData->updateCalls == 1);

    stack.PushScene(CreateScope<TrackingScene>("Pause", overlayData));
    CHECK(baseData->paused == 1);
    CHECK(overlayData->entered == 1);

    stack.OnUpdate(0.016f);
    CHECK(baseData->updateCalls == 1);
    CHECK(overlayData->updateCalls == 1);

    stack.OnFixedUpdate(0.02f);
    CHECK(baseData->fixedCalls == 0);
    CHECK(overlayData->fixedCalls == 1);

    SceneParameters resumeParams;
    resumeParams.Set("reason", std::string("back"));
    stack.PopTopScene(std::move(resumeParams), false);

    CHECK(baseData->resumed == 1);
    auto reason = baseData->lastResumeParams.Get<std::string>("reason");
    REQUIRE(reason.has_value());
    CHECK(*reason == "back");

    stack.OnUpdate(0.016f);
    CHECK(baseData->updateCalls == 2);
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
    CHECK(statefulData->entered == 1);
    CHECK(statefulData->resumed == 0);
    CHECK(statefulData->lastEnterPrevious.empty());
    CHECK(!statefulData->lastEnterRestored);

    stack.OnUpdate(0.016f);
    stack.OnUpdate(0.016f);
    stack.OnUpdate(0.016f);
    CHECK(top->GetCounter() == 3);

    SceneParameters overlayParams;
    overlayParams.Set("message", std::string("Pause"));
    manager.QueuePush("Overlay", std::move(overlayParams));
    manager.ProcessTransitions(stack);
    CHECK(stack.Size() == 2);
    CHECK(statefulData->paused == 1);
    CHECK(overlayData->entered == 1);
    CHECK(overlayData->resumed == 0);
    CHECK(overlayData->lastEnterPrevious == "Stateful");
    auto overlayMessage = overlayData->lastEnterParams.Get<std::string>("message");
    REQUIRE(overlayMessage.has_value());
    CHECK(*overlayMessage == "Pause");

    SceneParameters resumeParams;
    resumeParams.Set("overlayResult", 7);
    manager.QueuePop(true, std::move(resumeParams));
    manager.ProcessTransitions(stack);
    CHECK(stack.Size() == 1);
    CHECK(overlayData->exited == 1);
    CHECK(statefulData->resumed == 1);
    CHECK(statefulData->lastResumePrevious == "Overlay");
    auto resumeValue = statefulData->lastResumeParams.Get<int>("overlayResult");
    REQUIRE(resumeValue.has_value());
    CHECK(*resumeValue == 7);
    CHECK(!statefulData->lastResumeRestored);

    SceneParameters replaceParams;
    replaceParams.Set("restart", true);
    manager.QueueReplace("Stateful", std::move(replaceParams), true, true);
    manager.ProcessTransitions(stack);

    auto* replacement = dynamic_cast<StatefulScene*>(stack.GetTopScene());
    REQUIRE(replacement != nullptr);
    CHECK(statefulData->entered == 2);
    CHECK(statefulData->exited == 1);
    CHECK(statefulData->lastEnterRestored);
    auto restartFlag = statefulData->lastEnterParams.Get<bool>("restart");
    REQUIRE(restartFlag.has_value());
    CHECK(*restartFlag);
    CHECK(manager.HasSavedState("Stateful"));
    CHECK(replacement->GetCounter() == 3);
    CHECK(statefulData->lastCounterOnLoad == 3);
}

TEST_CASE(Texture_CalculateFootprint) {
    using SAGE::Texture;

    const std::size_t rgba = Texture::CalculateDataFootprint(Texture::Format::RGBA8, 256, 256, 1, false);
    CHECK(rgba == 256ULL * 256ULL * 4ULL);

    const std::size_t rgbaMip = Texture::CalculateDataFootprint(Texture::Format::RGBA8, 128, 128, 3, false);
    CHECK(rgbaMip > rgba);

    const std::size_t bc1 = Texture::CalculateDataFootprint(Texture::Format::BC1, 256, 256, 1, true);
    CHECK(bc1 == ((256ULL + 3ULL) / 4ULL) * ((256ULL + 3ULL) / 4ULL) * Texture::BytesPerBlock(Texture::Format::BC1));

    const std::size_t bc1Mip = Texture::CalculateDataFootprint(Texture::Format::BC1, 256, 256, 4, true);
    CHECK(bc1Mip > bc1);
}

TEST_CASE(SceneManager_SwapReplacesOverlayWithoutResumingBase) {
    SceneStack stack;
    SceneManager manager;

    auto gameData = std::make_shared<TrackingData>();
    auto overlayA = std::make_shared<TrackingData>();
    auto overlayB = std::make_shared<TrackingData>();

    manager.RegisterScene("Game", [gameData]() {
        return CreateScope<TrackingScene>("Game", gameData);
    });

    manager.RegisterScene("PauseA", [overlayA]() {
        return CreateScope<TrackingScene>("PauseA", overlayA);
    });

    manager.RegisterScene("PauseB", [overlayB]() {
        return CreateScope<TrackingScene>("PauseB", overlayB);
    });

    manager.QueuePush("Game");
    manager.ProcessTransitions(stack);
    CHECK(gameData->entered == 1);

    manager.QueuePush("PauseA");
    manager.ProcessTransitions(stack);
    CHECK(gameData->paused == 1);
    CHECK(overlayA->entered == 1);

    SceneParameters swapParams;
    swapParams.Set("theme", std::string("dark"));
    manager.QueueSwap("PauseB", std::move(swapParams));
    manager.ProcessTransitions(stack);

    CHECK(gameData->paused == 1);
    CHECK(gameData->resumed == 0);

    CHECK(overlayA->exited == 1);
    CHECK(overlayB->entered == 1);
    auto theme = overlayB->lastEnterParams.Get<std::string>("theme");
    REQUIRE(theme.has_value());
    CHECK(*theme == "dark");

    stack.OnUpdate(0.016f);
    CHECK(overlayB->updateCalls == 1);
    CHECK(gameData->updateCalls == 0);
}

TEST_CASE(SceneManager_RestoresStateOnResume) {
    SceneStack stack;
    SceneManager manager;

    auto gameData = std::make_shared<StatefulTrackingData>();
    auto overlayData = std::make_shared<TrackingData>();

    manager.RegisterScene("Game", [gameData]() {
        return CreateScope<StatefulScene>(gameData);
    });

    manager.RegisterScene("Pause", [overlayData]() {
        return CreateScope<TrackingScene>("Pause", overlayData);
    });

    manager.QueuePush("Game");
    manager.ProcessTransitions(stack);

    auto* game = dynamic_cast<StatefulScene*>(stack.GetTopScene());
    REQUIRE(game != nullptr);

    stack.OnUpdate(0.016f);
    stack.OnUpdate(0.016f);
    stack.OnUpdate(0.016f);
    CHECK(game->GetCounter() == 3);

    manager.SaveState(*game);
    stack.OnUpdate(0.016f);
    stack.OnUpdate(0.016f);
    CHECK(game->GetCounter() == 5);

    manager.QueuePush("Pause");
    manager.ProcessTransitions(stack);

    manager.QueuePop(false, SceneParameters{}, true);
    manager.ProcessTransitions(stack);

    CHECK(gameData->resumed == 1);
    CHECK(gameData->lastResumeRestored);
    CHECK(game->GetCounter() == 3);
}
