#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <ctime>
#include <nlohmann/json.hpp>

namespace SAGE {
namespace Editor {

using json = nlohmann::json;

class Project {
public:
    struct SceneInfo {
        std::string id;
        std::string name;
        std::string relativePath;  // Relative to project root
        bool includedInBuild = true;
        std::time_t lastModifiedUtc = 0; // UTC timestamp, 0 if unknown
    };

    Project() = default;
    ~Project() = default;

    // Create new project with folder structure
    static bool CreateNew(const std::string& projectPath, const std::string& projectName);
    
    // Load existing project
    bool Load(const std::string& projectFilePath);
    
    // Save project file
    bool Save() const;
    
    // Project info
    const std::string& GetName() const { return m_Name; }
    const std::string& GetPath() const { return m_ProjectPath; }
    std::filesystem::path GetProjectDirectory() const;
    std::filesystem::path GetScenesDirectory() const;
    std::filesystem::path GetAssetsDirectory() const;
    
    // Scenes management
    const std::vector<SceneInfo>& GetScenes() const { return m_Scenes; }
    void AddScene(const SceneInfo& scene);
    void RemoveScene(const std::string& relativePath);
    const SceneInfo* GetStartupScene() const;
    void SetStartupScene(const std::string& relativePath);
    bool RegisterSceneFile(const std::filesystem::path& absolutePath, const std::string& displayName = {});
    bool SaveSceneManifest() const;
    void RefreshSceneMetadata();
    const std::string& GetSceneManifestRelativePath() const { return m_SceneManifestRelativePath; }
    
    // Check if project is loaded
    bool IsLoaded() const { return !m_ProjectPath.empty(); }
    
private:
    std::string m_Name;
    std::string m_ProjectPath;  // Full path to .sageproject file
    std::string m_StartupScene;  // Relative path to startup scene
    std::string m_StartupSceneId; // Scene GUID of startup scene
    std::vector<SceneInfo> m_Scenes;
    int m_Version = 1;
    std::string m_SceneManifestRelativePath = "Scenes/SceneManifest.json";
    mutable bool m_SceneManifestDirty = false;
    
    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);
    static bool CreateProjectStructure(const std::filesystem::path& projectDir, const std::string& projectName);
    static std::string NormalizeRelativePath(const std::filesystem::path& path);
    SceneInfo* FindSceneByRelativePath(const std::string& relativePath);
    const SceneInfo* FindSceneByRelativePath(const std::string& relativePath) const;
    SceneInfo* FindSceneById(const std::string& id);
    const SceneInfo* FindSceneById(const std::string& id) const;
    bool LoadSceneManifest(const std::filesystem::path& manifestPath);
    bool SaveSceneManifestInternal(const std::filesystem::path& manifestPath) const;
    void LoadScenesFromProjectData(const nlohmann::json& projectData);
    bool ScanScenesDirectory(const std::filesystem::path& scenesDir);
    void EnsureSceneIds();
    std::string CreateSceneId() const;
    static std::time_t FileTimeToTimeT(const std::filesystem::file_time_type& fileTime);
    static std::time_t ParseTimestamp(const std::string& value);
    static std::string FormatTimestamp(std::time_t value);
};

} // namespace Editor
} // namespace SAGE
