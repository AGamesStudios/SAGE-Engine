#pragma once

#include "Graphics/Core/Resources/Spritesheet.h"
#include <cstdint>

namespace SAGE {

// Lightweight animator for a single row in a spritesheet.
// Usage:
//   SpriteAnimator anim(sheet, rowIndex, 0.12f, true);
//   anim.Update(deltaSeconds);
//   unsigned frameIndex = anim.GetFrame();
//   auto uv = sheet.GetUV(frameIndex);
class SpriteAnimator {
public:
    SpriteAnimator() = default;
    SpriteAnimator(const Spritesheet* sheet,
                   unsigned int row,
                   float frameDuration,
                   bool looping = true)
        : m_Sheet(sheet), m_Row(row), m_FrameDuration(frameDuration), m_Looping(looping) {
        Reset();
    }

    void SetSheet(const Spritesheet* sheet) { m_Sheet = sheet; Reset(); }
    void SetRow(unsigned int row) { m_Row = row; Reset(); }
    void SetFrameDuration(float secs) { m_FrameDuration = secs; }
    void SetLooping(bool looping) { m_Looping = looping; }

    void Reset() {
        m_CurrentFrame = 0;
        m_Accum = 0.0f;
        m_Finished = false;
        m_RowFrameCount = (m_Sheet && m_Row < m_Sheet->GetRows()) ? m_Sheet->GetColumns() : 0u;
    }

    void Update(float dt) {
        if(!m_Sheet || m_Finished || m_RowFrameCount==0 || m_FrameDuration <= 0.0f) return;
        m_Accum += dt;
        while(m_Accum >= m_FrameDuration) {
            m_Accum -= m_FrameDuration;
            ++m_CurrentFrame;
            if(m_CurrentFrame >= m_RowFrameCount) {
                if(m_Looping) {
                    m_CurrentFrame = 0;
                } else {
                    m_CurrentFrame = m_RowFrameCount - 1; // clamp
                    m_Finished = true;
                }
            }
        }
    }

    unsigned int GetFrame() const { return m_RowFrameCount? (m_Row * m_RowFrameCount + m_CurrentFrame) : 0u; }
    unsigned int GetLocalFrame() const { return m_CurrentFrame; }
    bool IsFinished() const { return m_Finished; }
    float GetProgress() const { return m_RowFrameCount? float(m_CurrentFrame) / float(m_RowFrameCount) : 0.0f; }

private:
    const Spritesheet* m_Sheet = nullptr;
    unsigned int m_Row = 0;
    unsigned int m_CurrentFrame = 0;
    unsigned int m_RowFrameCount = 0;
    float m_FrameDuration = 0.1f;
    float m_Accum = 0.0f;
    bool m_Looping = true;
    bool m_Finished = false;
};

} // namespace SAGE
