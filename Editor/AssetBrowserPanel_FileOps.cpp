// File operations implementation for AssetBrowserPanel
#include "AssetBrowserPanel.h"
#include "Core/Logger.h"

#include <imgui/imgui.h>
#include <filesystem>
#include <fstream>

namespace SAGE {
namespace Editor {

void AssetBrowserPanel::RenderContextMenu() {
    namespace fs = std::filesystem;
    
    // Create File Dialog
    if (m_ShowCreateFileDialog) {
        ImGui::OpenPopup("Create File");
        m_ShowCreateFileDialog = false;
    }
    
    if (ImGui::BeginPopupModal("Create File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter filename:");
        ImGui::InputText("##filename", m_DialogInputBuffer, sizeof(m_DialogInputBuffer));
        
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            if (m_DialogInputBuffer[0] != '\0') {
                CreateNewFile(m_DialogInputBuffer);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    // Create Folder Dialog
    if (m_ShowCreateFolderDialog) {
        ImGui::OpenPopup("Create Folder");
        m_ShowCreateFolderDialog = false;
    }
    
    if (ImGui::BeginPopupModal("Create Folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter folder name:");
        ImGui::InputText("##foldername", m_DialogInputBuffer, sizeof(m_DialogInputBuffer));
        
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            if (m_DialogInputBuffer[0] != '\0') {
                CreateNewFolder(m_DialogInputBuffer);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    // Rename Dialog
    if (m_ShowRenameDialog) {
        ImGui::OpenPopup("Rename");
        m_ShowRenameDialog = false;
    }
    
    if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new name:");
        ImGui::InputText("##newname", m_DialogInputBuffer, sizeof(m_DialogInputBuffer));
        
        if (ImGui::Button("Rename", ImVec2(120, 0))) {
            if (m_DialogInputBuffer[0] != '\0' && !m_ItemToRename.empty()) {
                RenameItem(m_ItemToRename, m_DialogInputBuffer);
                m_ItemToRename.clear();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_ItemToRename.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    // Delete Confirmation Dialog
    if (m_ShowDeleteDialog) {
        ImGui::OpenPopup("Delete Confirmation");
        m_ShowDeleteDialog = false;
    }
    
    if (ImGui::BeginPopupModal("Delete Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this item?");
        ImGui::Text("%s", m_ItemToDelete.c_str());
        ImGui::Separator();
        
        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            if (!m_ItemToDelete.empty()) {
                DeleteItem(m_ItemToDelete);
                m_ItemToDelete.clear();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_ItemToDelete.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void AssetBrowserPanel::CreateNewFile(const std::string& name) {
    namespace fs = std::filesystem;
    
    fs::path newFilePath = fs::path(m_CurrentDirectory) / name;
    
    if (fs::exists(newFilePath)) {
        SAGE_WARNING("AssetBrowserPanel: File '{}' already exists", newFilePath.string());
        return;
    }
    
    try {
        std::ofstream file(newFilePath);
        if (!file) {
            SAGE_ERROR("AssetBrowserPanel: Failed to create file '{}'", newFilePath.string());
            return;
        }
        file.close();
        
        SAGE_INFO("AssetBrowserPanel: Created file '{}'", newFilePath.string());
        try {
            m_SelectedAsset = std::filesystem::relative(newFilePath, m_AssetsRootPath).string();
        } catch (const std::exception&) {
            m_SelectedAsset.clear();
        }
        RefreshAssets();
    }
    catch (const std::exception& e) {
        SAGE_ERROR("AssetBrowserPanel: Exception creating file '{}': {}", newFilePath.string(), e.what());
    }
}

void AssetBrowserPanel::CreateNewFolder(const std::string& name) {
    namespace fs = std::filesystem;
    
    fs::path newFolderPath = fs::path(m_CurrentDirectory) / name;
    
    if (fs::exists(newFolderPath)) {
        SAGE_WARNING("AssetBrowserPanel: Folder '{}' already exists", newFolderPath.string());
        return;
    }
    
    try {
        if (!fs::create_directory(newFolderPath)) {
            SAGE_ERROR("AssetBrowserPanel: Failed to create folder '{}'", newFolderPath.string());
            return;
        }
        
        SAGE_INFO("AssetBrowserPanel: Created folder '{}'", newFolderPath.string());
        try {
            m_SelectedAsset = std::filesystem::relative(newFolderPath, m_AssetsRootPath).string();
        } catch (const std::exception&) {
            m_SelectedAsset.clear();
        }
        RefreshAssets();
    }
    catch (const fs::filesystem_error& e) {
        SAGE_ERROR("AssetBrowserPanel: Exception creating folder '{}': {}", newFolderPath.string(), e.what());
    }
}

void AssetBrowserPanel::RenameItem(const std::string& oldPath, const std::string& newName) {
    namespace fs = std::filesystem;
    
    fs::path oldFilePath = oldPath;
    fs::path newFilePath = oldFilePath.parent_path() / newName;
    
    if (fs::exists(newFilePath)) {
        SAGE_WARNING("AssetBrowserPanel: Item '{}' already exists", newFilePath.string());
        return;
    }
    
    try {
        fs::rename(oldFilePath, newFilePath);
        SAGE_INFO("AssetBrowserPanel: Renamed '{}' to '{}'", oldPath, newFilePath.string());
        
        try {
            const std::string oldRelative = std::filesystem::relative(oldFilePath, m_AssetsRootPath).string();
            const std::string newRelative = std::filesystem::relative(newFilePath, m_AssetsRootPath).string();
            if (m_SelectedAsset == oldRelative) {
                m_SelectedAsset = newRelative;
            }
        } catch (const std::exception&) {
            // Ignore relative path errors, selection will be cleared on refresh if missing
        }
        
        RefreshAssets();
    }
    catch (const fs::filesystem_error& e) {
        SAGE_ERROR("AssetBrowserPanel: Exception renaming '{}': {}", oldPath, e.what());
    }
}

void AssetBrowserPanel::DeleteItem(const std::string& path) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(path)) {
        SAGE_WARNING("AssetBrowserPanel: Item '{}' does not exist", path);
        return;
    }
    
    try {
        if (fs::is_directory(path)) {
            fs::remove_all(path);
        } else {
            fs::remove(path);
        }
        
        SAGE_INFO("AssetBrowserPanel: Deleted '{}'", path);
        
        try {
            const std::string relative = std::filesystem::relative(path, m_AssetsRootPath).string();
            if (m_SelectedAsset == relative) {
                m_SelectedAsset.clear();
            }
        } catch (const std::exception&) {
            m_SelectedAsset.clear();
        }
        
        RefreshAssets();
    }
    catch (const fs::filesystem_error& e) {
        SAGE_ERROR("AssetBrowserPanel: Exception deleting '{}': {}", path, e.what());
    }
}

void AssetBrowserPanel::CopyItem(const std::string& path) {
    m_ClipboardPath = path;
    m_ClipboardCut = false;
    SAGE_INFO("AssetBrowserPanel: Copied '{}' to clipboard", path);
}

void AssetBrowserPanel::PasteItem() {
    namespace fs = std::filesystem;
    
    if (m_ClipboardPath.empty()) {
        return;
    }
    
    fs::path sourcePath = m_ClipboardPath;
    fs::path destPath = fs::path(m_CurrentDirectory) / sourcePath.filename();
    
    // Handle name conflicts
    if (fs::exists(destPath)) {
        std::string baseName = destPath.stem().string();
        std::string extension = destPath.extension().string();
        int counter = 1;
        
        do {
            destPath = fs::path(m_CurrentDirectory) / (baseName + "_" + std::to_string(counter) + extension);
            ++counter;
        } while (fs::exists(destPath));
    }
    
    try {
        if (fs::is_directory(sourcePath)) {
            fs::copy(sourcePath, destPath, fs::copy_options::recursive);
        } else {
            fs::copy_file(sourcePath, destPath);
        }
        
        SAGE_INFO("AssetBrowserPanel: Pasted '{}' to '{}'", sourcePath.string(), destPath.string());
        try {
            m_SelectedAsset = std::filesystem::relative(destPath, m_AssetsRootPath).string();
        } catch (const std::exception&) {
            m_SelectedAsset.clear();
        }
        RefreshAssets();
    }
    catch (const fs::filesystem_error& e) {
        SAGE_ERROR("AssetBrowserPanel: Exception pasting '{}': {}", sourcePath.string(), e.what());
    }
}

void AssetBrowserPanel::DuplicateItem(const std::string& path) {
    namespace fs = std::filesystem;
    
    fs::path sourcePath = path;
    std::string baseName = sourcePath.stem().string();
    std::string extension = sourcePath.extension().string();
    
    fs::path destPath = sourcePath.parent_path() / (baseName + "_copy" + extension);
    
    // Handle name conflicts
    int counter = 1;
    while (fs::exists(destPath)) {
        destPath = sourcePath.parent_path() / (baseName + "_copy" + std::to_string(counter) + extension);
        ++counter;
    }
    
    try {
        if (fs::is_directory(sourcePath)) {
            fs::copy(sourcePath, destPath, fs::copy_options::recursive);
        } else {
            fs::copy_file(sourcePath, destPath);
        }
        
        SAGE_INFO("AssetBrowserPanel: Duplicated '{}' to '{}'", path, destPath.string());
        try {
            m_SelectedAsset = std::filesystem::relative(destPath, m_AssetsRootPath).string();
        } catch (const std::exception&) {
            m_SelectedAsset.clear();
        }
        RefreshAssets();
    }
    catch (const fs::filesystem_error& e) {
        SAGE_ERROR("AssetBrowserPanel: Exception duplicating '{}': {}", path, e.what());
    }
}

} // namespace Editor
} // namespace SAGE
