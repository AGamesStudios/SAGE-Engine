#include "ShaderManager.h"

#include "Core/Logger.h"
#include "Core/ServiceLocator.h"
#include "Graphics/Core/Resources/Shader.h"
#include "Graphics/Core/Resources/Material.h"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>
#include <chrono>

namespace SAGE {

const std::vector<std::string> ShaderManager::s_RendererUniforms = {
    "u_ViewProjection",
    "u_View",
    "u_Projection",
    "u_Time",
    "u_Texture",
    "u_HasTexture",
    "u_TextureMode"
};

// ========== Instance Implementation ==========

void ShaderManager::Init() {
    if (m_Initialized) {
        SAGE_WARNING("ShaderManager::Init called multiple times on instance");
        return;
    }
    m_Shaders.clear();
    m_ShaderPaths.clear();
    m_Metadata.clear();
    m_Initialized = true;
}

void ShaderManager::Shutdown() {
    if (!m_Initialized) {
        return;
    }
    m_Shaders.clear();
    m_ShaderPaths.clear();
    m_Metadata.clear();
    m_Initialized = false;
}

bool ShaderManager::IsInitialized() const {
    return m_Initialized;
}

void ShaderManager::EnsureInitialized() {
    if (!m_Initialized) {
        Init();
    }
}

Ref<Shader> ShaderManager::Load(const std::string& name,
                                const std::string& vertexSource,
                                const std::string& fragmentSource) {
    EnsureInitialized();

    if (name.empty()) {
        SAGE_WARNING("ShaderManager::Load called with empty shader name");
    }

    if (auto it = m_Shaders.find(name); it != m_Shaders.end()) {
        return it->second;
    }

    auto shader = CreateRef<Shader>(vertexSource, fragmentSource);
    if (!shader || !shader->IsValid()) {
        SAGE_ERROR("ShaderManager failed to create shader '{}'", name);
        return nullptr;
    }

    m_ShaderPaths.erase(name);
    m_Shaders[name] = shader;
    ShaderMeta meta; meta.name=name; meta.vertexPath="<memory>"; meta.fragmentPath="<memory>"; m_Metadata[name]=meta;
    return shader;
}

Ref<Shader> ShaderManager::LoadFromFile(const std::string& name,
                                         const std::string& vertexPath,
                                         const std::string& fragmentPath) {
    EnsureInitialized();

    if (name.empty()) {
        SAGE_WARNING("ShaderManager::LoadFromFile called with empty shader name");
        return nullptr;
    }

    // Read vertex shader
    std::ifstream vertexFile(vertexPath);
    if (!vertexFile.is_open()) {
        SAGE_ERROR("ShaderManager failed to open vertex shader file: {}", vertexPath);
        return nullptr;
    }
    std::stringstream vertexStream;
    vertexStream << vertexFile.rdbuf();
    std::string vertexSource = vertexStream.str();
    vertexFile.close();

    // Read fragment shader
    std::ifstream fragmentFile(fragmentPath);
    if (!fragmentFile.is_open()) {
        SAGE_ERROR("ShaderManager failed to open fragment shader file: {}", fragmentPath);
        return nullptr;
    }
    std::stringstream fragmentStream;
    fragmentStream << fragmentFile.rdbuf();
    std::string fragmentSource = fragmentStream.str();
    fragmentFile.close();

    // Load shader using existing method
    Ref<Shader> oldShader;
    if (auto existing = m_Shaders.find(name); existing != m_Shaders.end()) {
        oldShader = existing->second;
    }

    auto shader = CreateRef<Shader>(vertexSource, fragmentSource);
    if (!shader || !shader->IsValid()) {
        SAGE_ERROR("ShaderManager failed to create shader '{}' from files", name);
        return nullptr;
    }

    m_Shaders[name] = shader;
    m_ShaderPaths[name] = {vertexPath, fragmentPath};
    ShaderMeta meta; meta.name=name; meta.vertexPath=vertexPath; meta.fragmentPath=fragmentPath; m_Metadata[name]=meta; UpdateTimestamps(m_Metadata[name]);
    if (oldShader) {
        MaterialLibrary::ReplaceShader(oldShader, shader);
    }
    SAGE_INFO("ShaderManager loaded shader '{}' from files (vertex: {}, fragment: {})", 
              name, vertexPath, fragmentPath);

    return shader;
}

Ref<Shader> ShaderManager::Get(const std::string& name) {
    if (!m_Initialized) {
        return nullptr;
    }

    if (auto it = m_Shaders.find(name); it != m_Shaders.end()) {
        return it->second;
    }
    // return fallback if set
    return m_Fallback;    
}

void ShaderManager::Remove(const std::string& name) {
    if (!m_Initialized) {
        return;
    }
    m_Shaders.erase(name);
    m_ShaderPaths.erase(name);
    m_Metadata.erase(name);
}

void ShaderManager::Clear() {
    if (!m_Initialized) {
        return;
    }
    m_Shaders.clear();
    m_ShaderPaths.clear();
    m_Metadata.clear();
}

const std::vector<std::string>& ShaderManager::GetRendererUniformNames() const {
    return s_RendererUniforms;
}

bool ShaderManager::ReloadShader(const std::string& name) {
    if (!m_Initialized) {
        SAGE_WARNING("ShaderManager::ReloadShader: Manager not initialized");
        return false;
    }

    auto it = m_Shaders.find(name);
    if (it == m_Shaders.end()) {
        SAGE_WARNING("ShaderManager::ReloadShader: Shader '{}' not found", name);
        return false;
    }

    auto pathIt = m_ShaderPaths.find(name);
    if (pathIt == m_ShaderPaths.end()) {
        SAGE_WARNING("ShaderManager::ReloadShader: No file paths stored for shader '{}'", name);
        return false;
    }
    const auto& paths = pathIt->second;
    // Read vertex
    std::ifstream vFile(paths.vertexPath);
    if (!vFile.is_open()) {
        SAGE_ERROR("ShaderManager::ReloadShader: Failed to open vertex shader file: {}", paths.vertexPath);
        return false;
    }
    std::stringstream vss; vss << vFile.rdbuf(); std::string vSrc = vss.str(); vFile.close();
    // Read fragment
    std::ifstream fFile(paths.fragmentPath);
    if (!fFile.is_open()) {
        SAGE_ERROR("ShaderManager::ReloadShader: Failed to open fragment shader file: {}", paths.fragmentPath);
        return false;
    }
    std::stringstream fss; fss << fFile.rdbuf(); std::string fSrc = fss.str(); fFile.close();

    auto newShader = CreateRef<Shader>(vSrc, fSrc);
    if (!newShader || !newShader->IsValid()) {
        SAGE_ERROR("ShaderManager::ReloadShader: Failed to compile shader '{}'", name);
        return false;
    }
    auto oldShader = it->second;
    m_Shaders[name] = newShader;
    if (auto metaIt = m_Metadata.find(name); metaIt != m_Metadata.end()) { UpdateTimestamps(metaIt->second); metaIt->second.lastError.clear(); }
    MaterialLibrary::ReplaceShader(oldShader, newShader);
    SAGE_INFO("ShaderManager::ReloadShader: Successfully reloaded shader '{}'", name);
    return true;
}

bool ShaderManager::ReloadFromMeta(const std::string& name, ShaderMeta& meta) {
    if (meta.vertexPath.empty() || meta.fragmentPath.empty()) return false;
    std::ifstream vFile(meta.vertexPath); if (!vFile.is_open()) { SAGE_WARNING("ReloadFromMeta: cannot open vertex '{}': {}", name, meta.vertexPath); return false; }
    std::stringstream vss; vss << vFile.rdbuf(); std::string vSrc = vss.str(); vFile.close();
    std::ifstream fFile(meta.fragmentPath); if (!fFile.is_open()) { SAGE_WARNING("ReloadFromMeta: cannot open fragment '{}': {}", name, meta.fragmentPath); return false; }
    std::stringstream fss; fss << fFile.rdbuf(); std::string fSrc = fss.str(); fFile.close();
    auto newShader = CreateRef<Shader>(vSrc, fSrc);
    if (!newShader || !newShader->IsValid()) {
        meta.lastError = "Compile/link failure on hot reload";
        UpdateTimestamps(meta);
        SAGE_ERROR("Hot reload failed for shader '{}'", name);
        return false;
    }
    auto old = m_Shaders[name];
    m_Shaders[name] = newShader;
    UpdateTimestamps(meta); meta.lastError.clear();
    MaterialLibrary::ReplaceShader(old, newShader);
    SAGE_INFO("Hot reloaded shader '{}'", name);
    return true;
}

std::vector<std::string> ShaderManager::PollAndReloadChanged() {
    std::vector<std::string> reloaded;
    if (!m_Initialized) return reloaded;
    for (auto& [name, meta] : m_Metadata) {
        if (meta.vertexPath == "<memory>" || meta.fragmentPath == "<memory>") continue;
        auto CurTime = [&](const std::string& p)->std::uint64_t {
            if (p.empty()) return 0; std::error_code ec; auto ft = std::filesystem::last_write_time(p, ec); if (ec) return 0; auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ft - decltype(ft)::clock::now() + std::chrono::system_clock::now()); return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(sctp.time_since_epoch()).count()); };
        std::uint64_t vNow = CurTime(meta.vertexPath);
        std::uint64_t fNow = CurTime(meta.fragmentPath);
        bool changed = (vNow != 0 && vNow > meta.lastWriteTimeVertex) || (fNow != 0 && fNow > meta.lastWriteTimeFragment);
        if (!changed) continue;
        if (ReloadFromMeta(name, meta)) reloaded.push_back(name);
    }
    return reloaded;
}

