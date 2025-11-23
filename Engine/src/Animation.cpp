#include "SAGE/Graphics/Animation.h"
#include "SAGE/Log.h"
#include <algorithm>

namespace SAGE {

// ============================================
// AnimationClip Implementation
// ============================================

AnimationClip::AnimationClip(const std::string& name, bool loop)
    : m_Name(name)
    , m_Looping(loop)
{
}

void AnimationClip::AddFrame(const AnimationFrame& frame) {
    m_Frames.push_back(frame);
}

void AnimationClip::AddFrame(const Rect& uvRect, float duration) {
    AnimationFrame frame;
    frame.uvRect = uvRect;
    frame.duration = duration;
    m_Frames.push_back(frame);
}

void AnimationClip::ClearFrames() {
    m_Frames.clear();
}

void AnimationClip::SetFrameRate(float fps) {
    if (fps <= 0.0f) {
        SAGE_WARN("AnimationClip::SetFrameRate - FPS must be > 0, got {}", fps);
        return;
    }
    
    if (m_Frames.empty()) {
        SAGE_WARN("AnimationClip::SetFrameRate - No frames to modify");
        return;
    }
    
    float frameDuration = 1.0f / fps;
    for (auto& frame : m_Frames) {
        frame.duration = frameDuration;
    }
}

float AnimationClip::GetTotalDuration() const {
    float total = 0.0f;
    for (const auto& frame : m_Frames) {
        total += frame.duration;
    }
    return total;
}

const AnimationFrame& AnimationClip::GetFrame(size_t index) const {
    static AnimationFrame s_EmptyFrame;
    if (index >= m_Frames.size()) {
        SAGE_ERROR("AnimationClip::GetFrame - Index {} out of range (size: {})", index, m_Frames.size());
        return s_EmptyFrame;
    }
    return m_Frames[index];
}

// ============================================
// SpriteSheetAnimationBuilder Implementation
// ============================================

SpriteSheetAnimationBuilder::SpriteSheetAnimationBuilder(
    int textureWidth, int textureHeight,
    int frameWidth, int frameHeight)
    : m_TextureWidth(textureWidth)
    , m_TextureHeight(textureHeight)
    , m_FrameWidth(frameWidth)
    , m_FrameHeight(frameHeight)
{
    m_GridCols = textureWidth / frameWidth;
    m_GridRows = textureHeight / frameHeight;
}

AnimationClip SpriteSheetAnimationBuilder::BuildClip(
    const std::string& name,
    int startX, int startY,
    int frameCount,
    float frameDuration,
    bool looping)
{
    AnimationClip clip(name, looping);

    for (int i = 0; i < frameCount; ++i) {
        int gridX = startX + i;
        int gridY = startY;

        // Wrap to next row if needed
        while (gridX >= m_GridCols) {
            gridX -= m_GridCols;
            gridY++;
        }

        if (gridY >= m_GridRows) {
            SAGE_WARN("SpriteSheetAnimationBuilder::BuildClip - Frame {} out of bounds", i);
            break;
        }

        // Calculate UV coordinates (normalized 0-1)
        if (m_TextureWidth == 0 || m_TextureHeight == 0) {
            SAGE_ERROR("SpriteSheetAnimationBuilder::BuildClip - Invalid texture dimensions");
            break;
        }
        
        float w = static_cast<float>(m_FrameWidth) / m_TextureWidth;
        float h = static_cast<float>(m_FrameHeight) / m_TextureHeight;
        
        float u = static_cast<float>(gridX * m_FrameWidth) / m_TextureWidth;
        
        // Texture is NOT flipped (Top-Left origin).
        // Row 0 is at V=0.
        // We use negative height to indicate Top-Down UVs for SpriteRenderer.
        // v_top = gridY * h.
        // v_bottom = v_top + h.
        // We want v0 = v_bottom, v1 = v_top.
        // SpriteRenderer: v0 = y, v1 = y + h.
        // So y = v_bottom, h = -h.
        
        float v_top = static_cast<float>(gridY) * h;
        float v_bottom = v_top + h;

        clip.AddFrame(Rect{u, v_bottom, w, -h}, frameDuration);
    }

    return clip;
}

AnimationClip SpriteSheetAnimationBuilder::BuildHorizontalStrip(
    const std::string& name,
    int row,
    int frameCount,
    float frameDuration,
    bool looping)
{
    return BuildClip(name, 0, row, frameCount, frameDuration, looping);
}

AnimationClip SpriteSheetAnimationBuilder::BuildVerticalStrip(
    const std::string& name,
    int column,
    int frameCount,
    float frameDuration,
    bool looping)
{
    AnimationClip clip(name, looping);

    for (int i = 0; i < frameCount; ++i) {
        int gridX = column;
        int gridY = i;

        if (gridY >= m_GridRows) {
            SAGE_WARN("SpriteSheetAnimationBuilder::BuildVerticalStrip - Frame {} out of bounds", i);
            break;
        }

        if (m_TextureWidth == 0 || m_TextureHeight == 0) {
            SAGE_ERROR("SpriteSheetAnimationBuilder::BuildVerticalStrip - Invalid texture dimensions");
            break;
        }

        float w = static_cast<float>(m_FrameWidth) / m_TextureWidth;
        float h = static_cast<float>(m_FrameHeight) / m_TextureHeight;

        float u = static_cast<float>(gridX * m_FrameWidth) / m_TextureWidth;
        
        float v_top = static_cast<float>(gridY) * h;
        float v_bottom = v_top + h;

        clip.AddFrame(Rect{u, v_bottom, w, -h}, frameDuration);
    }

    return clip;
}

} // namespace SAGE
