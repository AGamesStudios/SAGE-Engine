#include "Graphics/API/RenderSystem.hpp"

#include "Graphics/Rendering/Layers/RenderLayer.hpp"
#include "Graphics/Core/Types/RenderTypes.hpp"
#include "Graphics/API/Renderer.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"

#include <algorithm>
#include <limits>

namespace SAGE::Graphics {

    namespace {
        constexpr std::uint32_t kInvalidIndex = std::numeric_limits<std::uint32_t>::max();
    }

    bool RenderSystem::Initialize(const RenderConfig& config) {
        if (m_Initialized) {
            SAGE_WARNING("RenderSystem already initialized");
            return true;
        }

        m_Config = config;
        m_RenderData.Clear();
        m_RenderData.Reserve(m_Config.initialCommandCapacity);
        m_Layers.clear();
        m_Layers.reserve(m_Config.initialLayerCapacity);
        m_FreeLayerIndices.clear();
        m_SortedLayerIndices.clear();
        m_LayerOrderingDirty = true;
        m_NextLayerOrder = 1;

        if (!SAGE::Renderer::IsInitialized()) {
            SAGE::Renderer::Init();
            m_ManagesRendererLifecycle = true;
        } else {
            m_ManagesRendererLifecycle = false;
        }

    SAGE::DepthSettings defaultDepth{};
    defaultDepth.testEnabled = false;
    defaultDepth.writeEnabled = false;
    defaultDepth.function = SAGE::DepthFunction::LessEqual;
    defaultDepth.biasConstant = 0.0f;
    defaultDepth.biasSlope = 0.0f;

    m_Layers.emplace_back();
    m_DefaultLayer.index = static_cast<std::uint32_t>(m_Layers.size() - 1);
    m_Layers.back().Initialize("Default", LayerType::World, 0, true, SAGE::BlendMode::Alpha,
                   defaultDepth, m_Config.initialCommandCapacity);
        m_LayerOrderingDirty = true;

        m_Initialized = true;
        return true;
    }

    void RenderSystem::Shutdown() {
        if (!m_Initialized) {
            return;
        }

        if (m_ManagesRendererLifecycle && SAGE::Renderer::IsInitialized()) {
            SAGE::Renderer::Shutdown();
        }

        m_Layers.clear();
        m_FreeLayerIndices.clear();
        m_SortedLayerIndices.clear();
        m_RenderData.Clear();
        m_Stats = {};
        m_DefaultLayer = RenderLayerHandle::Invalid();
        m_Initialized = false;
        m_InFrame = false;
        m_ManagesRendererLifecycle = false;
        m_LayerOrderingDirty = true;
        m_NextLayerOrder = 1;
    }

    RenderLayerHandle RenderSystem::CreateLayer(const std::string& name, LayerType type, bool visible) {
        if (!m_Initialized) {
            SAGE_ERROR("RenderSystem must be initialized before creating layers");
            return RenderLayerHandle::Invalid();
        }

        const int order = m_NextLayerOrder++;
        SAGE::DepthSettings depthSettings{};

        std::uint32_t index = kInvalidIndex;
        if (!m_FreeLayerIndices.empty()) {
            index = m_FreeLayerIndices.back();
            m_FreeLayerIndices.pop_back();
            RenderLayer& layer = m_Layers[index];
            layer.Initialize(name, type, order, visible, SAGE::BlendMode::Alpha,
                             depthSettings, m_Config.initialCommandCapacity);
        } else {
            m_Layers.emplace_back();
            index = static_cast<std::uint32_t>(m_Layers.size() - 1);
            m_Layers.back().Initialize(name, type, order, visible, SAGE::BlendMode::Alpha,
                                       depthSettings, m_Config.initialCommandCapacity);
        }

        m_LayerOrderingDirty = true;
        return RenderLayerHandle{ index };
    }

    void RenderSystem::DestroyLayer(RenderLayerHandle handle) {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("DestroyLayer called with invalid handle");
            return;
        }

        if (handle.index == m_DefaultLayer.index) {
            SAGE_WARNING("Default render layer cannot be destroyed");
            return;
        }

