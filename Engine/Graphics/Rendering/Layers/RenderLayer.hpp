#pragma once

#include "Graphics/Core/Types/RenderTypes.hpp"
#include "Graphics/Core/Resources/Material.h"

#include <string>
#include <utility>
#include <vector>

namespace SAGE::Graphics {

    class RenderLayer {
    public:
        RenderLayer() = default;
        RenderLayer(std::string name, LayerType type, int order, bool visible,
                    SAGE::BlendMode blendMode, const SAGE::DepthSettings& depthSettings,
                    std::size_t reserveCount) {
            Initialize(std::move(name), type, order, visible, blendMode, depthSettings, reserveCount);
        }

        void Initialize(std::string name, LayerType type, int order, bool visible,
                        SAGE::BlendMode blendMode, const SAGE::DepthSettings& depthSettings,
                        std::size_t reserveCount) {
            m_Name = std::move(name);
            m_Type = type;
            m_Order = order;
            m_Visible = visible;
            m_BlendMode = blendMode;
            m_DepthSettings = depthSettings;
            m_Active = true;
            m_CommandIndices.clear();
            if (reserveCount > 0 && m_CommandIndices.capacity() < reserveCount) {
                m_CommandIndices.reserve(reserveCount);
            }
        }

        void Deactivate() {
            m_Active = false;
            m_Visible = false;
            m_CommandIndices.clear();
        }

        [[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
        [[nodiscard]] LayerType GetType() const noexcept { return m_Type; }
        [[nodiscard]] bool IsActive() const noexcept { return m_Active; }
        [[nodiscard]] bool IsVisible() const noexcept { return m_Active && m_Visible; }
        [[nodiscard]] int GetOrder() const noexcept { return m_Order; }
        [[nodiscard]] SAGE::BlendMode GetBlendMode() const noexcept { return m_BlendMode; }
        [[nodiscard]] const SAGE::DepthSettings& GetDepthSettings() const noexcept { return m_DepthSettings; }
        [[nodiscard]] bool IsDepthTestEnabled() const noexcept { return m_DepthSettings.testEnabled; }
        [[nodiscard]] bool IsDepthWriteEnabled() const noexcept { return m_DepthSettings.writeEnabled; }
        [[nodiscard]] SAGE::DepthFunction GetDepthFunction() const noexcept { return m_DepthSettings.function; }
        [[nodiscard]] float GetDepthBiasConstant() const noexcept { return m_DepthSettings.biasConstant; }
        [[nodiscard]] float GetDepthBiasSlope() const noexcept { return m_DepthSettings.biasSlope; }

        void SetVisible(bool visible) noexcept { m_Visible = visible; }
        void SetOrder(int order) noexcept { m_Order = order; }
        void SetBlendMode(SAGE::BlendMode blendMode) noexcept { m_BlendMode = blendMode; }
        void SetDepthTest(bool enabled) noexcept { m_DepthSettings.testEnabled = enabled; }
        void SetDepthWrite(bool enabled) noexcept { m_DepthSettings.writeEnabled = enabled; }
        void SetDepthFunction(SAGE::DepthFunction function) noexcept { m_DepthSettings.function = function; }
        void SetDepthBias(float constantBias, float slopeBias) noexcept {
            m_DepthSettings.biasConstant = constantBias;
            m_DepthSettings.biasSlope = slopeBias;
        }

        void ResetForFrame() {
            if (!m_Active) {
                return;
            }
            m_CommandIndices.clear();
        }
        void AppendCommand(std::size_t commandIndex) {
            if (!m_Active) {
                return;
            }
            m_CommandIndices.push_back(commandIndex);
        }

        [[nodiscard]] const std::vector<std::size_t>& GetCommandIndices() const noexcept {
            return m_CommandIndices;
        }

    private:
        std::string m_Name;
        LayerType m_Type = LayerType::World;
        int m_Order = 0;
        bool m_Visible = true;
        bool m_Active = false;
        SAGE::BlendMode m_BlendMode = SAGE::BlendMode::Alpha;
        SAGE::DepthSettings m_DepthSettings{};
        std::vector<std::size_t> m_CommandIndices;
    };

} // namespace SAGE::Graphics
