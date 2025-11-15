#pragma once

#include "Graphics/Core/Types/RenderData.hpp"
#include "Graphics/Rendering/Layers/RenderLayer.hpp"
#include "Graphics/Core/Types/RenderStats.hpp"
#include "Graphics/Core/Types/RenderTypes.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace SAGE::Graphics {

    class RenderSystem {
    public:
        bool Initialize(const RenderConfig& config = {});
        void Shutdown();

        [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }

    RenderLayerHandle CreateLayer(const std::string& name, LayerType type, bool visible = true);
        void DestroyLayer(RenderLayerHandle handle);
    void SetLayerOrder(RenderLayerHandle handle, int order);
    void SetLayerVisible(RenderLayerHandle handle, bool visible);
    void SetLayerBlendMode(RenderLayerHandle handle, SAGE::BlendMode blendMode);
    void SetLayerDepthTest(RenderLayerHandle handle, bool enabled);
    void SetLayerDepthWrite(RenderLayerHandle handle, bool enabled);
    void SetLayerDepthFunction(RenderLayerHandle handle, SAGE::DepthFunction function);
    void SetLayerDepthBias(RenderLayerHandle handle, float constantBias, float slopeBias = 0.0f);
    [[nodiscard]] int GetLayerOrder(RenderLayerHandle handle) const;
    [[nodiscard]] bool IsLayerVisible(RenderLayerHandle handle) const;
    [[nodiscard]] SAGE::BlendMode GetLayerBlendMode(RenderLayerHandle handle) const;
    [[nodiscard]] bool IsLayerDepthTestEnabled(RenderLayerHandle handle) const;
    [[nodiscard]] bool IsLayerDepthWriteEnabled(RenderLayerHandle handle) const;
    [[nodiscard]] SAGE::DepthFunction GetLayerDepthFunction(RenderLayerHandle handle) const;
    [[nodiscard]] float GetLayerDepthBiasConstant(RenderLayerHandle handle) const;
    [[nodiscard]] float GetLayerDepthBiasSlope(RenderLayerHandle handle) const;

        void BeginFrame();
        void Submit(const RenderCommand& command);
        void EndFrame();

        [[nodiscard]] RenderLayerHandle GetDefaultLayer() const noexcept { return m_DefaultLayer; }
        [[nodiscard]] const RenderStats& GetStats() const noexcept { return m_Stats; }

    private:
        [[nodiscard]] bool IsLayerHandleValid(RenderLayerHandle handle) const noexcept;
        [[nodiscard]] RenderLayer& ResolveLayer(RenderLayerHandle handle);
        [[nodiscard]] const RenderLayer& ResolveLayer(RenderLayerHandle handle) const;
        void EnsureLayerOrdering();

    private:
        bool m_Initialized = false;
        bool m_InFrame = false;
        bool m_ManagesRendererLifecycle = false;
        RenderConfig m_Config{};
        RenderData m_RenderData;
        std::vector<RenderLayer> m_Layers;
        std::vector<std::uint32_t> m_FreeLayerIndices;
        RenderLayerHandle m_DefaultLayer = RenderLayerHandle::Invalid();
        RenderStats m_Stats{};
        std::chrono::steady_clock::time_point m_FrameBeginTimestamp{};
        std::vector<std::uint32_t> m_SortedLayerIndices;
        bool m_LayerOrderingDirty = true;
        int m_NextLayerOrder = 1;
    };

} // namespace SAGE::Graphics