        RenderLayer& layer = m_Layers[handle.index];
        layer.Deactivate();
        m_FreeLayerIndices.push_back(handle.index);
        m_LayerOrderingDirty = true;
    }

    void RenderSystem::SetLayerOrder(RenderLayerHandle handle, int order) {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("SetLayerOrder called with invalid handle");
            return;
        }

        RenderLayer& layer = ResolveLayer(handle);
        if (layer.GetOrder() == order) {
            return;
        }

        layer.SetOrder(order);
        m_LayerOrderingDirty = true;
    }

    void RenderSystem::SetLayerVisible(RenderLayerHandle handle, bool visible) {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("SetLayerVisible called with invalid handle");
            return;
        }

        if (handle.index == m_DefaultLayer.index && !visible) {
            SAGE_WARNING("Default render layer cannot be hidden");
            return;
        }

        RenderLayer& layer = ResolveLayer(handle);
        layer.SetVisible(visible);
    }

    int RenderSystem::GetLayerOrder(RenderLayerHandle handle) const {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("GetLayerOrder called with invalid handle");
            return 0;
        }

        return ResolveLayer(handle).GetOrder();
    }

    bool RenderSystem::IsLayerVisible(RenderLayerHandle handle) const {
        if (!IsLayerHandleValid(handle)) {
            return false;
        }

        return ResolveLayer(handle).IsVisible();
    }

    void RenderSystem::SetLayerBlendMode(RenderLayerHandle handle, SAGE::BlendMode blendMode) {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("SetLayerBlendMode called with invalid handle");
            return;
        }

        RenderLayer& layer = ResolveLayer(handle);
        if (layer.GetBlendMode() == blendMode) {
            return;
        }

        layer.SetBlendMode(blendMode);
    }

    void RenderSystem::SetLayerDepthTest(RenderLayerHandle handle, bool enabled) {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("SetLayerDepthTest called with invalid handle");
            return;
        }

        RenderLayer& layer = ResolveLayer(handle);
        layer.SetDepthTest(enabled);
    }

    void RenderSystem::SetLayerDepthWrite(RenderLayerHandle handle, bool enabled) {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("SetLayerDepthWrite called with invalid handle");
            return;
        }

        RenderLayer& layer = ResolveLayer(handle);
        layer.SetDepthWrite(enabled);
    }

    void RenderSystem::SetLayerDepthFunction(RenderLayerHandle handle, SAGE::DepthFunction function) {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("SetLayerDepthFunction called with invalid handle");
            return;
        }

        RenderLayer& layer = ResolveLayer(handle);
        layer.SetDepthFunction(function);
    }

    void RenderSystem::SetLayerDepthBias(RenderLayerHandle handle, float constantBias, float slopeBias) {
        if (!IsLayerHandleValid(handle)) {
            SAGE_WARNING("SetLayerDepthBias called with invalid handle");
            return;
        }

        RenderLayer& layer = ResolveLayer(handle);
        layer.SetDepthBias(constantBias, slopeBias);
    }

    SAGE::BlendMode RenderSystem::GetLayerBlendMode(RenderLayerHandle handle) const {
        if (!IsLayerHandleValid(handle)) {
            return SAGE::BlendMode::Alpha;
        }

        return ResolveLayer(handle).GetBlendMode();
    }

    bool RenderSystem::IsLayerDepthTestEnabled(RenderLayerHandle handle) const {
        if (!IsLayerHandleValid(handle)) {
            return false;
        }

        return ResolveLayer(handle).IsDepthTestEnabled();
    }

    bool RenderSystem::IsLayerDepthWriteEnabled(RenderLayerHandle handle) const {
        if (!IsLayerHandleValid(handle)) {
            return false;
        }

        return ResolveLayer(handle).IsDepthWriteEnabled();
    }

    SAGE::DepthFunction RenderSystem::GetLayerDepthFunction(RenderLayerHandle handle) const {
        if (!IsLayerHandleValid(handle)) {
            return SAGE::DepthFunction::LessEqual;
        }

        return ResolveLayer(handle).GetDepthFunction();
    }

    float RenderSystem::GetLayerDepthBiasConstant(RenderLayerHandle handle) const {
        if (!IsLayerHandleValid(handle)) {
            return 0.0f;
        }

        return ResolveLayer(handle).GetDepthBiasConstant();
    }

    float RenderSystem::GetLayerDepthBiasSlope(RenderLayerHandle handle) const {
        if (!IsLayerHandleValid(handle)) {
            return 0.0f;
        }

        return ResolveLayer(handle).GetDepthBiasSlope();
    }

    void RenderSystem::BeginFrame() {
        if (!m_Initialized) {
            SAGE_ERROR("RenderSystem::BeginFrame called before initialization");
            return;
        }
        if (m_InFrame) {
            SAGE_WARNING("RenderSystem::BeginFrame called while a frame is already active");
            return;
        }
        if (!SAGE::Renderer::IsInitialized()) {
            SAGE_ERROR("RenderSystem::BeginFrame requires Renderer to be initialized");
            return;
        }

        m_RenderData.Clear();
        m_RenderData.Reserve(m_Config.initialCommandCapacity);

        EnsureLayerOrdering();
        for (std::uint32_t index : m_SortedLayerIndices) {
            m_Layers[index].ResetForFrame();
        }

        m_Stats = {};
        m_FrameBeginTimestamp = std::chrono::steady_clock::now();

        SAGE::Renderer::BeginScene();
        m_InFrame = true;
    }

    void RenderSystem::Submit(const RenderCommand& command) {
        if (!m_InFrame) {
            SAGE_WARNING("RenderSystem::Submit called outside BeginFrame/EndFrame window");
            return;
        }

        RenderLayer* targetLayer = nullptr;
        if (IsLayerHandleValid(command.layer)) {
            RenderLayer& candidate = ResolveLayer(command.layer);
            if (candidate.IsVisible()) {
                targetLayer = &candidate;
            }
        }

        if (targetLayer == nullptr) {
            if (!IsLayerHandleValid(m_DefaultLayer) || !ResolveLayer(m_DefaultLayer).IsVisible()) {
                SAGE_WARNING("RenderSystem::Submit has no visible layer available; command ignored");
                return;
            }
            targetLayer = &ResolveLayer(m_DefaultLayer);
        }

        const std::size_t commandIndex = m_RenderData.Push(command.quad);
        targetLayer->AppendCommand(commandIndex);
        ++m_Stats.submittedQuads;
    }

    void RenderSystem::EndFrame() {
        if (!m_InFrame) {
            return;
        }

        EnsureLayerOrdering();

        bool renderFailed = false;

        for (std::uint32_t layerIndex : m_SortedLayerIndices) {
            const RenderLayer& layer = m_Layers[layerIndex];
            if (!layer.IsVisible()) {
                continue;
            }

            SAGE::Renderer::PushLayer(static_cast<float>(layer.GetOrder()));
            SAGE::Renderer::PushBlendMode(layer.GetBlendMode());
            SAGE::Renderer::PushDepthState(layer.IsDepthTestEnabled(), layer.IsDepthWriteEnabled(),
                                           layer.GetDepthFunction(), layer.GetDepthBiasConstant(),
                                           layer.GetDepthBiasSlope());

            for (std::size_t index : layer.GetCommandIndices()) {
                const QuadDesc quad = m_RenderData.Reconstruct(index);
                if (!SAGE::Renderer::DrawQuad(quad)) {
                    SAGE_ERROR("RenderSystem::EndFrame failed to queue quad for layer {0} (commandIndex={1})", layer.GetOrder(), index);
                    renderFailed = true;
                    break;
                }
                ++m_Stats.executedDrawCalls;
            }

            SAGE::Renderer::PopDepthState();
            SAGE::Renderer::PopBlendMode();
            SAGE::Renderer::PopLayer();

            if (renderFailed) {
                break;
            }
        }

        const bool endSceneOk = SAGE::Renderer::EndScene();
        renderFailed = renderFailed || !endSceneOk;
        if (!endSceneOk) {
            SAGE_ERROR("RenderSystem::EndFrame detected renderer flush failure");
        }

        const auto frameEnd = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration<float, std::milli>(frameEnd - m_FrameBeginTimestamp);
        m_Stats.frameTimeMs = elapsed.count();

        m_InFrame = false;

        if (renderFailed) {
            // Surface a high-priority metric to aid diagnostics.
            Profiler::RecordMetric("RenderSystem/LastFrameRenderFailed", 1.0f);
        }
        else {
            Profiler::RecordMetric("RenderSystem/LastFrameRenderFailed", 0.0f);
        }
    }

    bool RenderSystem::IsLayerHandleValid(RenderLayerHandle handle) const noexcept {
        return handle.IsValid() && handle.index < m_Layers.size() && m_Layers[handle.index].IsActive();
    }

    RenderLayer& RenderSystem::ResolveLayer(RenderLayerHandle handle) {
        return m_Layers[handle.index];
    }

    const RenderLayer& RenderSystem::ResolveLayer(RenderLayerHandle handle) const {
        return m_Layers[handle.index];
    }

    void RenderSystem::EnsureLayerOrdering() {
        if (!m_LayerOrderingDirty) {
            return;
        }

        m_SortedLayerIndices.clear();
        m_SortedLayerIndices.reserve(m_Layers.size());
        for (std::uint32_t index = 0; index < m_Layers.size(); ++index) {
            if (!m_Layers[index].IsActive()) {
                continue;
            }
            m_SortedLayerIndices.push_back(index);
        }

        std::stable_sort(m_SortedLayerIndices.begin(), m_SortedLayerIndices.end(),
            [this](std::uint32_t lhs, std::uint32_t rhs) {
                const RenderLayer& a = m_Layers[lhs];
                const RenderLayer& b = m_Layers[rhs];
                if (a.GetOrder() == b.GetOrder()) {
                    return lhs < rhs;
                }
                return a.GetOrder() < b.GetOrder();
            });

        m_LayerOrderingDirty = false;
    }

} // namespace SAGE::Graphics
