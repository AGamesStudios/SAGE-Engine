#include "AssetBrowserPanel.h"
#include "EditorConfig.h"
#include "EditorScene.h"
#include "SelectionContext.h"
#include "Core/Logger.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Core/ResourceManager.h"
#include "FileUtils.h"

#include <imgui/imgui.h>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace SAGE {
namespace Editor {

void AssetBrowserPanel::SetContext(EditorScene* scene, SelectionContext* selection) {
    m_Scene = scene;
    m_Selection = selection;
}

void AssetBrowserPanel::SetConfig(EditorConfig* config) {
    m_Config = config;
    if (!m_Config) {
        return;
    }

    m_ThumbnailSize = std::clamp(m_Config->assetThumbnailSizeDefault,
        m_Config->assetThumbnailSizeMin, m_Config->assetThumbnailSizeMax);

    const int cacheLimit = std::max(1, m_Config->maxThumbnailCacheSize);
    while (static_cast<int>(m_ThumbnailOrder.size()) > cacheLimit) {
        const std::string& oldest = m_ThumbnailOrder.front();
        m_ThumbnailCache.erase(oldest);
        m_ThumbnailOrder.pop_front();
    }
}

AssetBrowserPanel::AssetBrowserPanel()
    : m_AssetsRootPath("assets"), m_CurrentDirectory("assets") {
    namespace fs = std::filesystem;
    try {
        const fs::path root = fs::weakly_canonical(m_AssetsRootPath);
        m_AssetsRootPath = root.string();
        m_CurrentDirectory = m_AssetsRootPath;
    } catch (const std::exception& e) {
        SAGE_WARNING("AssetBrowserPanel: Failed to resolve assets root '{}': {}", m_AssetsRootPath, e.what());
    }

    m_NavigationHistory.clear();
    m_NavigationHistory.push_back(m_CurrentDirectory);
    m_NavigationIndex = static_cast<int>(m_NavigationHistory.size()) - 1;

    RefreshAssets();
}

void AssetBrowserPanel::Render() {
    // Navigation bar
    if (ImGui::Button("<-")) {
        NavigateBack();
    }
    ImGui::SameLine();
    if (ImGui::Button("->")) {
        NavigateForward();
    }
    ImGui::SameLine();
    if (ImGui::Button("Up")) {
        NavigateUp();
    }
    ImGui::SameLine();
    
    RenderBreadcrumb();
    
    // Toolbar
    if (ImGui::Button("Refresh")) {
        // Debounce to prevent spam
        const float currentTime = ImGui::GetTime();
        const float cooldown = 1.0f; // 1 second cooldown
        if (currentTime - m_LastRefreshTime >= cooldown) {
            RefreshAssets();
            m_LastRefreshTime = currentTime;
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("New File")) {
        m_ShowCreateFileDialog = true;
        m_DialogInputBuffer[0] = '\0';
    }
    
    ImGui::SameLine();
    if (ImGui::Button("New Folder")) {
        m_ShowCreateFolderDialog = true;
        m_DialogInputBuffer[0] = '\0';
    }
    
    ImGui::SameLine();
    if (ImGui::Checkbox("Textures Only", &m_ShowOnlyTextures)) {
        RefreshAssets();
    }
    
    ImGui::SameLine();
    const float thumbMin = m_Config ? m_Config->assetThumbnailSizeMin : 32.0f;
    const float thumbMax = m_Config ? m_Config->assetThumbnailSizeMax : 128.0f;
    m_ThumbnailSize = std::clamp(m_ThumbnailSize, thumbMin, thumbMax);
    if (ImGui::SliderFloat("Size", &m_ThumbnailSize, thumbMin, thumbMax, "%.0f")) {
        if (m_Config) {
            m_Config->assetThumbnailSizeDefault = m_ThumbnailSize;
        }
    }

    ImGui::Separator();

    // Asset count info
    ImGui::Text("Assets: %zu", m_Assets.size());
    
    ImGui::Separator();

    // Asset grid
    RenderAssetGrid();
    
    // Render dialogs and context menu
    RenderContextMenu();
}

void AssetBrowserPanel::RefreshAssets() {
    m_Assets.clear();
    
    if (!std::filesystem::exists(m_CurrentDirectory)) {
        SAGE_WARNING("AssetBrowserPanel: Directory '{}' does not exist", m_CurrentDirectory);
        return;
    }

    ScanCurrentDirectory();
    
    // Sort: folders first, then files alphabetically
    std::sort(m_Assets.begin(), m_Assets.end(), 
              [](const AssetEntry& a, const AssetEntry& b) {
                  if (a.isDirectory != b.isDirectory) {
                      return a.isDirectory > b.isDirectory;
                  }
                  return a.filename < b.filename;
              });
    
    if (!m_SelectedAsset.empty()) {
        const bool exists = std::any_of(m_Assets.begin(), m_Assets.end(), [&](const AssetEntry& entry) {
            return entry.relativePath == m_SelectedAsset;
        });
        if (!exists) {
            m_SelectedAsset.clear();
        }
    }

    SAGE_INFO("AssetBrowserPanel: Found {} items in '{}'", m_Assets.size(), m_CurrentDirectory);
}

void AssetBrowserPanel::ScanAssetDirectory(const std::string& directory) {
    namespace fs = std::filesystem;
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const std::string absPath = entry.path().string();
            const std::string relPath = fs::relative(entry.path(), m_AssetsRootPath).string();
            const std::string filename = entry.path().filename().string();
            const std::string ext = entry.path().extension().string();
            
            // Skip hidden files
            if (filename.empty() || filename[0] == '.') {
                continue;
            }

            const bool isTexture = IsTextureAsset(ext);
            
            if (m_ShowOnlyTextures && !isTexture) {
                continue;
            }

            AssetEntry asset;
            asset.relativePath = relPath;
            asset.absolutePath = absPath;
            asset.filename = filename;
            asset.extension = ext;
            asset.isDirectory = false;
            asset.isTexture = isTexture;
            
            m_Assets.push_back(asset);
        }
    }
    catch (const fs::filesystem_error& e) {
        SAGE_ERROR("AssetBrowserPanel: Filesystem error scanning '{}': {}", directory, e.what());
    }
}

