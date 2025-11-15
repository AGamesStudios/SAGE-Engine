#pragma once

#include "Graphics/Core/Animation/AnimationClip.h"
#include "Memory/Ref.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace SAGE {

    /**
     * @brief AnimationAtlas - sprite sheet with multiple animation clips
     * 
     * Loaded from JSON configuration files that define:
     * - Texture atlas path
     * - Frame definitions (pixel rects, UVs)
     * - Animation clip definitions (frame sequences, timings)
     * 
     * JSON format:
     * {
     *   "texture": "assets/character.png",
     *   "textureWidth": 512,
     *   "textureHeight": 256,
     *   "frames": [
     *     {"name": "idle_0", "x": 0, "y": 0, "w": 32, "h": 32, "pivotX": 0.5, "pivotY": 1.0},
     *     {"name": "idle_1", "x": 32, "y": 0, "w": 32, "h": 32, "pivotX": 0.5, "pivotY": 1.0},
     *     ...
     *   ],
     *   "animations": [
     *     {
     *       "name": "idle",
     *       "frames": ["idle_0", "idle_1", "idle_2", "idle_3"],
     *       "fps": 8,
     *       "loop": true
     *     },
     *     {
     *       "name": "walk",
     *       "frames": ["walk_0", "walk_1", "walk_2", "walk_3"],
     *       "fps": 12,
     *       "loop": true
     *     }
     *   ]
     * }
     */
    class AnimationAtlas {
    public:
        AnimationAtlas() = default;
        ~AnimationAtlas() = default;
        
        // Load from JSON file
        bool LoadFromFile(const std::string& filepath);
        
        // Save to JSON file (for editor)
        bool SaveToFile(const std::string& filepath) const;
        
        // Get animation clip by name
        Ref<AnimationClip> GetClip(const std::string& name) const;
        
        // Get all clip names
        std::vector<std::string> GetClipNames() const;
        
        // Atlas metadata
        const std::string& GetTexturePath() const { return m_TexturePath; }
        void SetTexturePath(const std::string& path) { m_TexturePath = path; }
        
        int GetTextureWidth() const { return m_TextureWidth; }
        int GetTextureHeight() const { return m_TextureHeight; }
        void SetTextureDimensions(int width, int height) {
            m_TextureWidth = width;
            m_TextureHeight = height;
        }
        
        // Frame definitions (for editor)
        struct FrameDef {
            std::string name;
            int x, y, w, h;         // Pixel rect
            float pivotX, pivotY;   // Normalized pivot (0-1)
        };
        
        void AddFrameDef(const FrameDef& frame);
        const std::vector<FrameDef>& GetFrameDefs() const { return m_FrameDefs; }
        
        // Animation clip management (for editor)
        void AddClip(const std::string& name, const Ref<AnimationClip>& clip);
        void RemoveClip(const std::string& name);
        void ClearClips();
        
        bool IsValid() const { return !m_Clips.empty() && !m_TexturePath.empty(); }
        
    private:
        std::string m_TexturePath;
        int m_TextureWidth = 0;
        int m_TextureHeight = 0;
        
        std::vector<FrameDef> m_FrameDefs;
        std::unordered_map<std::string, Ref<AnimationClip>> m_Clips;
        
        // Helper: convert pixel rect to UV
        Float2 PixelToUV(float pixelX, float pixelY) const;
    };

} // namespace SAGE
