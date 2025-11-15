#include "CommandBuffer.h"

namespace SAGE::Graphics::Batching {

void CommandBuffer::Initialize(std::size_t maxQuads) {
    m_MaxQuads = maxQuads;
    m_QuadCommands.clear();
    m_QuadCommands.reserve(m_MaxQuads);
}

void CommandBuffer::SetMaxQuads(std::size_t maxQuads) {
    m_MaxQuads = maxQuads;
    if (m_MaxQuads == 0) {
        m_QuadCommands.clear();
        return;
    }
    if (m_QuadCommands.capacity() < m_MaxQuads) {
        m_QuadCommands.reserve(m_MaxQuads);
    }
    if (m_QuadCommands.size() > m_MaxQuads) {
        m_QuadCommands.resize(m_MaxQuads);
    }
}

void CommandBuffer::Clear() {
    m_QuadCommands.clear();
}

bool CommandBuffer::Empty() const {
    return m_QuadCommands.empty();
}

std::size_t CommandBuffer::Size() const {
    return m_QuadCommands.size();
}

std::size_t CommandBuffer::Capacity() const {
    return m_MaxQuads;
}

bool CommandBuffer::PushQuad(const QuadCommand& command) {
    if (m_MaxQuads == 0 || m_QuadCommands.size() >= m_MaxQuads) {
        return false;
    }

    m_QuadCommands.emplace_back(command);
    return true;
}

} // namespace SAGE::Graphics::Batching
