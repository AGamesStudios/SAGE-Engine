#pragma once

#include "Graphics/Core/Types/RendererTypes.h"

#include <variant>
#include <vector>

namespace SAGE::Graphics::Rendering::Commands {

struct ScreenShakeCommand {
    float amplitude = 0.0f;
    float frequency = 0.0f;
    float duration = 0.0f;
};

enum class CommandType {
    Quad,
    Text
};

struct RenderCommand {
    CommandType type = CommandType::Quad;
    QuadDesc quad{};
    TextDesc text{};

    static RenderCommand CreateQuad(const QuadDesc& command) {
        RenderCommand result;
        result.type = CommandType::Quad;
        result.quad = command;
        return result;
    }

    static RenderCommand CreateText(const TextDesc& command) {
        RenderCommand result;
        result.type = CommandType::Text;
        result.text = command;
        return result;
    }

};

class RenderCommandQueue {
public:
    void Clear() { m_Commands.clear(); }
    bool Empty() const { return m_Commands.empty(); }
    const std::vector<RenderCommand>& Commands() const { return m_Commands; }

    void PushQuad(const QuadDesc& command) {
        m_Commands.emplace_back(RenderCommand::CreateQuad(command));
    }

    void PushText(const TextDesc& command) {
        m_Commands.emplace_back(RenderCommand::CreateText(command));
    }

private:
    std::vector<RenderCommand> m_Commands;
};

} // namespace SAGE::Graphics::Rendering::Commands
