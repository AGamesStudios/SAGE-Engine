#pragma once

#include "RenderTypes.hpp"

#include <cstdint>
#include <vector>

namespace SAGE::Graphics {

    struct RenderData {
        std::vector<Float2> positions;
        std::vector<Float2> sizes;
        std::vector<Color> colors;
        std::vector<Ref<Texture>> textures;
        std::vector<Float2> uvMins;
        std::vector<Float2> uvMaxs;
        std::vector<std::uint8_t> screenSpace;

        void Clear() {
            positions.clear();
            sizes.clear();
            colors.clear();
            textures.clear();
            uvMins.clear();
            uvMaxs.clear();
            screenSpace.clear();
        }

        void Reserve(std::size_t count) {
            positions.reserve(count);
            sizes.reserve(count);
            colors.reserve(count);
            textures.reserve(count);
            uvMins.reserve(count);
            uvMaxs.reserve(count);
            screenSpace.reserve(count);
        }
        [[nodiscard]] std::size_t Size() const noexcept { return positions.size(); }

        std::size_t Push(const QuadDesc& desc) {
            const std::size_t index = positions.size();
            positions.push_back(desc.position);
            sizes.push_back(desc.size);
            colors.push_back(desc.color);
            textures.push_back(desc.texture);
            uvMins.push_back(desc.uvMin);
            uvMaxs.push_back(desc.uvMax);
            screenSpace.push_back(static_cast<std::uint8_t>(desc.screenSpace ? 1 : 0));
            return index;
        }

        [[nodiscard]] QuadDesc Reconstruct(std::size_t index) const {
            QuadDesc quad;
            if (index >= positions.size()) {
                return quad;
            }
            quad.position = positions[index];
            quad.size = sizes[index];
            quad.color = colors[index];
            quad.texture = textures[index];
            quad.uvMin = uvMins[index];
            quad.uvMax = uvMaxs[index];
            quad.screenSpace = screenSpace[index] != 0;
            return quad;
        }
    };

} // namespace SAGE::Graphics
