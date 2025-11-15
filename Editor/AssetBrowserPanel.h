#pragma once

#include "Memory/Ref.h"
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>

namespace SAGE {

class Texture;

namespace Editor {
class EditorConfig;

class EditorScene;
struct SelectionContext;

class AssetBrowserPanel {
public:
    AssetBrowserPanel();
    ~AssetBrowserPanel() = default;

    void SetContext(EditorScene* scene, SelectionContext* selection);
    void SetConfig(EditorConfig* config);
    void Render();
    void RefreshAssets();

    // Callback when asset is double-clicked or selected
    using AssetSelectedCallback = std::function<void(const std::string& path)>;
    void SetAssetSelectedCallback(AssetSelectedCallback callback) { m_OnAssetSelected = callback; }
    
    using AssetDoubleClickedCallback = std::function<void(const std::string& path)>;
    void SetAssetDoubleClickedCallback(AssetDoubleClickedCallback callback) { m_OnAssetDoubleClicked = callback; }

private:
    void ScanAssetDirectory(const std::string& directory);
    void ScanCurrentDirectory();
    void RenderAssetGrid();
    void RenderAssetEntry(const std::string& path, bool isDirectory);
    void RenderBreadcrumb();
    void RenderContextMenu();
    bool IsTextureAsset(const std::string& path) const;
    void NavigateToDirectory(const std::string& path);
    void NavigateUp();
    void NavigateBack();
    void NavigateForward();
    
    // File operations
    void CreateNewFile(const std::string& name);
    void CreateNewFolder(const std::string& name);
    void RenameItem(const std::string& oldPath, const std::string& newName);
    void DeleteItem(const std::string& path);
    void CopyItem(const std::string& path);
    void PasteItem();
    void DuplicateItem(const std::string& path);
    
    // Thumbnail management
    Ref<Texture> GetOrLoadThumbnail(const std::string& absolutePath);
    void ClearThumbnailCache();
    
    struct AssetEntry {
        std::string relativePath;
        std::string absolutePath;
        std::string filename;
        std::string extension;
        bool isDirectory = false;
        bool isTexture = false;
    };

    std::vector<AssetEntry> m_Assets;
    std::string m_AssetsRootPath;
    std::string m_CurrentDirectory;
    std::string m_SelectedAsset;
    std::vector<std::string> m_NavigationHistory;
    int m_NavigationIndex = -1;
    
    // Clipboard
    std::string m_ClipboardPath;
    bool m_ClipboardCut = false;
    
    // Dialogs
    bool m_ShowCreateFileDialog = false;
    bool m_ShowCreateFolderDialog = false;
    bool m_ShowRenameDialog = false;
    bool m_ShowDeleteDialog = false;
    char m_DialogInputBuffer[256] = {};
    std::string m_ItemToRename;
    std::string m_ItemToDelete;
    
    EditorScene* m_Scene = nullptr;
    SelectionContext* m_Selection = nullptr;
    AssetSelectedCallback m_OnAssetSelected;
    AssetDoubleClickedCallback m_OnAssetDoubleClicked;
    EditorConfig* m_Config = nullptr;
    
    float m_ThumbnailSize = 64.0f;
    bool m_ShowOnlyTextures = false;
    
    // Thumbnail cache
    std::unordered_map<std::string, Ref<Texture>> m_ThumbnailCache;
    std::deque<std::string> m_ThumbnailOrder;
    
    // Refresh cooldown (prevent spam)
    float m_LastRefreshTime = 0.0f;
};

} // namespace Editor
} // namespace SAGE