// ===== Extended API =====
void ShaderManager::AddTags(const std::string& name, const std::vector<std::string>& tags) {
    if (auto it=m_Metadata.find(name); it!=m_Metadata.end()) {
        for (const auto& t: tags) if(!t.empty()) it->second.tags.insert(t);
    }
}

std::vector<std::string> ShaderManager::GetTags(const std::string& name) const {
    if (auto it=m_Metadata.find(name); it!=m_Metadata.end()) {
        return std::vector<std::string>(it->second.tags.begin(), it->second.tags.end());
    }
    return {};
}

std::vector<std::string> ShaderManager::ListShaders() const {
    std::vector<std::string> out; out.reserve(m_Shaders.size());
    for (auto& kv : m_Shaders) out.push_back(kv.first); return out;
}

std::vector<std::string> ShaderManager::ListByTag(const std::string& tag) const {
    std::vector<std::string> out; for (auto& kv : m_Metadata){ if (kv.second.tags.count(tag)) out.push_back(kv.first);} return out;
}

const ShaderManager::ShaderMeta* ShaderManager::GetMeta(const std::string& name) const {
    if (auto it=m_Metadata.find(name); it!=m_Metadata.end()) return &it->second; return nullptr;
}

static std::uint64_t FileWriteTime(const std::string& path){
    std::error_code ec; auto ftime = std::filesystem::last_write_time(path, ec); if (ec) return 0; auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()); return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(sctp.time_since_epoch()).count());
}

void ShaderManager::UpdateTimestamps(ShaderMeta& meta){
    if (!meta.vertexPath.empty() && meta.vertexPath!="<memory>") meta.lastWriteTimeVertex = FileWriteTime(meta.vertexPath);
    if (!meta.fragmentPath.empty() && meta.fragmentPath!="<memory>") meta.lastWriteTimeFragment = FileWriteTime(meta.fragmentPath);
    if (!meta.geometryPath.empty()) meta.lastWriteTimeGeometry = FileWriteTime(meta.geometryPath);
    if (!meta.computePath.empty()) meta.lastWriteTimeCompute = FileWriteTime(meta.computePath);
}

} // namespace SAGE
