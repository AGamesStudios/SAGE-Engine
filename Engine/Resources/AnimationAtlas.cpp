#include "AnimationAtlas.h"
#include "Core/Logger.h"
#include <json/json.hpp>
#include <fstream>
#include <algorithm>

using json = nlohmann::json;

namespace SAGE {

bool AnimationAtlas::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        SAGE_ERROR("AnimationAtlas::LoadFromFile - failed to open: {}", filepath);
        return false;
    }
    
    json j;
    file >> j;
    
    // Parse texture metadata
    if (j.contains("texture")) {
        m_TexturePath = j["texture"].get<std::string>();
    }
    if (j.contains("textureWidth")) {
        m_TextureWidth = j["textureWidth"].get<int>();
    }
    if (j.contains("textureHeight")) {
        m_TextureHeight = j["textureHeight"].get<int>();
    }
    
    // Parse frame definitions
    if (j.contains("frames")) {
        for (const auto& frameJson : j["frames"]) {
            FrameDef frame;
            frame.name = frameJson.value("name", "");
            frame.x = frameJson.value("x", 0);
            frame.y = frameJson.value("y", 0);
            frame.w = frameJson.value("w", 0);
            frame.h = frameJson.value("h", 0);
            frame.pivotX = frameJson.value("pivotX", 0.5f);
            frame.pivotY = frameJson.value("pivotY", 0.5f);
            
            m_FrameDefs.push_back(frame);
        }
    }
    
    // Parse animations
    if (j.contains("animations")) {
        for (const auto& animJson : j["animations"]) {
            std::string animName = animJson.value("name", "");
            if (animName.empty()) continue;
            
            auto clip = CreateRef<AnimationClip>(animName);
            
            // Parse playback mode
            std::string loopMode = animJson.value("loop", "true");
            if (loopMode == "pingpong") {
                clip->SetPlayMode(AnimationPlayMode::PingPong);
            } else if (loopMode == "once" || loopMode == "false") {
                clip->SetPlayMode(AnimationPlayMode::Once);
            } else if (loopMode == "reverse") {
                clip->SetPlayMode(AnimationPlayMode::LoopReverse);
            } else {
                clip->SetPlayMode(AnimationPlayMode::Loop);
            }
            
            // Parse frame rate
            float fps = animJson.value("fps", 10.0f);
            clip->SetFrameRate(fps);
            
            // Parse frame sequence
            if (animJson.contains("frames")) {
                for (const auto& frameNameJson : animJson["frames"]) {
                    std::string frameName = frameNameJson.get<std::string>();
                    
                    // Find frame definition by name
                    auto it = std::find_if(m_FrameDefs.begin(), m_FrameDefs.end(),
                        [&frameName](const FrameDef& def) { return def.name == frameName; });
                    
                    if (it == m_FrameDefs.end()) {
                        SAGE_WARNING("AnimationAtlas - frame '{}' not found in definitions", frameName);
                        continue;
                    }
                    
                    // Convert pixel rect to UVs
                    Float2 uvMin = PixelToUV(static_cast<float>(it->x), static_cast<float>(it->y));
                    Float2 uvMax = PixelToUV(static_cast<float>(it->x + it->w), static_cast<float>(it->y + it->h));
                    
                    // Create animation frame
                    AnimationFrame animFrame;
                    animFrame.uvMin = uvMin;
                    animFrame.uvMax = uvMax;
                    animFrame.pivot = Float2(it->pivotX, it->pivotY);
                    animFrame.duration = 1.0f / fps;
                    animFrame.pixelRect = Rect(it->x, it->y, it->w, it->h);
                    
                    clip->AddFrame(animFrame);
                }
            }
            
            if (clip->IsValid()) {
                m_Clips[animName] = clip;
                SAGE_INFO("AnimationAtlas - loaded clip '{}' with {} frames", animName, clip->GetFrameCount());
            }
        }
    }
    
    SAGE_INFO("AnimationAtlas loaded: {} ({} clips, {} frames)", 
              filepath, m_Clips.size(), m_FrameDefs.size());
    
    return IsValid();
}

