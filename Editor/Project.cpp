// Manifest-aware project implementation for scene management.
#include "Project.h"

#include <Core/Logger.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <system_error>

namespace SAGE {
namespace Editor {

namespace fs = std::filesystem;

namespace {

std::string GenerateRandomSceneId() {
    static thread_local std::mt19937_64 rng{[] {
        std::random_device rd;
        std::seed_seq seed{rd(), rd(), rd(), rd()};
        return std::mt19937_64(seed);
    }()};

    std::uniform_int_distribution<uint64_t> dist;
    const uint64_t hi = dist(rng);
    const uint64_t lo = dist(rng);

    std::ostringstream oss;
    oss << std::hex << std::nouppercase << std::setw(16) << std::setfill('0') << hi
        << std::setw(16) << std::setfill('0') << lo;
    return oss.str();
}

bool IsPathInside(const fs::path& base, const fs::path& target) {
    std::error_code ec;
    fs::path canonicalBase = fs::weakly_canonical(base, ec);
    if (ec) {
        return false;
    }
    fs::path canonicalTarget = fs::weakly_canonical(target, ec);
    if (ec) {
        return false;
    }

    auto baseIt = canonicalBase.begin();
    auto targetIt = canonicalTarget.begin();
    for (; baseIt != canonicalBase.end() && targetIt != canonicalTarget.end(); ++baseIt, ++targetIt) {
        if (*baseIt != *targetIt) {
            return false;
        }
    }
    return baseIt == canonicalBase.end();
}

} // namespace

// ===== Static helpers =====

std::string Project::NormalizeRelativePath(const fs::path& path) {
    fs::path normalized = path.lexically_normal();
    return normalized.generic_string();
}

std::time_t Project::FileTimeToTimeT(const fs::file_time_type& fileTime) {
    if (fileTime == fs::file_time_type{}) {
        return 0;
    }

    using namespace std::chrono;
    const auto nowFile = fs::file_time_type::clock::now();
    const auto sysNow = system_clock::now();
    const auto delta = fileTime - nowFile;
    const auto sysTime = sysNow + duration_cast<system_clock::duration>(delta);
    return system_clock::to_time_t(sysTime);
}

std::string Project::FormatTimestamp(std::time_t value) {
    if (value <= 0) {
        return {};
    }

    std::tm tm{};
#if defined(_WIN32)
    if (gmtime_s(&tm, &value) != 0) {
        return {};
    }
#else
    if (gmtime_r(&value, &tm) == nullptr) {
        return {};
    }
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::time_t Project::ParseTimestamp(const std::string& value) {
    if (value.empty()) {
        return 0;
    }

    std::tm tm{};
    std::istringstream iss(value);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (iss.fail()) {
        return 0;
    }

#if defined(_WIN32)
    return _mkgmtime(&tm);
#else
    return timegm(&tm);
#endif
}

// ===== Internal helpers =====

Project::SceneInfo* Project::FindSceneById(const std::string& id) {
    if (id.empty()) {
        return nullptr;
    }

    for (auto& scene : m_Scenes) {
        if (scene.id == id) {
            return &scene;
        }
    }
    return nullptr;
}

const Project::SceneInfo* Project::FindSceneById(const std::string& id) const {
    if (id.empty()) {
        return nullptr;
    }

    for (const auto& scene : m_Scenes) {
        if (scene.id == id) {
            return &scene;
        }
    }
    return nullptr;
}

Project::SceneInfo* Project::FindSceneByRelativePath(const std::string& relativePath) {
    if (relativePath.empty()) {
        return nullptr;
    }

    for (auto& scene : m_Scenes) {
        if (scene.relativePath == relativePath) {
            return &scene;
        }
    }
    return nullptr;
}

const Project::SceneInfo* Project::FindSceneByRelativePath(const std::string& relativePath) const {
    if (relativePath.empty()) {
        return nullptr;
    }

    for (const auto& scene : m_Scenes) {
        if (scene.relativePath == relativePath) {
            return &scene;
        }
    }
    return nullptr;
}

std::string Project::CreateSceneId() const {
    std::string candidate;
    do {
        candidate = GenerateRandomSceneId();
    } while (FindSceneById(candidate) != nullptr);
    return candidate;
}

void Project::EnsureSceneIds() {
    for (auto& scene : m_Scenes) {
        if (scene.id.empty()) {
            scene.id = CreateSceneId();
            m_SceneManifestDirty = true;
        }
    }
}

// ===== Scene manifest I/O =====

bool Project::SaveSceneManifestInternal(const fs::path& manifestPath) const {
    if (manifestPath.empty()) {
        return false;
    }

    try {
        if (auto parent = manifestPath.parent_path(); !parent.empty()) {
            std::error_code ec;
            fs::create_directories(parent, ec);
        }

        json manifest;
        manifest["version"] = 1;

        json scenesJson = json::array();
        for (const auto& scene : m_Scenes) {
            json entry;
            entry["id"] = scene.id;
            entry["name"] = scene.name;
            entry["path"] = scene.relativePath;
            entry["includedInBuild"] = scene.includedInBuild;

            const std::string timestamp = FormatTimestamp(scene.lastModifiedUtc);
            if (!timestamp.empty()) {
                entry["lastModified"] = timestamp;
            }
            scenesJson.push_back(std::move(entry));
        }

        manifest["scenes"] = std::move(scenesJson);

        std::ofstream file(manifestPath, std::ios::trunc);
        if (!file.is_open()) {
            SAGE_ERROR("Failed to write scene manifest: {}", manifestPath.string());
            return false;
        }

        file << manifest.dump(4);
        if (!file.good()) {
            SAGE_ERROR("Failed to flush scene manifest: {}", manifestPath.string());
            return false;
        }

        m_SceneManifestDirty = false;
        return true;
    } catch (const std::exception& e) {
        SAGE_ERROR("Exception while saving scene manifest '{}': {}", manifestPath.string(), e.what());
        return false;
    }
}

bool Project::LoadSceneManifest(const fs::path& manifestPath) {
    std::error_code ec;
    if (manifestPath.empty() || !fs::exists(manifestPath, ec)) {
        return false;
    }

    try {
        std::ifstream file(manifestPath);
        if (!file.is_open()) {
            SAGE_ERROR("Failed to open scene manifest: {}", manifestPath.string());
            return false;
        }

        json data;
        file >> data;
        if (!data.is_object()) {
            SAGE_WARNING("Scene manifest malformed: {}", manifestPath.string());
            return false;
        }

        const json* scenesPtr = data.contains("scenes") && data["scenes"].is_array()
                                    ? &data["scenes"]
                                    : nullptr;
        if (!scenesPtr) {
            SAGE_WARNING("Scene manifest missing 'scenes' array: {}", manifestPath.string());
            return false;
        }

        m_Scenes.clear();
        for (const auto& sceneJson : *scenesPtr) {
            if (!sceneJson.is_object()) {
                continue;
            }

            SceneInfo info;
            info.id = sceneJson.value("id", "");
            info.name = sceneJson.value("name", "Unnamed Scene");
            info.relativePath = NormalizeRelativePath(sceneJson.value("path", ""));
            info.includedInBuild = sceneJson.value("includedInBuild", true);
            info.lastModifiedUtc = ParseTimestamp(sceneJson.value("lastModified", std::string{}));

            if (info.relativePath.empty()) {
                continue;
            }
            if (info.id.empty()) {
                info.id = CreateSceneId();
            }
            m_Scenes.push_back(std::move(info));
        }

        m_SceneManifestDirty = false;
        return true;
    } catch (const std::exception& e) {
        SAGE_ERROR("Exception while loading scene manifest '{}': {}", manifestPath.string(), e.what());
        return false;
    }
}

void Project::LoadScenesFromProjectData(const json& projectData) {
    m_Scenes.clear();
    if (!projectData.contains("scenes") || !projectData["scenes"].is_array()) {
        return;
    }

    for (const auto& sceneJson : projectData["scenes"]) {
        if (!sceneJson.is_object()) {
            continue;
        }

        SceneInfo info;
        info.id = sceneJson.value("id", "");
        info.name = sceneJson.value("name", "Unnamed Scene");
        info.relativePath = NormalizeRelativePath(sceneJson.value("path", ""));
        info.includedInBuild = sceneJson.value("includedInBuild", true);
        info.lastModifiedUtc = ParseTimestamp(sceneJson.value("lastModified", std::string{}));

        if (info.relativePath.empty()) {
            continue;
        }
        if (info.id.empty()) {
            info.id = CreateSceneId();
        }
        m_Scenes.push_back(std::move(info));
    }

    m_SceneManifestDirty = true;
}

bool Project::ScanScenesDirectory(const fs::path& scenesDir) {
    std::error_code ec;
    if (scenesDir.empty() || !fs::exists(scenesDir, ec)) {
        return false;
    }

    bool added = false;
    for (auto it = fs::recursive_directory_iterator(scenesDir, ec); !ec && it != fs::recursive_directory_iterator(); ++it) {
        if (!it->is_regular_file()) {
            continue;
        }
        const auto ext = it->path().extension();
        if (ext != ".sscene" && ext != ".json") {
            continue;
        }

        const fs::path relative = fs::relative(it->path(), GetProjectDirectory(), ec);
        if (ec) {
            continue;
        }

        SceneInfo info;
        info.relativePath = NormalizeRelativePath(relative);
        info.name = it->path().stem().string();
        info.id = CreateSceneId();
        info.includedInBuild = true;
        info.lastModifiedUtc = FileTimeToTimeT(fs::last_write_time(it->path(), ec));
        m_Scenes.push_back(std::move(info));
        added = true;
    }

    if (added) {
        m_SceneManifestDirty = true;
    }
    return added;
}

void Project::RefreshSceneMetadata() {
    if (!IsLoaded()) {
        return;
    }

    const fs::path projectDir = GetProjectDirectory();
    for (auto& scene : m_Scenes) {
        std::error_code ec;
        fs::path absolute = projectDir / scene.relativePath;
        if (!fs::exists(absolute, ec)) {
            continue;
        }

        const std::time_t timestamp = FileTimeToTimeT(fs::last_write_time(absolute, ec));
        if (timestamp != 0 && timestamp != scene.lastModifiedUtc) {
            scene.lastModifiedUtc = timestamp;
            m_SceneManifestDirty = true;
        }
    }
}

// ===== Public API =====

bool Project::CreateNew(const std::string& projectPath, const std::string& projectName) {
    try {
        fs::path projectDir(projectPath);

        if (!CreateProjectStructure(projectDir, projectName)) {
            return false;
        }

        fs::path sceneFile = projectDir / "Scenes" / "MainScene.sscene";

        json sceneData;
        sceneData["sceneVersion"] = 2;
        sceneData["defaultNameCounter"] = 1;
        sceneData["entities"] = json::array();

        {
            std::ofstream file(sceneFile, std::ios::trunc);
            if (!file.is_open()) {
                SAGE_ERROR("Failed to create default scene: {}", sceneFile.string());
                return false;
            }
            file << sceneData.dump(4);
        }

        const fs::path projectFile = projectDir / (projectName + ".sageproject");

        Project newProject;
        newProject.m_Name = projectName;
        newProject.m_ProjectPath = projectFile.string();
        newProject.m_Version = 2;
        newProject.m_SceneManifestRelativePath = "Scenes/SceneManifest.json";

        SceneInfo defaultScene;
        defaultScene.name = "MainScene";
        defaultScene.relativePath = "Scenes/MainScene.sscene";
        defaultScene.includedInBuild = true;

        std::error_code tsEc;
        defaultScene.lastModifiedUtc = Project::FileTimeToTimeT(fs::last_write_time(sceneFile, tsEc));
        if (defaultScene.lastModifiedUtc == 0) {
            defaultScene.lastModifiedUtc = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
        defaultScene.id = newProject.CreateSceneId();

        newProject.m_Scenes.push_back(defaultScene);
        newProject.m_StartupScene = defaultScene.relativePath;
        newProject.m_StartupSceneId = defaultScene.id;
        newProject.m_SceneManifestDirty = true;

        if (!newProject.SaveToFile(projectFile.string())) {
            SAGE_ERROR("Failed to save project file: {}", projectFile.string());
            return false;
        }

        SAGE_INFO("Project created successfully: {}", projectFile.string());
        return true;

    } catch (const std::exception& e) {
        SAGE_ERROR("Exception while creating project: {}", e.what());
        return false;
    }
}

bool Project::Load(const std::string& projectFilePath) {
    return LoadFromFile(projectFilePath);
}

bool Project::Save() const {
    if (m_ProjectPath.empty()) {
        SAGE_ERROR("Cannot save project: no project path set");
        return false;
    }
    return SaveToFile(m_ProjectPath);
}

std::filesystem::path Project::GetProjectDirectory() const {
    if (m_ProjectPath.empty()) {
        return {};
    }
    return fs::path(m_ProjectPath).parent_path();
}

std::filesystem::path Project::GetScenesDirectory() const {
    auto base = GetProjectDirectory();
    if (base.empty()) {
        return {};
    }
    return base / "Scenes";
}

std::filesystem::path Project::GetAssetsDirectory() const {
    auto base = GetProjectDirectory();
    if (base.empty()) {
        return {};
    }
    return base / "Assets";
}

void Project::AddScene(const SceneInfo& scene) {
    if (scene.relativePath.empty()) {
        SAGE_WARNING("Attempted to add scene with empty path");
        return;
    }

    SceneInfo copy = scene;
    copy.relativePath = NormalizeRelativePath(fs::path(copy.relativePath));
    if (copy.id.empty() || FindSceneById(copy.id)) {
        copy.id = CreateSceneId();
    }

    if (auto* existing = FindSceneByRelativePath(copy.relativePath)) {
        *existing = copy;
    } else {
        m_Scenes.push_back(copy);
    }
    m_SceneManifestDirty = true;
}

void Project::RemoveScene(const std::string& relativePath) {
    const std::string normalized = NormalizeRelativePath(fs::path(relativePath));
    const auto beforeSize = m_Scenes.size();

    m_Scenes.erase(std::remove_if(m_Scenes.begin(), m_Scenes.end(), [&](const SceneInfo& info) {
        return info.relativePath == normalized;
    }), m_Scenes.end());

    if (m_Scenes.size() != beforeSize) {
        m_SceneManifestDirty = true;
        if (m_StartupScene == normalized) {
            m_StartupScene.clear();
            m_StartupSceneId.clear();
            if (!m_Scenes.empty()) {
                m_StartupScene = m_Scenes.front().relativePath;
                m_StartupSceneId = m_Scenes.front().id;
            }
        }
    }
}

const Project::SceneInfo* Project::GetStartupScene() const {
    if (!m_StartupSceneId.empty()) {
        if (const auto* scene = FindSceneById(m_StartupSceneId)) {
            return scene;
        }
    }

    if (!m_StartupScene.empty()) {
        if (const auto* scene = FindSceneByRelativePath(m_StartupScene)) {
            return scene;
        }
    }

    return nullptr;
}

void Project::SetStartupScene(const std::string& relativePath) {
    const std::string normalized = NormalizeRelativePath(fs::path(relativePath));
    if (auto* scene = FindSceneByRelativePath(normalized)) {
        m_StartupScene = scene->relativePath;
        m_StartupSceneId = scene->id;
        m_SceneManifestDirty = true;
    } else {
        SAGE_WARNING("Cannot set startup scene: {} not registered", relativePath);
    }
}

bool Project::RegisterSceneFile(const fs::path& absolutePath, const std::string& displayName) {
    if (!IsLoaded()) {
        return false;
    }

    const fs::path projectDir = GetProjectDirectory();
    std::error_code ec;
    fs::path canonical = fs::weakly_canonical(absolutePath, ec);
    if (ec) {
        canonical = absolutePath.lexically_normal();
    }

    if (!IsPathInside(projectDir, canonical)) {
        SAGE_WARNING("Scene '{}' is outside of project directory and will not be registered", canonical.string());
        return false;
    }

    fs::path relative = fs::relative(canonical, projectDir, ec);
    if (ec) {
        SAGE_WARNING("Failed to compute relative path for scene '{}': {}", canonical.string(), ec.message());
        return false;
    }

    const std::string normalized = NormalizeRelativePath(relative);
    SceneInfo* existing = FindSceneByRelativePath(normalized);

    std::string name = displayName;
    if (name.empty()) {
        name = canonical.stem().string();
    }

    std::time_t timestamp = FileTimeToTimeT(fs::last_write_time(canonical, ec));
    if (timestamp == 0) {
        timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }

    bool modified = false;
    if (existing) {
        if (existing->name != name) {
            existing->name = name;
            modified = true;
        }
        if (existing->lastModifiedUtc != timestamp) {
            existing->lastModifiedUtc = timestamp;
            modified = true;
        }
        if (existing->id.empty()) {
            existing->id = CreateSceneId();
            modified = true;
        }
    } else {
        SceneInfo info;
        info.relativePath = normalized;
        info.name = name;
        info.id = CreateSceneId();
        info.includedInBuild = true;
        info.lastModifiedUtc = timestamp;
        m_Scenes.push_back(std::move(info));
        modified = true;
    }

    if (modified) {
        m_SceneManifestDirty = true;
        if (m_StartupScene.empty()) {
            m_StartupScene = normalized;
            if (auto* scene = FindSceneByRelativePath(normalized)) {
                m_StartupSceneId = scene->id;
            }
        }
    }

    return modified;
}

bool Project::SaveSceneManifest() const {
    if (!IsLoaded()) {
        return false;
    }

    fs::path manifestPath = GetProjectDirectory() / m_SceneManifestRelativePath;
    return SaveSceneManifestInternal(manifestPath);
}

// ===== Persistence =====

bool Project::SaveToFile(const std::string& path) const {
    try {
        json projectData;
        projectData["version"] = std::max(m_Version, 2);
        projectData["name"] = m_Name;
        projectData["startupScene"] = m_StartupScene;
        projectData["startupSceneId"] = m_StartupSceneId;
        projectData["sceneManifest"] = m_SceneManifestRelativePath;

        json scenesArray = json::array();
        for (const auto& scene : m_Scenes) {
            json entry;
            entry["id"] = scene.id;
            entry["name"] = scene.name;
            entry["path"] = scene.relativePath;
            entry["includedInBuild"] = scene.includedInBuild;

            const std::string timestamp = FormatTimestamp(scene.lastModifiedUtc);
            if (!timestamp.empty()) {
                entry["lastModified"] = timestamp;
            }

            scenesArray.push_back(std::move(entry));
        }
        projectData["scenes"] = std::move(scenesArray);

        std::ofstream file(path, std::ios::trunc);
        if (!file.is_open()) {
            SAGE_ERROR("Failed to open project file for writing: {}", path);
            return false;
        }

        file << projectData.dump(4);
        if (!file.good()) {
            SAGE_ERROR("Failed to write project file: {}", path);
            return false;
        }

        fs::path manifestPath = fs::path(path).parent_path() / m_SceneManifestRelativePath;
        if (!SaveSceneManifestInternal(manifestPath)) {
            SAGE_WARNING("Project saved but failed to update scene manifest: {}", manifestPath.string());
        }

        return true;
    } catch (const std::exception& e) {
        SAGE_ERROR("Exception while saving project: {}", e.what());
        return false;
    }
}

bool Project::LoadFromFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            SAGE_ERROR("Failed to open project file: {}", path);
            return false;
        }

        json projectData;
        file >> projectData;

        m_Version = projectData.value("version", 1);
        m_Name = projectData.value("name", "Untitled");
        m_StartupScene = NormalizeRelativePath(projectData.value("startupScene", ""));
        m_StartupSceneId = projectData.value("startupSceneId", "");
        m_ProjectPath = path;
        m_SceneManifestRelativePath = projectData.value("sceneManifest", std::string("Scenes/SceneManifest.json"));

        if (m_SceneManifestRelativePath.empty()) {
            m_SceneManifestRelativePath = "Scenes/SceneManifest.json";
        }

        bool loadedManifest = false;
        if (!m_ProjectPath.empty()) {
            fs::path manifestPath = GetProjectDirectory() / m_SceneManifestRelativePath;
            loadedManifest = LoadSceneManifest(manifestPath);
        }

        if (!loadedManifest) {
            LoadScenesFromProjectData(projectData);
            if (m_Scenes.empty()) {
                ScanScenesDirectory(GetScenesDirectory());
            }
        }

        EnsureSceneIds();
        RefreshSceneMetadata();

        if (!m_StartupSceneId.empty()) {
            if (const auto* scene = FindSceneById(m_StartupSceneId)) {
                m_StartupScene = scene->relativePath;
            }
        } else if (!m_StartupScene.empty()) {
            if (const auto* scene = FindSceneByRelativePath(m_StartupScene)) {
                m_StartupSceneId = scene->id;
            }
        } else if (!m_Scenes.empty()) {
            m_StartupScene = m_Scenes.front().relativePath;
            m_StartupSceneId = m_Scenes.front().id;
        }

        if (m_SceneManifestDirty) {
            SaveSceneManifest();
        }

        SAGE_INFO("Project loaded: {}", m_Name);
        return true;

    } catch (const std::exception& e) {
        SAGE_ERROR("Exception while loading project: {}", e.what());
        return false;
    }
}

bool Project::CreateProjectStructure(const std::filesystem::path& projectDir, const std::string& projectName) {
    try {
        if (!fs::exists(projectDir)) {
            fs::create_directories(projectDir);
            SAGE_INFO("Created project directory: {}", projectDir.string());
        }

        fs::create_directories(projectDir / "Scenes");
        fs::create_directories(projectDir / "Assets");
        fs::create_directories(projectDir / "Assets" / "Textures");
        fs::create_directories(projectDir / "Assets" / "Scripts");
        fs::create_directories(projectDir / "Assets" / "Audio");

        SAGE_INFO("Project structure created for: {}", projectName);
        return true;

    } catch (const std::exception& e) {
        SAGE_ERROR("Failed to create project structure: {}", e.what());
        return false;
    }
}

} // namespace Editor
} // namespace SAGE
