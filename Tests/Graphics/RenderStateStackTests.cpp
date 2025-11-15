#include "TestFramework.h"

#include "Engine/Graphics/Rendering/StateManagement/RenderStateManager.h"
#include "Engine/Graphics/Core/Types/RendererTypes.h"

using namespace TestFramework;

TEST_CASE(RenderStateManager_BlendDepthStackBalance) {
    using namespace SAGE::StateManagement;

    RenderStateManager::Init();

    const SAGE::BlendMode initialBlend = RenderStateManager::GetBlendMode();
    const SAGE::DepthSettings initialDepth = RenderStateManager::GetDepthState();

    RenderStateManager::PushBlendMode(SAGE::BlendMode::Additive);
    RenderStateManager::PushBlendMode(SAGE::BlendMode::Multiply);
    RenderStateManager::PopBlendMode();
    RenderStateManager::PopBlendMode();

    ASSERT_EQ(static_cast<int>(initialBlend), static_cast<int>(RenderStateManager::GetBlendMode()));

    SAGE::DepthSettings custom{};
    custom.testEnabled = true;
    custom.writeEnabled = false;
    custom.function = SAGE::DepthFunction::Greater;
    custom.biasConstant = 1.25f;
    custom.biasSlope = 0.5f;

    RenderStateManager::PushDepthState(custom);
    RenderStateManager::PushDepthState(initialDepth);
    RenderStateManager::PopDepthState();
    RenderStateManager::PopDepthState();

    const SAGE::DepthSettings restored = RenderStateManager::GetDepthState();
    ASSERT(restored.testEnabled == initialDepth.testEnabled);
    ASSERT(restored.writeEnabled == initialDepth.writeEnabled);
    ASSERT_EQ(static_cast<int>(restored.function), static_cast<int>(initialDepth.function));
    ASSERT_NEAR(restored.biasConstant, initialDepth.biasConstant, 1e-6f);
    ASSERT_NEAR(restored.biasSlope, initialDepth.biasSlope, 1e-6f);

    RenderStateManager::Shutdown();
}
