#pragma once

#include <Core/Application.h>
#include <array>
#include <filesystem>
#include <memory>
#include <string>

#include "EditorConfig.h"
#include "EditorScene.h"
#include "SelectionContext.h"
#include "Project.h"
#include <deque>

// Forward declarations for new systems
namespace SAGE { namespace Editor { class UndoStack; } }

namespace SAGE {
namespace Editor {

class Viewport;
class HierarchyPanel;
class InspectorPanel;
class AssetBrowserPanel;
class ImageViewerWindow;
class GameWindow;

class EditorApplication : public Application {
public:
    EditorApplication();
    ~EditorApplication() override;

    void OnInit() override;
    void OnUpdate(float deltaTime);
    void OnRender();
    void OnShutdown();
    void HandleGlobalShortcuts();
    void RenderStatusBar(float deltaTime);
    void RenderNotifications();
    void OpenImageViewer(const std::string& imagePath);

private:
    static constexpr std::size_t ScenePathCapacity = 512;
    void RenderMenuBar();
    void RenderToolbar();
    void RenderDockSpace();
    void RenderPanels();
    void RenderTilemapToolWindow();
    void RenderSplitters(float topOffset, float availableHeight, float sidebarWidth,
        float hierarchyHeight, float inspectorHeight, bool assetBrowserVisible,
        float assetBrowserHeight, float viewportPosX, float viewportWidth);
    void SetActiveScene(std::unique_ptr<EditorScene> scene);
    void RenderSceneDialogs();
    bool LoadSceneFromPath(const std::string& path);
    bool SaveSceneToPath(const std::string& path);
    void CreateDefaultScene();
    void PreparePathBuffer(std::array<char, ScenePathCapacity>& buffer, const std::string& path) const;
    void ShowSceneStatus(const std::string& message, bool isError);
    void UpdateSceneStatusTimer(float deltaTime);
    static std::string NormalizeScenePath(const std::string& rawPath);
    void RenderStatusOverlay();
    void RequestClose();
    void CheckUnsavedChanges();
    void RenderHelpWindows();
    void AddRecentProject(const std::string& projectPath);
    void LoadRecentProjectsFromConfig();
    void StoreRecentProjectsToConfig();
    
    // Project management
    void RenderProjectDialogs();
    void ShowNewProjectDialog();
    void ShowOpenProjectDialog();
    bool CreateNewProject(const std::string& path, const std::string& name);
    bool OpenProject(const std::string& projectPath);
    void CloseProject();
    std::string OpenFolderDialog(const std::string& title);
    std::string OpenFileDialog(const std::string& title, const std::string& filter);
    
    // Template object creation
    void CreateEmptyObject();
    void CreateSpriteObject();
    void CreateCameraObject();

    std::unique_ptr<Viewport> m_Viewport;
    std::unique_ptr<HierarchyPanel> m_HierarchyPanel;
    std::unique_ptr<InspectorPanel> m_InspectorPanel;
    std::unique_ptr<AssetBrowserPanel> m_AssetBrowserPanel;
    std::unique_ptr<ImageViewerWindow> m_ImageViewer;
    std::unique_ptr<GameWindow> m_GameWindow;

    bool m_ShowViewport = true;
    bool m_ShowHierarchy = true;
    bool m_ShowInspector = true;
    bool m_ShowAssetBrowser = true;
    bool m_TilemapToolOpen = false;
    bool m_ShowImageViewer = false;
    bool m_ShowAboutWindow = false;
    bool m_ShowHelpWindow = false;
    bool m_ShowGameWindow = false;

    EditorConfig m_Config;
    std::filesystem::path m_ConfigPath;
    EditorScene* m_EditorScene = nullptr; // Non-owning pointer - SceneStack owns the memory
    SelectionContext m_Selection;
    std::string m_CurrentScenePath;
    bool m_SceneDirty = false;
    bool m_OpenScenePopupRequested = false;
    bool m_SaveScenePopupRequested = false;
    bool m_OpenSceneFocusPending = false;
    bool m_SaveSceneFocusPending = false;
    std::array<char, ScenePathCapacity> m_OpenScenePathBuffer{};
    std::array<char, ScenePathCapacity> m_SaveScenePathBuffer{};
    std::string m_OpenSceneError;
    std::string m_SaveSceneError;
    std::string m_SceneStatusMessage;
    bool m_SceneStatusIsError = false;
    float m_SceneStatusTimer = 0.0f;
    float m_FpsAccumulator = 0.0f;
    int m_FpsFrames = 0;
    float m_CurrentFPS = 0.0f;
    std::unique_ptr<UndoStack> m_Undo;
    float m_MenuBarHeight = 0.0f;
    float m_ToolbarHeight = 0.0f;
    bool m_DockspaceInitialized = false;
    
    // Project system
    Project m_Project;
    bool m_ShowNewProjectDialog = false;
    bool m_ShowOpenProjectDialog = false;
    std::array<char, ScenePathCapacity> m_NewProjectNameBuffer{};
    std::array<char, ScenePathCapacity> m_NewProjectPathBuffer{};
    std::string m_ProjectError;
    static constexpr std::size_t MaxRecentProjects = 10;
    std::deque<std::string> m_RecentProjects;
};

} // namespace Editor
} // namespace SAGE