bool AnimationAtlas::SaveToFile(const std::string& filepath) const {
    json j;
    
    // Save texture metadata
    j["texture"] = m_TexturePath;
    j["textureWidth"] = m_TextureWidth;
    j["textureHeight"] = m_TextureHeight;
    
    // Save frame definitions
    json framesArray = json::array();
    for (const auto& frame : m_FrameDefs) {
        json frameJson;
        frameJson["name"] = frame.name;
        frameJson["x"] = frame.x;
        frameJson["y"] = frame.y;
        frameJson["w"] = frame.w;
        frameJson["h"] = frame.h;
        frameJson["pivotX"] = frame.pivotX;
        frameJson["pivotY"] = frame.pivotY;
        framesArray.push_back(frameJson);
    }
    j["frames"] = framesArray;
    
    // Save animations
    json animsArray = json::array();
    for (const auto& [name, clip] : m_Clips) {
        json animJson;
        animJson["name"] = name;
        animJson["fps"] = clip->GetFrameRate();
        
        // Save loop mode
        switch (clip->GetPlayMode()) {
            case AnimationPlayMode::Loop:
                animJson["loop"] = "true";
                break;
            case AnimationPlayMode::PingPong:
                animJson["loop"] = "pingpong";
                break;
            case AnimationPlayMode::Once:
                animJson["loop"] = "once";
                break;
            case AnimationPlayMode::LoopReverse:
                animJson["loop"] = "reverse";
                break;
        }
        
        // Save frame sequence (by name)
        json frameNamesArray = json::array();
        for (size_t i = 0; i < clip->GetFrameCount(); ++i) {
            const AnimationFrame& frame = clip->GetFrame(i);
            
            // Find matching frame definition
            for (const auto& frameDef : m_FrameDefs) {
                Rect frameRect(frameDef.x, frameDef.y, frameDef.w, frameDef.h);
                if (frameRect == frame.pixelRect) {
                    frameNamesArray.push_back(frameDef.name);
                    break;
                }
            }
        }
        animJson["frames"] = frameNamesArray;
        
        animsArray.push_back(animJson);
    }
    j["animations"] = animsArray;
    
    // Write to file
    std::ofstream file(filepath);
    if (!file.is_open()) {
        SAGE_ERROR("AnimationAtlas::SaveToFile - failed to create: {}", filepath);
        return false;
    }
    
    file << j.dump(2); // Pretty print with 2-space indent
    SAGE_INFO("AnimationAtlas saved: {}", filepath);
    
    return true;
}

Ref<AnimationClip> AnimationAtlas::GetClip(const std::string& name) const {
    auto it = m_Clips.find(name);
    if (it == m_Clips.end()) {
        SAGE_WARNING("AnimationAtlas::GetClip - clip '{}' not found", name);
        return nullptr;
    }
    return it->second;
}

std::vector<std::string> AnimationAtlas::GetClipNames() const {
    std::vector<std::string> names;
    names.reserve(m_Clips.size());
    for (const auto& [name, clip] : m_Clips) {
        names.push_back(name);
    }
    return names;
}

void AnimationAtlas::AddFrameDef(const FrameDef& frame) {
    m_FrameDefs.push_back(frame);
}

void AnimationAtlas::AddClip(const std::string& name, const Ref<AnimationClip>& clip) {
    m_Clips[name] = clip;
}

void AnimationAtlas::RemoveClip(const std::string& name) {
    m_Clips.erase(name);
}

void AnimationAtlas::ClearClips() {
    m_Clips.clear();
    m_FrameDefs.clear();
}

Float2 AnimationAtlas::PixelToUV(float pixelX, float pixelY) const {
    if (m_TextureWidth <= 0 || m_TextureHeight <= 0) {
        std::fprintf(stderr, "AnimationAtlas::PixelToUV - Invalid texture dimensions: %d x %d\n", 
                     m_TextureWidth, m_TextureHeight);
        return Float2(0.0f, 0.0f);
    }
    
    return Float2(
        pixelX / static_cast<float>(m_TextureWidth),
        pixelY / static_cast<float>(m_TextureHeight)
    );
}

} // namespace SAGE
