#include "InputBuffer.h"

namespace SAGE {

void InputBuffer::AddInput(const std::string& actionName, ActionState state) {
    // Only buffer press events
    if (state != ActionState::Pressed) {
        return;
    }
    
    m_Buffer.emplace_back(actionName, state);
    
    // Limit buffer size
    while (m_Buffer.size() > m_MaxSize) {
        m_Buffer.pop_front();
    }
}

bool InputBuffer::WasPressed(const std::string& actionName, bool consumeInput) {
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = m_Buffer.begin(); it != m_Buffer.end(); ++it) {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->timestamp).count();
        
        if (age > m_BufferTimeMs) {
            continue; // Too old
        }
        
        if (it->actionName == actionName && it->state == ActionState::Pressed) {
            if (consumeInput) {
                m_Buffer.erase(it);
            }
            return true;
        }
    }
    
    return false;
}

bool InputBuffer::CheckSequence(const std::vector<std::string>& sequence,
                               int maxSequenceTimeMs,
                               bool consumeInputs) {
    if (sequence.empty() || m_Buffer.empty()) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    std::vector<std::deque<BufferedInput>::iterator> matchedInputs;
    size_t sequenceIndex = 0;
    
    // Search buffer backwards (most recent first) for sequence
    for (auto it = m_Buffer.rbegin(); it != m_Buffer.rend(); ++it) {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->timestamp).count();
        
        if (age > maxSequenceTimeMs) {
            break; // Too old, stop searching
        }
        
        // Check if this input matches current position in sequence
        if (sequenceIndex < sequence.size() && it->actionName == sequence[sequence.size() - 1 - sequenceIndex]) {
            matchedInputs.push_back(std::prev(it.base()));
            sequenceIndex++;
            
            if (sequenceIndex == sequence.size()) {
                // Full sequence matched!
                if (consumeInputs) {
                    // Remove matched inputs
                    for (auto matchIt : matchedInputs) {
                        m_Buffer.erase(matchIt);
                    }
                }
                return true;
            }
        }
    }
    
    return false;
}

void InputBuffer::Update() {
    auto now = std::chrono::steady_clock::now();
    
    // Remove old inputs
    while (!m_Buffer.empty()) {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_Buffer.front().timestamp).count();
        
        if (age > m_BufferTimeMs) {
            m_Buffer.pop_front();
        } else {
            break; // Rest are newer
        }
    }
}

} // namespace SAGE