void AssetBrowserPanel::ScanCurrentDirectory() {
    namespace fs = std::filesystem;
    
    try {
        for (const auto& entry : fs::directory_iterator(m_CurrentDirectory)) {
            const std::string absPath = entry.path().string();
            const std::string relPath = fs::relative(entry.path(), m_AssetsRootPath).string();
            const std::string filename = entry.path().filename().string();
            const std::string ext = entry.path().extension().string();
            
            // Skip hidden files
            if (filename.empty() || filename[0] == '.') {
                continue;
            }

            const bool isDir = entry.is_directory();
            const bool isTexture = !isDir && IsTextureAsset(ext);
            
            if (m_ShowOnlyTextures && !isTexture && !isDir) {
                continue;
            }

            AssetEntry asset;
            asset.relativePath = relPath;
            asset.absolutePath = absPath;
            asset.filename = filename;
            asset.extension = ext;
            asset.isDirectory = isDir;
            asset.isTexture = isTexture;
            
            m_Assets.push_back(asset);
        }
    }
    catch (const fs::filesystem_error& e) {
        SAGE_ERROR("AssetBrowserPanel: Filesystem error scanning '{}': {}", m_CurrentDirectory, e.what());
    }
}

void AssetBrowserPanel::RenderAssetGrid() {
    const float cellPadding = m_Config ? m_Config->assetGridCellPadding : 16.0f;
    const float cellSize = m_ThumbnailSize + cellPadding;
    const float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(panelWidth / cellSize));

    ImGui::BeginChild("AssetGridScroll", ImVec2(0, 0), false);

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(cellPadding * 0.5f, cellPadding * 0.5f));
    if (ImGui::BeginTable("AssetGrid", columns, ImGuiTableFlags_SizingFixedFit)) {
        int column = 0;
        for (const auto& asset : m_Assets) {
            if (column == 0) {
                ImGui::TableNextRow();
            }
            ImGui::TableSetColumnIndex(column);
            
            RenderAssetEntry(asset.relativePath, asset.isDirectory);
            
            column = (column + 1) % columns;
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
    
    if (ImGui::BeginPopupContextWindow("AssetBrowserBackground", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("New File")) {
            m_ShowCreateFileDialog = true;
            m_DialogInputBuffer[0] = '\0';
        }
        if (ImGui::MenuItem("New Folder")) {
            m_ShowCreateFolderDialog = true;
            m_DialogInputBuffer[0] = '\0';
        }
        if (!m_ClipboardPath.empty() && ImGui::MenuItem("Paste")) {
            PasteItem();
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
}

void AssetBrowserPanel::RenderAssetEntry(const std::string& path, bool isDirectory) {
    ImGui::PushID(path.c_str());
    
    const bool isSelected = (m_SelectedAsset == path);
    
    // Find asset entry
    const auto it = std::find_if(m_Assets.begin(), m_Assets.end(),
                                  [&](const AssetEntry& e) { return e.relativePath == path; });
    if (it == m_Assets.end()) {
        ImGui::PopID();
        return;
    }
    const auto& entry = *it;
    
    // Draw thumbnail
    ImVec4 buttonColor = isSelected ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    if (isDirectory) {
        buttonColor = isSelected ? ImVec4(0.5f, 0.6f, 0.3f, 1.0f) : ImVec4(0.4f, 0.4f, 0.2f, 1.0f);
    }
    
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    
    // Try to load thumbnail for image files
    Ref<Texture> thumbnail = nullptr;
    if (!isDirectory && entry.isTexture) {
        thumbnail = GetOrLoadThumbnail(entry.absolutePath);
    }
    
    bool clicked = false;
    if (thumbnail && thumbnail->GetRendererID() != 0) {
        // Render actual texture preview
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.7f));
        
        ImTextureID texID = (ImTextureID)(uintptr_t)thumbnail->GetRendererID();
        ImTextureRef texRef(texID);
        
        clicked = ImGui::ImageButton(
            "##thumb",
            texRef,
            ImVec2(m_ThumbnailSize, m_ThumbnailSize),
            ImVec2(0, 1), ImVec2(1, 0) // Flip Y for OpenGL
        );
        
        ImGui::PopStyleColor(3);
    } else {
        // Fallback to colored button
        clicked = ImGui::Button("##thumb", ImVec2(m_ThumbnailSize, m_ThumbnailSize));
    }
    
    if (clicked) {
        m_SelectedAsset = path;
        if (m_OnAssetSelected && !isDirectory) {
            m_OnAssetSelected(path);
        }
    }
    
    ImGui::PopStyleColor();
    
    // Draw selection border
    const float selectionBorderWidth = m_Config ? m_Config->assetSelectionBorderWidth : 2.0f;
    if (isSelected && selectionBorderWidth > 0.0f) {
        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(rectMin, rectMax, IM_COL32(255, 200, 0, 255), 0.0f, 0, selectionBorderWidth);
    }
    
    // Drag & Drop source
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        const std::filesystem::path& fsPath = entry.absolutePath;
        const std::string pathStr = fsPath.string();
        ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", pathStr.c_str(), pathStr.size() + 1);
        ImGui::Text("%s", entry.filename.c_str());
        ImGui::EndDragDropSource();
    }
    
    // Double-click to open
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (isDirectory) {
            NavigateToDirectory(path);
        } else {
            // Call double-click callback
            if (m_OnAssetDoubleClicked) {
                const std::filesystem::path& fsPath = entry.absolutePath;
                m_OnAssetDoubleClicked(fsPath.string());
            }
        }
    }
    
    // Context menu
    if (ImGui::BeginPopupContextItem("AssetContextMenu")) {
        m_SelectedAsset = path;
        
        if (ImGui::MenuItem("Rename")) {
            m_ShowRenameDialog = true;
            m_ItemToRename = entry.absolutePath;
            std::strncpy(m_DialogInputBuffer, entry.filename.c_str(), sizeof(m_DialogInputBuffer) - 1);
        }
        if (ImGui::MenuItem("Delete")) {
            m_ShowDeleteDialog = true;
            m_ItemToDelete = entry.absolutePath;
        }
        if (ImGui::MenuItem("Copy")) {
            CopyItem(entry.absolutePath);
        }
        if (ImGui::MenuItem("Duplicate")) {
            DuplicateItem(entry.absolutePath);
        }
        if (!m_ClipboardPath.empty() && ImGui::MenuItem("Paste")) {
            PasteItem();
        }
        
        ImGui::EndPopup();
    }
    
    // Double-click to navigate or assign texture
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        if (isDirectory) {
            NavigateToDirectory(entry.absolutePath);
        } else if (m_Scene && m_Selection && m_Selection->HasSelection()) {
            if (m_Scene->SetSpriteTexture(m_Selection->selectedEntity, path)) {
                SAGE_INFO("AssetBrowserPanel: Assigned texture '{}' to selected entity", path);
            }
        }
    }
    
    // Folder icon text
    if (isDirectory) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.4f, 1.0f));
        ImGui::Text("[DIR]");
        ImGui::PopStyleColor();
    }
    
    // Asset name label (truncated if needed)
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + m_ThumbnailSize);
    ImGui::TextWrapped("%s", entry.filename.c_str());
    ImGui::PopTextWrapPos();
    
    // Tooltip with full path
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", path.c_str());
    }
    
    ImGui::PopID();
}

