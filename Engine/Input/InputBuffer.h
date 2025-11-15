#pragma once

#include "InputAction.h"
#include <deque>
#include <chrono>

/**
 * @file InputBuffer.h
 * @brief Input buffering for frame-perfect inputs
 */

namespace SAGE {

/**
 * @brief Buffered input entry
 */
struct BufferedInput {
    std::string actionName;
    ActionState state;
    std::chrono::steady_clock::time_point timestamp;
    
    BufferedInput(const std::string& name, ActionState s)
        : actionName(name), state(s), timestamp(std::chrono::steady_clock::now()) {}
};

/**
 * @brief Input buffer for fighting games, platformers, etc.
 * 
 * Stores recent inputs with timestamps to allow:
 * - Input buffering (press jump before landing -> auto-jump on land)
 * - Combo detection (quarter-circle-forward + punch)
 * - Input forgiveness (early inputs count)
 */
class InputBuffer {
public:
    /**
     * @brief Constructor
     * @param bufferTimeMs How long to keep inputs in buffer (default 200ms)
     * @param maxSize Maximum number of buffered inputs
     */
    explicit InputBuffer(int bufferTimeMs = 200, size_t maxSize = 32)
        : m_BufferTimeMs(bufferTimeMs), m_MaxSize(maxSize) {}
    
    /**
     * @brief Add input to buffer
     */
    void AddInput(const std::string& actionName, ActionState state);
    
    /**
     * @brief Check if action was pressed within buffer time
     * @param actionName Action to check
     * @param consumeInput If true, remove the input from buffer after checking
     */
    bool WasPressed(const std::string& actionName, bool consumeInput = true);
    
    /**
     * @brief Check for input sequence (e.g., "Down", "Forward", "Punch")
     * @param sequence List of action names in order
     * @param maxSequenceTimeMs Maximum time for entire sequence (default 500ms)
     * @param consumeInputs If true, remove consumed inputs from buffer
     */
    bool CheckSequence(const std::vector<std::string>& sequence, 
                      int maxSequenceTimeMs = 500,
                      bool consumeInputs = true);
    
    /**
     * @brief Clear old inputs beyond buffer time
     */
    void Update();
    
    /**
     * @brief Clear all buffered inputs
     */
    void Clear() { m_Buffer.clear(); }
    
    /**
     * @brief Get buffer size
     */
    size_t GetSize() const { return m_Buffer.size(); }
    
    /**
     * @brief Set buffer time
     */
    void SetBufferTime(int ms) { m_BufferTimeMs = ms; }
    int GetBufferTime() const { return m_BufferTimeMs; }
    
private:
    std::deque<BufferedInput> m_Buffer;
    int m_BufferTimeMs;
    size_t m_MaxSize;
};

} // namespace SAGE
