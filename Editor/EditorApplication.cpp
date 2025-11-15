#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <shlobj.h>
#include <commdlg.h>
// Forward declare GLFW Win32 function to avoid header order issues
struct GLFWwindow;
extern "C" HWND glfwGetWin32Window(GLFWwindow* window);
#endif

#include "EditorApplication.h"
#include "Undo/EditorCommands.h"
#include "Notifications/NotificationBus.h"
#include "Viewport.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "AssetBrowserPanel.h"
#include "ImageViewerWindow.h"
#include "GameWindow.h"
#include "EditorConfig.h"
#include "Localization.h"
#include "FileUtils.h"

#include <Graphics/API/Renderer.h>
#include <ECS/Components/TilemapComponent.h>
#include <Core/Logger.h>
#include <Core/Window.h>
#include <Core/ResourceManager.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <array>
#include <algorithm>
#include <filesystem>
#include <system_error>
#include <cstdio>

namespace SAGE {
namespace Editor {

namespace {

constexpr float kSceneStatusDuration = 4.0f;

void ApplyPanelContexts(EditorScene* scene, SelectionContext* selection,
                        Viewport* viewport, HierarchyPanel* hierarchy, InspectorPanel* inspector,
                        AssetBrowserPanel* assetBrowser) {
    if (viewport) {
        viewport->SetContext(scene, selection);
    }
    if (hierarchy) {
        hierarchy->SetContext(scene, selection);
    }
    if (inspector) {
        inspector->SetContext(scene, selection);
    }
    if (assetBrowser) {
        assetBrowser->SetContext(scene, selection);
    }
}

} // namespace

EditorApplication::EditorApplication()
    : Application("SAGE Engine Editor")  // Application only takes name parameter
{
}

EditorApplication::~EditorApplication() = default;

std::string EditorApplication::NormalizeScenePath(const std::string& rawPath) {
    if (rawPath.empty()) {
        return {};
    }

    std::filesystem::path fsPath(rawPath);
    
    // If path ends with separator, add default filename
    if (rawPath.back() == '\\' || rawPath.back() == '/') {
        fsPath /= "NewScene.sscene";
    }
    
    // Add .sscene extension if no extension present
    if (!fsPath.has_extension()) {
        fsPath.replace_extension(".sscene");
    }

    std::error_code ec;
    std::filesystem::path canonical = std::filesystem::weakly_canonical(fsPath, ec);
    std::filesystem::path target = ec ? fsPath.lexically_normal() : canonical;
    target.make_preferred();
    return target.string();
}

void EditorApplication::SetActiveScene(std::unique_ptr<EditorScene> scene) {
    // Remove old scene from SceneStack if exists
    if (!GetSceneStack().Empty() && m_EditorScene) {
        PopScene(m_EditorScene);
    }
    
    m_Selection.Clear();
    
    // Push new scene to SceneStack and store raw pointer for editor use
    if (scene) {
        // Save raw pointer before transferring ownership to SceneStack
        m_EditorScene = scene.get();
        
        // Transfer ownership to SceneStack via unique_ptr cast to base Scene
        auto scenePtr = std::unique_ptr<Scene>(scene.release());
        PushScene(std::move(scenePtr));
        
        // Update panel contexts
        SelectionContext* selectionPtr = &m_Selection;
        ApplyPanelContexts(m_EditorScene, selectionPtr,
                           m_Viewport.get(), m_HierarchyPanel.get(), m_InspectorPanel.get(),
                           m_AssetBrowserPanel.get());
        
        m_EditorScene->RefreshSpriteTextures();
        m_EditorScene->ClearDirtyFlag();
    } else {
        m_EditorScene = nullptr;
    }
    
    m_SceneDirty = false;
}

void EditorApplication::PreparePathBuffer(std::array<char, ScenePathCapacity>& buffer, const std::string& path) const {
    buffer.fill('\0');
    if (!path.empty()) {
        std::snprintf(buffer.data(), buffer.size(), "%s", path.c_str());
    }
}

void EditorApplication::ShowSceneStatus(const std::string& message, bool isError) {
    m_SceneStatusMessage = message;
    m_SceneStatusIsError = isError;
    m_SceneStatusTimer = kSceneStatusDuration;
}

void EditorApplication::UpdateSceneStatusTimer(float deltaTime) {
    if (m_SceneStatusTimer <= 0.0f) {
        return;
    }
    m_SceneStatusTimer = std::max(0.0f, m_SceneStatusTimer - deltaTime);
    if (m_SceneStatusTimer == 0.0f) {
        m_SceneStatusMessage.clear();
    }
}

void EditorApplication::RequestClose() {
    bool hasUnsavedChanges = m_SceneDirty || (m_EditorScene && m_EditorScene->IsDirty());
    
    if (!hasUnsavedChanges) {
        Close(); // No unsaved changes, close immediately
        return;
    }
    
    // Open modal dialog for unsaved changes
    ImGui::OpenPopup("Unsaved Changes##CloseConfirm");
}

void EditorApplication::CheckUnsavedChanges() {
    // Kept for API compatibility - now just calls RequestClose
    RequestClose();
}

void EditorApplication::RenderStatusOverlay() {
    // Render unsaved changes dialog
    if (ImGui::BeginPopupModal("Unsaved Changes##CloseConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto& loc = Localization::Instance();
        ImGui::Text("You have unsaved changes. Do you want to save before closing?");
        ImGui::Separator();
        
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            if (!m_CurrentScenePath.empty()) {
                SaveSceneToPath(m_CurrentScenePath);
            } else {
                m_SaveScenePopupRequested = true;
            }
            ImGui::CloseCurrentPopup();
            Close(); // Call Application::Close()
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Don't Save", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            Close(); // Call Application::Close()
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    if (m_SceneStatusMessage.empty()) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 12.0f, io.DisplaySize.y - 12.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.85f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoFocusOnAppearing;

    if (ImGui::Begin("SceneStatusOverlay##Editor", nullptr, flags)) {
        ImVec4 color = m_SceneStatusIsError ? ImVec4(0.95f, 0.45f, 0.45f, 1.0f)
                                            : ImVec4(0.55f, 0.85f, 0.60f, 1.0f);
        ImGui::TextColored(color, "%s", m_SceneStatusMessage.c_str());
    }
    ImGui::End();
}

void EditorApplication::CreateDefaultScene() {
    auto scene = std::make_unique<EditorScene>();
    auto& loc = Localization::Instance();
    
    // Set scene name so it displays in status bar even without file path
    scene->SetName("Untitled Scene");
    
    ECS::Entity defaultEntity = scene->CreateEntity(loc.Get(TextID::Hierarchy_DefaultSpriteName));
    SetActiveScene(std::move(scene));
    if (ECS::IsValid(defaultEntity)) {
        m_Selection.selectedEntity = defaultEntity;
    }
    m_CurrentScenePath.clear();
    if (m_EditorScene) {
        m_EditorScene->MarkDirty();
    }
    m_SceneDirty = true;
    ShowSceneStatus(loc.Get(TextID::SceneStatus_NewSceneCreated), false);
}

bool EditorApplication::LoadSceneFromPath(const std::string& path) {
    auto& loc = Localization::Instance();
    if (path.empty()) {
        ShowSceneStatus(loc.Get(TextID::SceneStatus_SpecifyScenePath), true);
        return false;
    }

    const std::string normalized = NormalizeScenePath(path);

    auto scene = std::make_unique<EditorScene>();
    if (!scene->LoadFromFile(normalized)) {
        ShowSceneStatus(loc.Get(TextID::SceneStatus_LoadFailed), true);
        return false;
    }

    SetActiveScene(std::move(scene));
    m_CurrentScenePath = normalized;

    const auto& entities = m_EditorScene->GetEntities();
    if (!entities.empty()) {
        m_Selection.selectedEntity = entities.front().id;
    } else {
        m_Selection.Clear();
    }

    ShowSceneStatus(loc.Format(TextID::SceneStatus_Loaded, normalized), false);
    return true;
}

bool EditorApplication::SaveSceneToPath(const std::string& path) {
    auto& loc = Localization::Instance();
    if (!m_EditorScene) {
        ShowSceneStatus(loc.Get(TextID::SceneStatus_NoActiveScene), true);
        return false;
    }

    if (path.empty()) {
        ShowSceneStatus(loc.Get(TextID::SceneStatus_SpecifySavePath), true);
        return false;
    }

    const std::string normalized = NormalizeScenePath(path);

    if (!m_EditorScene->SaveToFile(normalized)) {
        ShowSceneStatus(loc.Get(TextID::SceneStatus_SaveFailed), true);
        return false;
    }

    m_CurrentScenePath = normalized;
    if (m_EditorScene) {
        m_EditorScene->ClearDirtyFlag();
    }
    m_SceneDirty = false;
    ShowSceneStatus(loc.Format(TextID::SceneStatus_Saved, normalized), false);
    
    // Show notification for successful save
    NotificationBus::Get().Notify("Scene saved successfully", NotificationLevel::Info);
    
    return true;
}

void EditorApplication::RenderSceneDialogs() {
    auto& loc = Localization::Instance();
    const std::string openScenePopupLabel = loc.Get(TextID::Dialog_OpenScene_Title) + "##OpenScene";
    const std::string saveScenePopupLabel = loc.Get(TextID::Dialog_SaveScene_Title) + "##SaveScene";

    if (m_OpenScenePopupRequested) {
        PreparePathBuffer(m_OpenScenePathBuffer, m_CurrentScenePath);
        m_OpenSceneError.clear();
        ImGui::OpenPopup(openScenePopupLabel.c_str());
        m_OpenScenePopupRequested = false;
        m_OpenSceneFocusPending = true;
    }

    if (m_SaveScenePopupRequested) {
        const std::string& defaultPath = m_CurrentScenePath.empty()
            ? (std::filesystem::current_path() / "scene.json").string()
            : m_CurrentScenePath;
        PreparePathBuffer(m_SaveScenePathBuffer, defaultPath);
        m_SaveSceneError.clear();
        ImGui::OpenPopup(saveScenePopupLabel.c_str());
        m_SaveScenePopupRequested = false;
        m_SaveSceneFocusPending = true;
    }

    if (ImGui::BeginPopupModal(openScenePopupLabel.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted(loc.Get(TextID::Dialog_OpenScene_Prompt).c_str());
        if (m_OpenSceneFocusPending) {
            ImGui::SetKeyboardFocusHere();
            m_OpenSceneFocusPending = false;
        }
        const ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
        bool submit = ImGui::InputText("##OpenScenePath", m_OpenScenePathBuffer.data(), ScenePathCapacity, inputFlags);
        if (!m_OpenSceneError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.45f, 0.45f, 1.0f));
            ImGui::TextWrapped("%s", m_OpenSceneError.c_str());
            ImGui::PopStyleColor();
        }
        if (ImGui::Button(loc.Get(TextID::Dialog_Open_Button).c_str()) || submit) {
            std::string rawPath = m_OpenScenePathBuffer.data();
            if (rawPath.empty()) {
                m_OpenSceneError = loc.Get(TextID::Dialog_PathEmptyError);
            } else {
                std::string normalized = NormalizeScenePath(rawPath);
                if (LoadSceneFromPath(normalized)) {
                    m_OpenSceneError.clear();
                    ImGui::CloseCurrentPopup();
                } else {
                    m_OpenSceneError = loc.Get(TextID::SceneStatus_LoadFailed);
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(loc.Get(TextID::Dialog_Cancel_Button).c_str())) {
            ImGui::CloseCurrentPopup();
            m_OpenSceneError.clear();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(saveScenePopupLabel.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted(loc.Get(TextID::Dialog_SaveScene_Prompt).c_str());
        if (m_SaveSceneFocusPending) {
            ImGui::SetKeyboardFocusHere();
            m_SaveSceneFocusPending = false;
        }
        const ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
        bool submit = ImGui::InputText("##SaveScenePath", m_SaveScenePathBuffer.data(), ScenePathCapacity, inputFlags);
        if (!m_SaveSceneError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.45f, 0.45f, 1.0f));
            ImGui::TextWrapped("%s", m_SaveSceneError.c_str());
            ImGui::PopStyleColor();
        }
        if (ImGui::Button(loc.Get(TextID::Dialog_Save_Button).c_str()) || submit) {
            std::string rawPath = m_SaveScenePathBuffer.data();
            if (rawPath.empty()) {
                m_SaveSceneError = loc.Get(TextID::Dialog_PathEmptyError);
            } else {
                std::string normalized = NormalizeScenePath(rawPath);
                if (SaveSceneToPath(normalized)) {
                    m_SaveSceneError.clear();
                    ImGui::CloseCurrentPopup();
                } else {
                    m_SaveSceneError = loc.Get(TextID::SceneStatus_SaveFailed);
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(loc.Get(TextID::Dialog_Cancel_Button).c_str())) {
            ImGui::CloseCurrentPopup();
            m_SaveSceneError.clear();
        }
        ImGui::EndPopup();
    }
}

void EditorApplication::RenderProjectDialogs() {
    // New Project Dialog
    if (m_ShowNewProjectDialog) {
        ImGui::OpenPopup("New Project##NewProjectDialog");
        m_ShowNewProjectDialog = false;
    }
    
    if (ImGui::BeginPopupModal("New Project##NewProjectDialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Create a new SAGE project");
        ImGui::Separator();
        
        ImGui::TextUnformatted("Project Name:");
        ImGui::InputText("##ProjectName", m_NewProjectNameBuffer.data(), ScenePathCapacity);
        
        ImGui::TextUnformatted("Project Location:");
        ImGui::InputText("##ProjectPath", m_NewProjectPathBuffer.data(), ScenePathCapacity);
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            std::string selectedPath = OpenFolderDialog("Select Project Location");
            if (!selectedPath.empty()) {
                PreparePathBuffer(m_NewProjectPathBuffer, selectedPath);
            }
        }
        
        if (!m_ProjectError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.45f, 0.45f, 1.0f));
            ImGui::TextWrapped("%s", m_ProjectError.c_str());
            ImGui::PopStyleColor();
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Create")) {
            std::string projectName = m_NewProjectNameBuffer.data();
            std::string projectPath = m_NewProjectPathBuffer.data();
            
            if (projectName.empty()) {
                m_ProjectError = "Project name cannot be empty";
            } else if (projectPath.empty()) {
                m_ProjectError = "Project location cannot be empty";
            } else {
                std::filesystem::path fullPath = std::filesystem::path(projectPath) / projectName;
                if (CreateNewProject(fullPath.string(), projectName)) {
                    m_ProjectError.clear();
                    ImGui::CloseCurrentPopup();
                } else {
                    m_ProjectError = "Failed to create project";
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_ProjectError.clear();
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // Open Project Dialog
    if (m_ShowOpenProjectDialog) {
        ImGui::OpenPopup("Open Project##OpenProjectDialog");
        m_ShowOpenProjectDialog = false;
    }
    
    if (ImGui::BeginPopupModal("Open Project##OpenProjectDialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Open existing SAGE project");
        ImGui::Separator();
        
        ImGui::TextUnformatted("Project File (.sageproject):");
        ImGui::InputText("##ProjectFilePath", m_NewProjectPathBuffer.data(), ScenePathCapacity);
        ImGui::SameLine();
        if (ImGui::Button("Browse...##OpenProject")) {
            std::string selectedFile = OpenFileDialog("Select Project File", "SAGE Project (*.sageproject)\0*.sageproject\0All Files (*.*)\0*.*\0");
            if (!selectedFile.empty()) {
                PreparePathBuffer(m_NewProjectPathBuffer, selectedFile);
            }
        }
        
        if (!m_ProjectError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.45f, 0.45f, 1.0f));
            ImGui::TextWrapped("%s", m_ProjectError.c_str());
            ImGui::PopStyleColor();
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Open")) {
            std::string projectPath = m_NewProjectPathBuffer.data();
            
            if (projectPath.empty()) {
                m_ProjectError = "Please select a project file";
            } else {
                if (OpenProject(projectPath)) {
                    m_ProjectError.clear();
                    ImGui::CloseCurrentPopup();
                } else {
                    m_ProjectError = "Failed to open project";
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_ProjectError.clear();
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

bool EditorApplication::CreateNewProject(const std::string& path, const std::string& name) {
    if (!Project::CreateNew(path, name)) {
        SAGE_ERROR("Failed to create project");
        return false;
    }
    
    // Load the newly created project
    std::filesystem::path projectFile = std::filesystem::path(path) / (name + ".sageproject");
    return OpenProject(projectFile.string());
}

bool EditorApplication::OpenProject(const std::string& projectPath) {
    if (!m_Project.Load(projectPath)) {
        SAGE_ERROR("Failed to load project: {}", projectPath);
        return false;
    }
    
    SAGE_INFO("Project opened: {}", m_Project.GetName());

    AddRecentProject(projectPath);
    
    // Load startup scene
    const auto* startupScene = m_Project.GetStartupScene();
    if (startupScene) {
        std::filesystem::path scenePath = m_Project.GetProjectDirectory() / startupScene->relativePath;
        LoadSceneFromPath(scenePath.string());
    } else {
        // No startup scene, create default
        CreateDefaultScene();
    }
    
    return true;
}

void EditorApplication::CloseProject() {
    // Check for unsaved changes
    CheckUnsavedChanges();
    
    m_Project = Project();
    SetActiveScene(nullptr);
    m_CurrentScenePath.clear();
}

void EditorApplication::CreateEmptyObject() {
    if (!m_EditorScene || !m_Undo) {
        return;
    }
    
    auto cmd = std::make_unique<CreateEntityCommand>("Empty");
    m_Undo->Push(std::move(cmd), *m_EditorScene);
    
    // Select the newly created entity
    if (!m_EditorScene->GetEntities().empty()) {
        m_Selection.selectedEntity = m_EditorScene->GetEntities().back().id;
        NotificationBus::Get().Notify("Empty object created", NotificationLevel::Info);
    }
}

void EditorApplication::CreateSpriteObject() {
    if (!m_EditorScene || !m_Undo) {
        return;
    }
    
    auto cmd = std::make_unique<CreateEntityCommand>("Sprite");
    m_Undo->Push(std::move(cmd), *m_EditorScene);
    
    // Get the newly created entity and add sprite component
    if (!m_EditorScene->GetEntities().empty()) {
        ECS::Entity newEntity = m_EditorScene->GetEntities().back().id;
        
        // Add default sprite component
        if (auto* sprite = m_EditorScene->GetSprite(newEntity)) {
            sprite->visible = true;
            sprite->flipX = false;
            sprite->flipY = false;
            sprite->layer = 0;
        }
        
        m_Selection.selectedEntity = newEntity;
        NotificationBus::Get().Notify("Sprite object created", NotificationLevel::Info);
    }
}

void EditorApplication::CreateCameraObject() {
    if (!m_EditorScene || !m_Undo) {
        return;
    }
    
    auto cmd = std::make_unique<CreateEntityCommand>("Camera");
    m_Undo->Push(std::move(cmd), *m_EditorScene);
    
    // Get the newly created entity
    if (!m_EditorScene->GetEntities().empty()) {
        ECS::Entity newEntity = m_EditorScene->GetEntities().back().id;
        
        // Set transform to have a reasonable default position
        if (auto* transform = m_EditorScene->GetTransform(newEntity)) {
            transform->position = Vector2(0.0f, 0.0f);
            transform->SetRotation(0.0f);
            transform->scale = Vector2(1.0f, 1.0f);
            transform->size = Vector2(96.0f, 54.0f); // Wide aspect so camera gizmo reads clearly
        }

        if (auto* sprite = m_EditorScene->GetSprite(newEntity)) {
            sprite->visible = false;
        }
        
        m_Selection.selectedEntity = newEntity;
        NotificationBus::Get().Notify("Camera object created", NotificationLevel::Info);
    }
}

std::string EditorApplication::OpenFolderDialog(const std::string& title) {
#ifdef _WIN32
    // Get native window handle
    HWND hwnd = glfwGetWin32Window(GetWindow().GetNativeWindow());
    
    // Use Windows folder picker dialog
    BROWSEINFOA bi = {};
    bi.hwndOwner = hwnd;
    bi.lpszTitle = title.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != nullptr) {
        char path[MAX_PATH];
        if (SHGetPathFromIDListA(pidl, path)) {
            CoTaskMemFree(pidl);
            return std::string(path);
        }
        CoTaskMemFree(pidl);
    }
#endif
    return "";
}

std::string EditorApplication::OpenFileDialog(const std::string& title, const std::string& filter) {
#ifdef _WIN32
    // Get native window handle
    HWND hwnd = glfwGetWin32Window(GetWindow().GetNativeWindow());
    
    char filename[MAX_PATH] = {};
    
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    
    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
#endif
    return "";
}

void EditorApplication::OnInit() {
#ifdef _WIN32
    // Initialize COM for Windows file dialogs
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
#endif

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Disabled: requires ImGui docking branch
    
    // Setup ImGui style - Use default ImGui dark style with improved borders
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    
    // Make borders slightly more visible
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // ResourceManager is singleton; ensure GPU resource loading enabled for editor session
    ResourceManager::Get().SetGpuLoadingEnabled(true);

    ImFont* loadedFont = nullptr;
    const std::array<std::filesystem::path, 4> fontCandidates = {
        std::filesystem::path("assets/fonts/Inter-Regular.ttf"),
        std::filesystem::path("assets/fonts/DejaVuSans.ttf"),
        std::filesystem::path("assets/fonts/Roboto-Regular.ttf"),
        std::filesystem::path(R"(C:/Windows/Fonts/segoeui.ttf)")
    };

    auto makeUtf8 = [](const std::filesystem::path& path) {
        const auto utf8 = path.u8string();
        return std::string(utf8.begin(), utf8.end());
    };

    for (const auto& candidate : fontCandidates) {
        if (candidate.empty()) {
            continue;
        }

        std::filesystem::path resolved = candidate;
        if (!resolved.is_absolute()) {
            resolved = std::filesystem::current_path() / resolved;
        }

        if (!std::filesystem::exists(resolved)) {
            continue;
        }

        const std::string fontPathUtf8 = makeUtf8(resolved);
        loadedFont = io.Fonts->AddFontFromFileTTF(fontPathUtf8.c_str(), 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
        if (loadedFont) {
            io.FontDefault = loadedFont;
            SAGE_INFO("Editor font loaded: {}", resolved.string());
            break;
        }
    }

    if (!loadedFont) {
        io.Fonts->AddFontDefault();
        SAGE_WARNING("Не удалось загрузить шрифт с поддержкой UTF-8. Используется шрифт ImGui по умолчанию.");
    }
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(GetWindow().GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    SAGE_INFO("ImGui initialized");
    
    // Create editor panels
    m_Viewport = std::make_unique<Viewport>();
    m_HierarchyPanel = std::make_unique<HierarchyPanel>();
    m_InspectorPanel = std::make_unique<InspectorPanel>();
    m_AssetBrowserPanel = std::make_unique<AssetBrowserPanel>();
    m_ImageViewer = std::make_unique<ImageViewerWindow>();
    m_GameWindow = std::make_unique<GameWindow>();
    m_Undo = std::make_unique<UndoStack>();
    
    // Set up asset browser callbacks
    if (m_AssetBrowserPanel) {
        m_AssetBrowserPanel->SetAssetDoubleClickedCallback([this](const std::string& path) {
            // Check if it's an image file
            if (FileUtils::IsImageFile(path)) {
                OpenImageViewer(path);
            }
        });
    }

    try {
        m_ConfigPath = std::filesystem::current_path() / "editor_config.json";
        m_Config.Load(m_ConfigPath.string());
    } catch (const std::exception& e) {
        SAGE_WARNING("Failed to resolve editor config path: {}", e.what());
    }

    Language initialLanguage = Localization::FromLanguageCode(m_Config.languageCode);
    Localization::Instance().SetLanguage(initialLanguage);
    m_Config.languageCode = std::string(Localization::GetLanguageCode(initialLanguage));

    LoadRecentProjectsFromConfig();

    if (m_Viewport) {
        m_Viewport->SetConfig(&m_Config);
    }
    if (m_AssetBrowserPanel) {
        m_AssetBrowserPanel->SetConfig(&m_Config);
    }

    // ALWAYS create a default scene - editor needs at least one scene to work
    // EditorScene now properly inherits from Scene and integrates with SceneStack
    CreateDefaultScene();
    SAGE_INFO("Default EditorScene created and pushed to SceneStack - editor is ready");

    // Show project dialog on startup if no project is loaded
    if (!m_Project.IsLoaded()) {
        m_ShowNewProjectDialog = true;
        std::string defaultPath = std::filesystem::current_path().string();
        PreparePathBuffer(m_NewProjectPathBuffer, defaultPath);
        m_NewProjectNameBuffer.fill('\0');
    }
}

void EditorApplication::OnUpdate(float deltaTime) {
    // Update viewport
    if (m_Viewport) {
        m_Viewport->Update(deltaTime);
    }
    
    // Check if EditorScene is dirty (don't call Update - SceneStack does it)
    if (m_EditorScene) {
        m_SceneDirty = m_EditorScene->IsDirty();
    } else {
        m_SceneDirty = false;
    }
    
    UpdateSceneStatusTimer(deltaTime);
    HandleGlobalShortcuts();
    NotificationBus::Get().Update(deltaTime);
    
    // FPS tracking
    m_FpsAccumulator += deltaTime;
    ++m_FpsFrames;
    if (m_FpsAccumulator >= 0.5f) { // update twice a second
        m_CurrentFPS = m_FpsFrames / m_FpsAccumulator;
        m_FpsAccumulator = 0.0f;
        m_FpsFrames = 0;
    }
}

void EditorApplication::OnRender() {
    Renderer::Clear(0.1f, 0.1f, 0.1f, 1.0f);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    RenderDockSpace();
    RenderMenuBar();
    RenderToolbar();
    RenderPanels();
    RenderTilemapToolWindow();
    RenderSceneDialogs();
    RenderProjectDialogs();
    RenderStatusOverlay();
    RenderStatusBar(ImGui::GetIO().DeltaTime);
    RenderNotifications();
    RenderHelpWindows();
    
    // Render Image Viewer
    if (m_ImageViewer && m_ShowImageViewer) {
        m_ImageViewer->Render(&m_ShowImageViewer);
    }

    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData) {
        SAGE_INFO("ImGui draw lists: {} vertices: {}", drawData->CmdListsCount, drawData->TotalVtxCount);
        Renderer::SetUIRenderCallback([drawData]() {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        });
    } else {
        Renderer::SetUIRenderCallback({});
    }
}
void EditorApplication::HandleGlobalShortcuts() {
    ImGuiIO& io = ImGui::GetIO();
    // Only process shortcuts when not typing in text inputs or when ImGui wants keyboard input
    if (io.WantTextInput || io.WantCaptureKeyboard) {
        return;
    }
    
    // F5 - Start Play Mode
    if (ImGui::IsKeyPressed(ImGuiKey_F5, false) && !ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
        if (m_EditorScene && m_EditorScene->GetPlayState() == EditorScene::PlayState::Stopped) {
            m_EditorScene->StartPlayMode();
            NotificationBus::Get().Notify("Play Mode Started", NotificationLevel::Info);
        }
    }
    
    // Shift+F5 - Stop Play Mode
    if (ImGui::IsKeyPressed(ImGuiKey_F5, false) && ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
        if (m_EditorScene && m_EditorScene->GetPlayState() != EditorScene::PlayState::Stopped) {
            m_EditorScene->StopPlayMode();
            NotificationBus::Get().Notify("Play Mode Stopped", NotificationLevel::Info);
        }
    }
    
    // Ctrl+S save
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        if (!m_CurrentScenePath.empty()) {
            SaveSceneToPath(m_CurrentScenePath);
        } else {
            m_SaveScenePopupRequested = true;
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
        m_ShowHelpWindow = true;
    }
    // Ctrl+O open
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_O, false)) {
        m_OpenScenePopupRequested = true;
    }
    // Ctrl+Shift+N create empty object
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyPressed(ImGuiKey_N, false)) {
        CreateEmptyObject();
    }
    // Ctrl+D duplicate selected entity via Undo system
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_D, false)) {
        if (m_EditorScene && m_Selection.HasSelection() && m_Undo) {
            auto cmd = std::make_unique<CreateEntityCommand>("Duplicate");
            m_Undo->Push(std::move(cmd), *m_EditorScene);
            ECS::Entity newEntity = m_EditorScene->GetEntities().back().id;
            if (auto* srcT = m_EditorScene->GetTransform(m_Selection.selectedEntity))
                if (auto* dstT = m_EditorScene->GetTransform(newEntity)) *dstT = *srcT;
            if (auto* srcS = m_EditorScene->GetSprite(m_Selection.selectedEntity))
                if (auto* dstS = m_EditorScene->GetSprite(newEntity)) *dstS = *srcS;
            m_Selection.selectedEntity = newEntity;
            NotificationBus::Get().Notify("Сущность дублирована", NotificationLevel::Info);
        }
    }
    // Delete key - delete entity
    if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
        if (m_EditorScene && m_Selection.HasSelection() && m_Undo) {
            auto cmd = std::make_unique<DeleteEntityCommand>(m_Selection.selectedEntity);
            m_Undo->Push(std::move(cmd), *m_EditorScene);
            m_Selection.Clear();
            NotificationBus::Get().Notify("Сущность удалена", NotificationLevel::Info);
        }
    }
    // Ctrl+Z / Ctrl+Y undo/redo
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        if (m_EditorScene && m_Undo && m_Undo->CanUndo()) { m_Undo->Undo(*m_EditorScene); NotificationBus::Get().Notify("Undo", NotificationLevel::Info); }
    }
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
        if (m_EditorScene && m_Undo && m_Undo->CanRedo()) { m_Undo->Redo(*m_EditorScene); NotificationBus::Get().Notify("Redo", NotificationLevel::Info); }
    }
}

void EditorApplication::RenderStatusBar(float /*deltaTime*/) {
    ImGuiIO& io = ImGui::GetIO();
    const float height = 26.0f;
    ImGui::SetNextWindowPos(ImVec2(0.0f, io.DisplaySize.y - height), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, height), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    if (ImGui::Begin("StatusBar##Editor", nullptr, flags)) {
        auto& loc = Localization::Instance();
        const size_t entityCount = m_EditorScene ? m_EditorScene->GetEntities().size() : 0;
        
        // Display scene name: if we have a file path, show it; otherwise show scene name
        std::string sceneName;
        if (!m_CurrentScenePath.empty()) {
            sceneName = m_CurrentScenePath;
        } else if (m_EditorScene) {
            sceneName = m_EditorScene->GetName();
        } else {
            sceneName = loc.Get(TextID::SceneLabel_NewPlaceholder);
        }
        
        std::string left = loc.Format(TextID::SceneLabel_Format, sceneName);
        if (m_SceneDirty) left += " *";
        
        // Align text vertically in center
        const float textHeight = ImGui::GetTextLineHeight();
        const float windowHeight = ImGui::GetWindowHeight();
        const float padding = ImGui::GetStyle().WindowPadding.y;
        ImGui::SetCursorPosY(padding + (windowHeight - textHeight - padding * 2.0f) * 0.5f);
        
        ImGui::TextUnformatted(left.c_str());
        ImGui::SameLine();
        ImGui::Text("| Entities: %zu", entityCount);
        ImGui::SameLine();
        ImGui::Text("| FPS: %.1f", m_CurrentFPS);
        ImGui::SameLine();
        ImGui::Text("| Lang: %s", Localization::GetLanguageCode(loc.GetLanguage()));
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorApplication::RenderNotifications() {
    const auto& items = NotificationBus::Get().GetItems();
    if (items.empty()) return;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(12.0f, io.DisplaySize.y - 140.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.4f);
    if (ImGui::Begin("Notifications##Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        for (const auto& n : items) {
            ImVec4 col;
            switch (n.level) {
            case NotificationLevel::Info: col = ImVec4(0.6f, 0.8f, 1.0f, 1.0f); break;
            case NotificationLevel::Warning: col = ImVec4(0.95f, 0.75f, 0.3f, 1.0f); break;
            case NotificationLevel::Error: col = ImVec4(0.95f, 0.4f, 0.4f, 1.0f); break;
            }
            ImGui::TextColored(col, "%s", n.message.c_str());
        }
    }
    ImGui::End();
}

void EditorApplication::OpenImageViewer(const std::string& imagePath) {
    if (m_ImageViewer) {
        m_ImageViewer->Open(imagePath);
        m_ShowImageViewer = true;
    }
}

void EditorApplication::AddRecentProject(const std::string& projectPath) {
    if (projectPath.empty()) {
        return;
    }

    std::filesystem::path fsPath(projectPath);
    std::error_code ec;
    std::filesystem::path normalized = std::filesystem::weakly_canonical(fsPath, ec);
    if (ec) {
        normalized = fsPath.lexically_normal();
    }
    normalized.make_preferred();
    const std::string normalizedStr = normalized.string();

    auto existing = std::find(m_RecentProjects.begin(), m_RecentProjects.end(), normalizedStr);
    if (existing != m_RecentProjects.end()) {
        m_RecentProjects.erase(existing);
    }
    m_RecentProjects.push_front(normalizedStr);
    while (m_RecentProjects.size() > MaxRecentProjects) {
        m_RecentProjects.pop_back();
    }

    StoreRecentProjectsToConfig();
}

void EditorApplication::LoadRecentProjectsFromConfig() {
    m_RecentProjects.clear();
    for (auto it = m_Config.recentProjects.rbegin(); it != m_Config.recentProjects.rend(); ++it) {
        const std::string& entry = *it;
        if (entry.empty()) {
            continue;
        }

        std::filesystem::path fsPath(entry);
        std::error_code ec;
        std::filesystem::path normalized = std::filesystem::weakly_canonical(fsPath, ec);
        if (ec) {
            normalized = fsPath.lexically_normal();
        }
        normalized.make_preferred();
        const std::string normalizedStr = normalized.string();

        auto existing = std::find(m_RecentProjects.begin(), m_RecentProjects.end(), normalizedStr);
        if (existing != m_RecentProjects.end()) {
            m_RecentProjects.erase(existing);
        }
        m_RecentProjects.push_front(normalizedStr);
    }
    StoreRecentProjectsToConfig();
}

void EditorApplication::StoreRecentProjectsToConfig() {
    m_Config.recentProjects.assign(m_RecentProjects.begin(), m_RecentProjects.end());
}

void EditorApplication::RenderHelpWindows() {
    if (m_ShowHelpWindow) {
        if (ImGui::Begin("Keyboard Shortcuts##Help", &m_ShowHelpWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Viewport & Scene");
            ImGui::Separator();
            ImGui::BulletText("Ctrl+S - %s", Localization::Instance().Get(TextID::Menu_SaveScene).c_str());
            ImGui::BulletText("Ctrl+Shift+S - %s", Localization::Instance().Get(TextID::Menu_SaveSceneAs).c_str());
            ImGui::BulletText("Ctrl+O - %s", Localization::Instance().Get(TextID::Menu_OpenScene).c_str());
            ImGui::BulletText("Ctrl+Shift+N - Create Empty Object");
            ImGui::BulletText("F1 - Toggle this help window");
            ImGui::Spacing();
            ImGui::TextUnformatted("Navigation");
            ImGui::Separator();
            ImGui::BulletText("Mouse Wheel - Zoom viewport");
            ImGui::BulletText("Right Mouse + Drag - Pan viewport");
            ImGui::BulletText("Gizmo Handles - Translate/Rotate/Scale");
            ImGui::Spacing();
            ImGui::TextWrapped("More documentation is available inside the project docs folder.");
        }
        ImGui::End();
    }

    if (m_ShowAboutWindow) {
        if (ImGui::Begin("About SAGE Editor", &m_ShowAboutWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("SAGE Engine Editor");
            ImGui::Separator();
            ImGui::Text("Version: %s", "Experimental");
            ImGui::Text("Build Date: %s", __DATE__ " " __TIME__);
            ImGui::Spacing();
            ImGui::TextWrapped("SAGE Engine Editor provides tools for creating scenes, managing assets, and configuring gameplay objects.");
            ImGui::Spacing();
            ImGui::TextWrapped("Documentation: docs/README.md");
        }
        ImGui::End();
    }
}

void EditorApplication::OnShutdown() {
    SetActiveScene(nullptr);

    m_Viewport.reset();
    m_HierarchyPanel.reset();
    m_InspectorPanel.reset();

    if (!m_ConfigPath.empty()) {
        StoreRecentProjectsToConfig();
        m_Config.Save(m_ConfigPath.string());
    }

    // Clear resource cache on shutdown (textures, etc.)
    ResourceManager::Get().ClearCache();
    
    // Shutdown ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

#ifdef _WIN32
    // Uninitialize COM
    CoUninitialize();
#endif
}

void EditorApplication::RenderMenuBar() {
    auto& loc = Localization::Instance();
    m_MenuBarHeight = ImGui::GetFrameHeight();
    if (ImGui::BeginMainMenuBar()) {
        m_MenuBarHeight = ImGui::GetWindowSize().y;
    auto separatorText = [](const char* label) {
#if defined(IMGUI_VERSION_NUM) && IMGUI_VERSION_NUM >= 18996
        ImGui::SeparatorText(label);
#else
        ImGui::Separator();
        ImGui::TextDisabled("%s", label);
#endif
    };
        if (ImGui::BeginMenu(loc.Get(TextID::Menu_File).c_str())) {
        separatorText("Project");
            if (ImGui::MenuItem("New Project...", "Ctrl+Shift+P")) {
                m_ShowNewProjectDialog = true;
                // Initialize with default path
                std::string defaultPath = std::filesystem::current_path().string();
                PreparePathBuffer(m_NewProjectPathBuffer, defaultPath);
                m_NewProjectNameBuffer.fill('\0');
            }
            if (ImGui::MenuItem("Open Project...", "Ctrl+Shift+O")) {
                m_ShowOpenProjectDialog = true;
                m_NewProjectPathBuffer.fill('\0');
            }
            if (ImGui::MenuItem("Close Project", nullptr, false, m_Project.IsLoaded())) {
                CloseProject();
            }

            const bool hasRecentProjects = !m_RecentProjects.empty();
            if (hasRecentProjects) {
                if (ImGui::BeginMenu(loc.Get(TextID::Menu_RecentProjects).c_str())) {
                    for (const auto& entry : m_RecentProjects) {
                        std::filesystem::path projectPath(entry);
                        std::string label = projectPath.filename().string();
                        if (label.empty()) {
                            label = entry;
                        }
                        if (ImGui::MenuItem(label.c_str())) {
                            if (!OpenProject(entry)) {
                                NotificationBus::Get().Notify("Failed to open project", NotificationLevel::Error);
                            }
                        }
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                            ImGui::SetTooltip("%s", entry.c_str());
                        }
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem(loc.Get(TextID::Menu_ClearRecent).c_str())) {
                        m_RecentProjects.clear();
                        StoreRecentProjectsToConfig();
                    }
                    ImGui::EndMenu();
                }
            } else {
                ImGui::BeginDisabled();
                std::string emptyLabel = loc.Get(TextID::Menu_RecentProjects) + std::string(" (empty)");
                ImGui::MenuItem(emptyLabel.c_str(), nullptr, false, false);
                ImGui::EndDisabled();
            }

            separatorText("Scene");
            if (ImGui::MenuItem(loc.Get(TextID::Menu_NewScene).c_str())) {
                CreateDefaultScene();
            }
            if (ImGui::MenuItem(loc.Get(TextID::Menu_OpenScene).c_str(), "Ctrl+O")) {
                m_OpenScenePopupRequested = true;
            }
            bool canSave = (m_EditorScene != nullptr);
            if (ImGui::MenuItem(loc.Get(TextID::Menu_SaveScene).c_str(), "Ctrl+S", false, canSave)) {
                if (!m_CurrentScenePath.empty()) {
                    SaveSceneToPath(m_CurrentScenePath);
                } else {
                    m_SaveScenePopupRequested = true;
                }
            }
            if (ImGui::MenuItem(loc.Get(TextID::Menu_SaveSceneAs).c_str(), "Ctrl+Shift+S", false, canSave)) {
                m_SaveScenePopupRequested = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem(loc.Get(TextID::Menu_Exit).c_str())) {
                RequestClose();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(loc.Get(TextID::Menu_View).c_str())) {
            ImGui::MenuItem(loc.Get(TextID::Menu_Viewport).c_str(), nullptr, &m_ShowViewport);
            ImGui::MenuItem(loc.Get(TextID::Menu_Hierarchy).c_str(), nullptr, &m_ShowHierarchy);
            ImGui::MenuItem(loc.Get(TextID::Menu_Inspector).c_str(), nullptr, &m_ShowInspector);
            ImGui::MenuItem("Asset Browser", nullptr, &m_ShowAssetBrowser);
            ImGui::MenuItem("Tilemap Editor", nullptr, &m_TilemapToolOpen);
            ImGui::Separator();
            
            // Game Window (not fully implemented)
            bool gameWindowDisabled = true;  // Temporarily disabled
            if (gameWindowDisabled) {
                ImGui::BeginDisabled();
            }
            if (ImGui::MenuItem("Game Window (WIP)", nullptr, &m_ShowGameWindow)) {
                if (m_GameWindow) {
                    if (m_ShowGameWindow) {
                        if (!m_GameWindow->IsOpen()) {
                            m_GameWindow->Create(800, 600);
                        }
                        m_GameWindow->Show();
                    } else {
                        m_GameWindow->Hide();
                    }
                }
            }
            if (gameWindowDisabled) {
                ImGui::EndDisabled();
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip("Game Window is not yet implemented.\nUse Play mode in Viewport instead.");
                }
            }
            ImGui::EndMenu();
        }

        // GameObject menu
        if (ImGui::BeginMenu("GameObject")) {
            if (ImGui::MenuItem("Create Empty", "Ctrl+Shift+N")) {
                CreateEmptyObject();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Sprite")) {
                CreateSpriteObject();
            }
            if (ImGui::MenuItem("Camera")) {
                CreateCameraObject();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(loc.Get(TextID::Menu_Language).c_str())) {
            const Language currentLanguage = loc.GetLanguage();
            const bool isEnglish = currentLanguage == Language::English;
            const bool isRussian = currentLanguage == Language::Russian;

            auto applyLanguage = [this](Language language) {
                auto& localization = Localization::Instance();
                if (localization.GetLanguage() == language) {
                    return;
                }
                localization.SetLanguage(language);
                m_Config.languageCode = std::string(Localization::GetLanguageCode(language));
                m_OpenSceneError.clear();
                m_SaveSceneError.clear();
                m_SceneStatusMessage.clear();
                m_SceneStatusTimer = 0.0f;
            };

            if (ImGui::MenuItem(loc.Get(TextID::Language_English).c_str(), nullptr, isEnglish, !isEnglish)) {
                applyLanguage(Language::English);
            }
            if (ImGui::MenuItem(loc.Get(TextID::Language_Russian).c_str(), nullptr, isRussian, !isRussian)) {
                applyLanguage(Language::Russian);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(loc.Get(TextID::Menu_Help).c_str())) {
            if (ImGui::MenuItem(loc.Get(TextID::Menu_HelpShortcuts).c_str(), "F1")) {
                m_ShowHelpWindow = true;
            }
            if (ImGui::MenuItem(loc.Get(TextID::Menu_HelpDocs).c_str())) {
                NotificationBus::Get().Notify("Open docs/README.md for detailed guides.", NotificationLevel::Info);
            }
            ImGui::Separator();
            if (ImGui::MenuItem(loc.Get(TextID::Menu_About).c_str())) {
                m_ShowAboutWindow = true;
            }
            ImGui::EndMenu();
        }

        // Display scene name in menu bar
        std::string sceneName;
        if (!m_CurrentScenePath.empty()) {
            sceneName = m_CurrentScenePath;
        } else if (m_EditorScene) {
            sceneName = m_EditorScene->GetName();
        } else {
            sceneName = loc.Get(TextID::SceneLabel_NewPlaceholder);
        }
        std::string sceneLabel = loc.Format(TextID::SceneLabel_Format, sceneName);
        if (m_SceneDirty) {
            sceneLabel += " *";
        }

        std::string projectLabel;
        if (m_Project.IsLoaded()) {
            projectLabel = "Project: " + m_Project.GetName();
        } else if (!m_RecentProjects.empty()) {
            projectLabel = "Project: <none>";
        } else {
            projectLabel = "Project: <none>";
        }

        float projectWidth = ImGui::CalcTextSize(projectLabel.c_str()).x;
        float sceneWidth = ImGui::CalcTextSize(sceneLabel.c_str()).x;
        float separatorWidth = ImGui::CalcTextSize(" | ").x;
        float totalWidth = projectWidth + separatorWidth + sceneWidth;
        float regionMax = ImGui::GetWindowContentRegionMax().x;
        float cursorY = ImGui::GetCursorPosY();
        ImGui::SameLine();
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), regionMax - totalWidth - 16.0f));
        ImGui::SetCursorPosY(cursorY);
        ImGui::TextUnformatted(projectLabel.c_str());
        ImGui::SameLine();
        ImGui::TextUnformatted("|");
        ImGui::SameLine();
        ImGui::TextUnformatted(sceneLabel.c_str());

        ImGui::EndMainMenuBar();
    }
}

void EditorApplication::RenderToolbar() {
    m_ToolbarHeight = 0.0f;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, m_MenuBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, ImGui::GetFrameHeightWithSpacing() + 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(16.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg]);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;

    if (ImGui::Begin("Toolbar##Editor", nullptr, flags)) {
        m_ToolbarHeight = ImGui::GetWindowSize().y;
        
        // === PLAY MODE CONTROLS ===
        if (m_EditorScene) {
            auto playState = m_EditorScene->GetPlayState();
            bool isPlaying = (playState == EditorScene::PlayState::Playing);
            bool isPaused = (playState == EditorScene::PlayState::Paused);
            bool isStopped = (playState == EditorScene::PlayState::Stopped);
            
            // Play Button (Green)
            if (isStopped) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 0.7f));
            }
            
            if (ImGui::Button(isPlaying ? "▶ Playing" : isPaused ? "▶ Paused" : "▶ Play")) {
                if (isStopped) {
                    m_EditorScene->StartPlayMode();
                }
            }
            ImGui::PopStyleColor(3);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Start game simulation (F5)");
            }
            
            // Pause Button (Yellow)
            ImGui::SameLine();
            ImGui::BeginDisabled(isStopped);
            if (isPaused) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.7f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.5f, 0.1f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.7f));
            }
            
            if (ImGui::Button(isPaused ? "⏸ Paused" : "⏸ Pause")) {
                m_EditorScene->PausePlayMode();
            }
            ImGui::PopStyleColor(3);
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Pause/Resume simulation");
            }
            
            // Stop Button (Red)
            ImGui::SameLine();
            ImGui::BeginDisabled(isStopped);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            
            if (ImGui::Button("⏹ Stop")) {
                m_EditorScene->StopPlayMode();
            }
            ImGui::PopStyleColor(3);
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Stop simulation and restore scene (Shift+F5)");
            }
            
            ImGui::SameLine();
            ImGui::TextUnformatted("|");
            ImGui::SameLine();
        }

        if (ImGui::Button("Tile Mapping")) {
            m_TilemapToolOpen = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Open the tilemap editor");
        }

        if (m_Viewport) {
            ImGui::SameLine();
            bool showGrid = m_Viewport->IsGridVisible();
            if (ImGui::Checkbox("Grid", &showGrid)) {
                m_Viewport->SetShowGrid(showGrid);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Toggle viewport grid overlay");
            }

            ImGui::SameLine();
            bool showAxes = m_Viewport->IsAxesVisible();
            if (ImGui::Checkbox("Axes", &showAxes)) {
                m_Viewport->SetShowAxes(showAxes);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Toggle axis guide overlay");
            }

            ImGui::SameLine();
            bool showGizmo = m_Viewport->AreGizmosVisible();
            if (ImGui::Checkbox("Gizmo", &showGizmo)) {
                m_Viewport->SetShowGizmos(showGizmo);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Show or hide transform gizmos");
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

void EditorApplication::RenderTilemapToolWindow() {
    if (!m_TilemapToolOpen) {
        return;
    }

    if (!ImGui::Begin("Tilemap Editor", &m_TilemapToolOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    auto markSceneDirty = [this]() {
        if (m_EditorScene) {
            m_EditorScene->MarkDirty();
            m_SceneDirty = true;
        }
    };

    if (!m_EditorScene) {
        ImGui::TextUnformatted("Load a scene to edit tilemaps.");
        ImGui::End();
        return;
    }

    if (!m_Selection.HasSelection()) {
        ImGui::TextUnformatted("Select an entity with a TilemapComponent.");
        ImGui::End();
        return;
    }

    ECS::TilemapComponent* tilemap = m_EditorScene->GetECS().GetRegistry().GetComponent<ECS::TilemapComponent>(m_Selection.selectedEntity);
    if (!tilemap) {
        ImGui::TextUnformatted("Selected entity has no TilemapComponent.");
        ImGui::End();
        return;
    }

    const EditorScene::EntityRecord* record = m_EditorScene->FindRecord(m_Selection.selectedEntity);
    const char* entityName = record ? record->name.c_str() : "Tilemap";

    ImGui::Text("Entity: %s", entityName);

    ImGui::Text("Map Size: %d x %d tiles", tilemap->mapWidth, tilemap->mapHeight);
    ImGui::Text("Tile Size: %d x %d px", tilemap->tileWidth, tilemap->tileHeight);
    ImGui::Text("Layers: %zu", tilemap->layers.size());
    ImGui::Separator();

    if (tilemap->layers.empty()) {
        ImGui::TextUnformatted("This tilemap has no layers to edit.");
    }

    for (size_t i = 0; i < tilemap->layers.size(); ++i) {
        auto& layer = tilemap->layers[i];
        ImGui::PushID(static_cast<int>(i));
        std::string label = layer.name.empty() ? ("Layer " + std::to_string(i)) : layer.name;
        if (ImGui::TreeNodeEx("LayerNode", ImGuiTreeNodeFlags_DefaultOpen, "%s", label.c_str())) {
            if (ImGui::Checkbox("Visible", &layer.visible)) {
                markSceneDirty();
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("Collision", &layer.collision)) {
                markSceneDirty();
            }
            if (ImGui::SliderFloat("Opacity", &layer.opacity, 0.0f, 1.0f)) {
                markSceneDirty();
            }
            if (ImGui::DragFloat2("Parallax Factor", &layer.parallaxFactor.x, 0.01f, 0.0f, 2.0f, "%.2f")) {
                markSceneDirty();
            }
            ImGui::Text("Size: %d x %d tiles (%d total)", layer.width, layer.height, static_cast<int>(layer.tiles.size()));
            if (layer.vboCached) {
                ImGui::TextColored(ImVec4(0.1f, 0.7f, 0.2f, 1.0f), "VBO Cached (ID: %u)", layer.vboID);
            } else {
                ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "VBO Not Cached");
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
        if (i + 1 < tilemap->layers.size()) {
            ImGui::Separator();
        }
    }

    ImGui::End();
}

void EditorApplication::RenderDockSpace() {
    // Disabled: requires ImGui docking branch
    return;
    
    /*
    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == 0) {
        return;
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("EditorDockspaceHost", nullptr, windowFlags)) {
        const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGuiID dockspaceID = ImGui::GetID("EditorDockspace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

        if (!m_DockspaceInitialized) {
            m_DockspaceInitialized = true;
            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

            const float displayWidth = std::max(1.0f, viewport->Size.x);
            const float displayHeight = std::max(1.0f, viewport->Size.y);

            const float sidebarRatio = std::clamp(m_Config.sidebarWidth / displayWidth, 0.15f, 0.5f);
            const float assetRatio = std::clamp(m_Config.assetBrowserHeight / displayHeight, 0.15f, 0.5f);

            ImGuiID dockMainID = dockspaceID;
            ImGuiID dockLeftID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, sidebarRatio, nullptr, &dockMainID);
            ImGuiID dockBottomID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, assetRatio, nullptr, &dockMainID);
            ImGuiID dockInspectorID = ImGui::DockBuilderSplitNode(dockLeftID, ImGuiDir_Down, 0.5f, nullptr, &dockLeftID);

            auto& loc = Localization::Instance();
            const std::string viewportLabel = loc.Get(TextID::Viewport_WindowTitle) + "##Viewport";
            const std::string hierarchyLabel = loc.Get(TextID::Hierarchy_WindowTitle) + "##Hierarchy";
            const std::string inspectorLabel = loc.Get(TextID::Inspector_WindowTitle) + "##Inspector";

            ImGui::DockBuilderDockWindow(viewportLabel.c_str(), dockMainID);
            ImGui::DockBuilderDockWindow("Asset Browser", dockBottomID);
            ImGui::DockBuilderDockWindow(hierarchyLabel.c_str(), dockLeftID);
            ImGui::DockBuilderDockWindow(inspectorLabel.c_str(), dockInspectorID);

            ImGui::DockBuilderFinish(dockspaceID);
        }
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    */
}

void EditorApplication::RenderPanels() {
    ImGuiIO& io = ImGui::GetIO();
    const float topOffset = m_MenuBarHeight + m_ToolbarHeight;
    const float statusBarHeight = 26.0f;
    const float availableHeight = std::max(1.0f, io.DisplaySize.y - topOffset - statusBarHeight);
    
    // Panel dimensions from config with safety checks
    const float sidebarWidth = std::clamp(m_Config.sidebarWidth, m_Config.minSidebarWidth, 
        std::max(m_Config.minSidebarWidth, io.DisplaySize.x - m_Config.minViewportSize - m_Config.padding * 3.0f));
    const float assetBrowserHeight = m_ShowAssetBrowser 
        ? std::clamp(m_Config.assetBrowserHeight, m_Config.minAssetBrowserHeight, 
            std::max(m_Config.minAssetBrowserHeight, availableHeight - m_Config.minViewportSize - m_Config.padding * 2.0f))
        : 0.0f;
    
    const float viewportWidth = std::max(m_Config.minViewportSize, io.DisplaySize.x - sidebarWidth - m_Config.padding * 3.0f);
    const float viewportHeight = std::max(m_Config.minViewportSize, availableHeight - assetBrowserHeight - (m_ShowAssetBrowser ? m_Config.padding * 2.0f : m_Config.padding));
    
    const float hierarchyRatio = std::clamp(m_Config.hierarchyHeightRatio, 0.2f, 0.8f);
    const float hierarchyHeight = std::max(m_Config.minPanelHeight, (availableHeight - m_Config.padding) * hierarchyRatio);
    const float inspectorHeight = std::max(m_Config.minPanelHeight, availableHeight - hierarchyHeight - m_Config.padding);

    // Render splitters FIRST (they should be behind panels)
    RenderSplitters(topOffset, availableHeight, sidebarWidth, hierarchyHeight, inspectorHeight,
        m_ShowAssetBrowser, assetBrowserHeight, m_Config.padding + sidebarWidth + m_Config.padding, viewportWidth);

    const ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    // Hierarchy Panel (top-left)
    if (m_ShowHierarchy && m_HierarchyPanel) {
        ImGui::SetNextWindowPos(ImVec2(m_Config.padding, topOffset + m_Config.padding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth, hierarchyHeight), ImGuiCond_Always);
        bool open = m_ShowHierarchy;
        m_HierarchyPanel->Render(&open, panelFlags, nullptr);
        m_ShowHierarchy = open;
    }

    // Inspector Panel (bottom-left)
    if (m_ShowInspector && m_InspectorPanel) {
        ImGui::SetNextWindowPos(ImVec2(m_Config.padding, topOffset + m_Config.padding + hierarchyHeight + m_Config.padding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth, inspectorHeight), ImGuiCond_Always);
        bool open = m_ShowInspector;
        m_InspectorPanel->Render(&open, panelFlags, nullptr);
        m_ShowInspector = open;
    }

    // Viewport (top-right)
    if (m_ShowViewport && m_Viewport) {
        const float viewportX = m_Config.padding + sidebarWidth + m_Config.padding;
        ImGui::SetNextWindowPos(ImVec2(viewportX, topOffset + m_Config.padding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(viewportWidth, viewportHeight), ImGuiCond_Always);
        ImGuiWindowFlags viewportFlags = panelFlags | ImGuiWindowFlags_NoScrollbar;
        m_Viewport->Render(&m_ShowViewport, viewportFlags, nullptr);
    }

    // Asset Browser (bottom-right)
    if (m_ShowAssetBrowser && m_AssetBrowserPanel) {
        const float assetX = m_Config.padding + sidebarWidth + m_Config.padding;
        const float assetY = topOffset + m_Config.padding + viewportHeight + m_Config.padding;
        ImGui::SetNextWindowPos(ImVec2(assetX, assetY), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(viewportWidth, assetBrowserHeight), ImGuiCond_Always);
        if (ImGui::Begin("Asset Browser", &m_ShowAssetBrowser, panelFlags)) {
            m_AssetBrowserPanel->Render();
        }
        ImGui::End();
    }
    
    // Game Window (floating, auto-opens in play mode)
    // Update Game Window (native window)
    if (m_EditorScene && m_EditorScene->IsPlaying()) {
        if (!m_ShowGameWindow) {
            m_ShowGameWindow = true;
            if (!m_GameWindow->IsOpen()) {
                m_GameWindow->Create(800, 600);
            }
            m_GameWindow->Show();
        }
        
        // Update game window each frame during play mode
        if (m_GameWindow->IsOpen()) {
            m_GameWindow->Update(m_EditorScene);
        }
    }
    else if (m_ShowGameWindow && m_GameWindow->IsOpen()) {
        // Stop mode - hide game window
        m_GameWindow->Hide();
        m_ShowGameWindow = false;
    }

    
    // Draw visible borders around panels manually
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    const ImU32 borderColor = ImGui::GetColorU32(ImVec4(0.43f, 0.43f, 0.50f, 0.50f));
    const float borderThickness = 1.0f;
    
    // Hierarchy border
    if (m_ShowHierarchy) {
        ImVec2 min(m_Config.padding, topOffset + m_Config.padding);
        ImVec2 max(m_Config.padding + sidebarWidth, topOffset + m_Config.padding + hierarchyHeight);
        drawList->AddRect(min, max, borderColor, 0.0f, 0, borderThickness);
    }
    
    // Inspector border
    if (m_ShowInspector) {
        ImVec2 min(m_Config.padding, topOffset + m_Config.padding + hierarchyHeight + m_Config.padding);
        ImVec2 max(m_Config.padding + sidebarWidth, topOffset + m_Config.padding + hierarchyHeight + m_Config.padding + inspectorHeight);
        drawList->AddRect(min, max, borderColor, 0.0f, 0, borderThickness);
    }
    
    // Viewport border
    if (m_ShowViewport) {
        const float viewportX = m_Config.padding + sidebarWidth + m_Config.padding;
        ImVec2 min(viewportX, topOffset + m_Config.padding);
        ImVec2 max(viewportX + viewportWidth, topOffset + m_Config.padding + viewportHeight);
        drawList->AddRect(min, max, borderColor, 0.0f, 0, borderThickness);
    }
    
    // Asset Browser border
    if (m_ShowAssetBrowser) {
        const float assetX = m_Config.padding + sidebarWidth + m_Config.padding;
        const float assetY = topOffset + m_Config.padding + viewportHeight + m_Config.padding;
        ImVec2 min(assetX, assetY);
        ImVec2 max(assetX + viewportWidth, assetY + assetBrowserHeight);
        drawList->AddRect(min, max, borderColor, 0.0f, 0, borderThickness);
    }
}

void EditorApplication::RenderSplitters(float topOffset, float availableHeight, float sidebarWidth,
    float hierarchyHeight, float inspectorHeight, bool assetBrowserVisible,
    float assetBrowserHeight, float viewportPosX, float viewportWidth) {
    ImGuiIO& io = ImGui::GetIO();
    const float overlayHeight = std::max(0.0f, io.DisplaySize.y - topOffset);
    if (overlayHeight <= 0.0f) {
        return;
    }

    // Create an invisible overlay window that only handles splitter interactions
    // Must be rendered BEFORE other windows so they receive input priority
    ImGui::SetNextWindowPos(ImVec2(0.0f, topOffset));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, overlayHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    // Key: NoInputs flag removed, but we'll only place InvisibleButtons in specific areas
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowBgAlpha(0.0f);
    bool splitterWindowOpen = true;
    
    if (!ImGui::Begin("##SplittersLayer", &splitterWindowOpen, flags)) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float splitterThickness = 6.0f;

    // Vertical splitter between sidebar and viewport
    const float verticalX = m_Config.padding + sidebarWidth + m_Config.padding * 0.5f - splitterThickness * 0.5f;
    ImVec2 verticalPos(verticalX, topOffset + m_Config.padding);
    ImVec2 verticalSize(splitterThickness, overlayHeight - m_Config.padding * 2.0f);
    
    ImGui::SetCursorScreenPos(verticalPos);
    ImGui::InvisibleButton("##SidebarSplitter", verticalSize);
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    if (hovered || active) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        ImU32 color = ImGui::GetColorU32(active ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f)
                                               : ImVec4(1.0f, 1.0f, 1.0f, 0.35f));
        ImVec2 verticalMax(verticalPos.x + verticalSize.x, verticalPos.y + verticalSize.y);
        drawList->AddRectFilled(verticalPos, verticalMax, color);
    }
    if (active) {
        const float maxSidebar = std::max(m_Config.minSidebarWidth,
            io.DisplaySize.x - m_Config.minViewportSize - m_Config.padding * 3.0f);
        m_Config.sidebarWidth = std::clamp(m_Config.sidebarWidth + io.MouseDelta.x,
            m_Config.minSidebarWidth, maxSidebar);
    }
    // Save config when splitter is released
    static bool wasDraggingVertical = false;
    if (wasDraggingVertical && !active) {
        m_Config.Save(m_ConfigPath.string());
        wasDraggingVertical = false;
    }
    if (active) wasDraggingVertical = true;

    // Horizontal splitter inside sidebar (Hierarchy/Inspector)
    if (hierarchyHeight > 0.0f && inspectorHeight > 0.0f && availableHeight > 0.0f) {
        ImVec2 horizontalPos(m_Config.padding,
            topOffset + m_Config.padding + hierarchyHeight + m_Config.padding * 0.5f - splitterThickness * 0.5f);
        ImVec2 horizontalSize(sidebarWidth, splitterThickness);
        
        ImGui::SetCursorScreenPos(horizontalPos);
        ImGui::InvisibleButton("##HierarchySplitter", horizontalSize);
        hovered = ImGui::IsItemHovered();
        active = ImGui::IsItemActive();
        if (hovered || active) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            ImU32 color = ImGui::GetColorU32(active ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f)
                                                   : ImVec4(1.0f, 1.0f, 1.0f, 0.35f));
            ImVec2 horizontalMax(horizontalPos.x + horizontalSize.x, horizontalPos.y + horizontalSize.y);
            drawList->AddRectFilled(horizontalPos, horizontalMax, color);
        }
        if (active) {
            float newHierarchy = hierarchyHeight + io.MouseDelta.y;
            const float minHierarchy = m_Config.minPanelHeight;
            const float maxHierarchy = std::max(minHierarchy, availableHeight - m_Config.minPanelHeight);
            newHierarchy = std::clamp(newHierarchy, minHierarchy, maxHierarchy);
            m_Config.hierarchyHeightRatio = std::clamp(newHierarchy / availableHeight, 0.05f, 0.95f);
        }
        // Save config when splitter is released
        static bool wasDraggingHorizontal = false;
        if (wasDraggingHorizontal && !active) {
            m_Config.Save(m_ConfigPath.string());
            wasDraggingHorizontal = false;
        }
        if (active) wasDraggingHorizontal = true;
    }

    // Horizontal splitter above asset browser
    if (assetBrowserVisible && assetBrowserHeight > 0.0f) {
        const float viewportBottom = topOffset + m_Config.padding + (availableHeight - assetBrowserHeight - m_Config.padding);
        ImVec2 assetPos(viewportPosX, viewportBottom - splitterThickness * 0.5f);
        ImVec2 assetSize(std::max(viewportWidth, 1.0f), splitterThickness);
        
        ImGui::SetCursorScreenPos(assetPos);
        ImGui::InvisibleButton("##AssetBrowserSplitter", assetSize);
        hovered = ImGui::IsItemHovered();
        active = ImGui::IsItemActive();
        if (hovered || active) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            ImU32 color = ImGui::GetColorU32(active ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f)
                                                   : ImVec4(1.0f, 1.0f, 1.0f, 0.35f));
            ImVec2 assetMax(assetPos.x + assetSize.x, assetPos.y + assetSize.y);
            drawList->AddRectFilled(assetPos, assetMax, color);
        }
        if (active) {
            const float maxAssetHeight = std::max(m_Config.minAssetBrowserHeight,
                availableHeight - m_Config.minViewportSize - m_Config.padding);
            m_Config.assetBrowserHeight = std::clamp(assetBrowserHeight - io.MouseDelta.y,
                m_Config.minAssetBrowserHeight, maxAssetHeight);
        }
        // Save config when splitter is released
        static bool wasDraggingAsset = false;
        if (wasDraggingAsset && !active) {
            m_Config.Save(m_ConfigPath.string());
            wasDraggingAsset = false;
        }
        if (active) wasDraggingAsset = true;
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace Editor
} // namespace SAGE