bool AssetBrowserPanel::IsTextureAsset(const std::string& ext) const {
    static const std::vector<std::string> textureExtensions = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".psd", ".gif", ".hdr", ".pic", ".ppm", ".pgm"
    };
    
    std::string lowerExt = ext;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    return std::find(textureExtensions.begin(), textureExtensions.end(), lowerExt) != textureExtensions.end();
}

void AssetBrowserPanel::RenderBreadcrumb() {
    namespace fs = std::filesystem;
    
    fs::path current = m_CurrentDirectory;
    fs::path root = m_AssetsRootPath;
    
    std::vector<std::string> parts;
    while (current != root && current.has_parent_path() && current != current.parent_path()) {
        parts.push_back(current.filename().string());
        current = current.parent_path();
    }
    parts.push_back(root.filename().string());
    std::reverse(parts.begin(), parts.end());
    
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            ImGui::SameLine();
            ImGui::TextUnformatted("/");
            ImGui::SameLine();
        }
        
        if (ImGui::SmallButton(parts[i].c_str())) {
            fs::path target = root;
            for (size_t j = 1; j <= i; ++j) {
                target /= parts[j];
            }
            NavigateToDirectory(target.string());
        }
    }
}

void AssetBrowserPanel::NavigateToDirectory(const std::string& path) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(path) || !fs::is_directory(path)) {
        SAGE_WARNING("AssetBrowserPanel: Cannot navigate to '{}'", path);
        return;
    }
    
    fs::path target;
    try {
        target = fs::weakly_canonical(path);
    } catch (const std::exception& e) {
        SAGE_WARNING("AssetBrowserPanel: Failed to resolve directory '{}': {}", path, e.what());
        return;
    }

    const std::string root = fs::weakly_canonical(m_AssetsRootPath).string();
    const std::string targetStr = target.string();
    if (targetStr.rfind(root, 0) != 0) {
        SAGE_WARNING("AssetBrowserPanel: Attempt to open directory outside assets root: '{}'", targetStr);
        return;
    }

    if (m_CurrentDirectory == targetStr) {
        return;
    }

    if (m_NavigationIndex >= 0 && static_cast<size_t>(m_NavigationIndex) < m_NavigationHistory.size() - 1) {
        m_NavigationHistory.erase(m_NavigationHistory.begin() + m_NavigationIndex + 1, m_NavigationHistory.end());
    }

    m_CurrentDirectory = targetStr;
    m_NavigationHistory.push_back(m_CurrentDirectory);
    
    // Limit navigation history size
    const size_t maxHistorySize = 50;
    if (m_NavigationHistory.size() > maxHistorySize) {
        const size_t toRemove = m_NavigationHistory.size() - maxHistorySize;
        m_NavigationHistory.erase(m_NavigationHistory.begin(), m_NavigationHistory.begin() + toRemove);
        m_NavigationIndex = static_cast<int>(m_NavigationHistory.size()) - 1;
    } else {
        m_NavigationIndex = static_cast<int>(m_NavigationHistory.size()) - 1;
    }
    
    m_SelectedAsset.clear();
    RefreshAssets();
}

void AssetBrowserPanel::NavigateUp() {
    namespace fs = std::filesystem;
    
    fs::path current = m_CurrentDirectory;
    fs::path parent = current.parent_path();
    
    if (parent.empty() || parent == current) {
        return;
    }
    
    // Don't go above assets root
    const std::string root = fs::weakly_canonical(m_AssetsRootPath).string();
    if (fs::weakly_canonical(parent).string().rfind(root, 0) != 0) {
        return;
    }
    
    NavigateToDirectory(parent.string());
}

void AssetBrowserPanel::NavigateBack() {
    if (m_NavigationIndex <= 0 || m_NavigationHistory.empty()) {
        return;
    }
    
    --m_NavigationIndex;
    m_CurrentDirectory = m_NavigationHistory[m_NavigationIndex];
    m_SelectedAsset.clear();
    RefreshAssets();
}

void AssetBrowserPanel::NavigateForward() {
    if (m_NavigationIndex < 0 || m_NavigationIndex >= static_cast<int>(m_NavigationHistory.size()) - 1) {
        return;
    }
    
    ++m_NavigationIndex;
    m_CurrentDirectory = m_NavigationHistory[m_NavigationIndex];
    m_SelectedAsset.clear();
    RefreshAssets();
}

Ref<Texture> AssetBrowserPanel::GetOrLoadThumbnail(const std::string& absolutePath) {
    // Check cache first
    auto it = m_ThumbnailCache.find(absolutePath);
    if (it != m_ThumbnailCache.end()) {
        auto orderIt = std::find(m_ThumbnailOrder.begin(), m_ThumbnailOrder.end(), absolutePath);
        if (orderIt != m_ThumbnailOrder.end()) {
            m_ThumbnailOrder.erase(orderIt);
            m_ThumbnailOrder.push_back(absolutePath);
        }
        return it->second;
    }
    
    // Check if it's an image file
    if (!FileUtils::IsImageFile(absolutePath)) {
        return nullptr;
    }
    
    // Load texture
    try {
        auto texture = ResourceManager::Get().Load<Texture>(absolutePath);
        if (texture) {
            m_ThumbnailCache[absolutePath] = texture;
            m_ThumbnailOrder.push_back(absolutePath);
            const int cacheLimit = m_Config ? m_Config->maxThumbnailCacheSize : 100;
            while (static_cast<int>(m_ThumbnailOrder.size()) > cacheLimit) {
                const std::string& oldest = m_ThumbnailOrder.front();
                m_ThumbnailCache.erase(oldest);
                m_ThumbnailOrder.pop_front();
            }
            return texture;
        }
    } catch (const std::exception& e) {
        SAGE_ERROR("AssetBrowserPanel: Failed to load thumbnail '{}': {}", absolutePath, e.what());
    }
    
    return nullptr;
}

void AssetBrowserPanel::ClearThumbnailCache() {
    m_ThumbnailCache.clear();
    m_ThumbnailOrder.clear();
}

} // namespace Editor
} // namespace SAGE
