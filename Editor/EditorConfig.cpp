#include "EditorConfig.h"

#include <Core/Logger.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace SAGE {
namespace Editor {

using json = nlohmann::json;

namespace {
constexpr float kMinRatio = 0.05f;
constexpr float kMaxRatio = 0.95f;

template <typename T>
T JsonGetOrDefault(const json& object, const char* key, const T& fallback) {
    if (!object.is_object() || key == nullptr) {
        return fallback;
    }

    try {
        return object.value(std::string(key), fallback);
    } catch (const std::exception&) {
        return fallback;
    }
}
}

bool EditorConfig::Load(const std::string& path) {
    if (path.empty()) {
        SAGE_WARNING("EditorConfig::Load received empty path");
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        SAGE_INFO("EditorConfig not found at '{}', using defaults", path);
        return false;
    }

    try {
        json data;
        file >> data;

        sidebarWidth = JsonGetOrDefault<float>(data, "sidebarWidth", sidebarWidth);
        hierarchyHeightRatio = JsonGetOrDefault<float>(data, "hierarchyHeightRatio", hierarchyHeightRatio);
        padding = JsonGetOrDefault<float>(data, "padding", padding);
        minPanelHeight = JsonGetOrDefault<float>(data, "minPanelHeight", minPanelHeight);
        minSidebarWidth = JsonGetOrDefault<float>(data, "minSidebarWidth", minSidebarWidth);
        minViewportSize = JsonGetOrDefault<float>(data, "minViewportSize", minViewportSize);
        assetBrowserHeight = JsonGetOrDefault<float>(data, "assetBrowserHeight", assetBrowserHeight);
        minAssetBrowserHeight = JsonGetOrDefault<float>(data, "minAssetBrowserHeight", minAssetBrowserHeight);
        assetThumbnailSizeDefault = JsonGetOrDefault<float>(data, "assetThumbnailSizeDefault", assetThumbnailSizeDefault);
        assetThumbnailSizeMin = JsonGetOrDefault<float>(data, "assetThumbnailSizeMin", assetThumbnailSizeMin);
        assetThumbnailSizeMax = JsonGetOrDefault<float>(data, "assetThumbnailSizeMax", assetThumbnailSizeMax);
        assetGridCellPadding = JsonGetOrDefault<float>(data, "assetGridCellPadding", assetGridCellPadding);
        assetSelectionBorderWidth = JsonGetOrDefault<float>(data, "assetSelectionBorderWidth", assetSelectionBorderWidth);
        gizmoHandleSize = JsonGetOrDefault<float>(data, "gizmoHandleSize", gizmoHandleSize);
        gizmoRotationHandleDistance = JsonGetOrDefault<float>(data, "gizmoRotationHandleDistance", gizmoRotationHandleDistance);
        gizmoMinSize = JsonGetOrDefault<float>(data, "gizmoMinSize", gizmoMinSize);
        gizmoLineWidth = JsonGetOrDefault<float>(data, "gizmoLineWidth", gizmoLineWidth);
        viewportZoomMin = JsonGetOrDefault<float>(data, "viewportZoomMin", viewportZoomMin);
        viewportZoomMax = JsonGetOrDefault<float>(data, "viewportZoomMax", viewportZoomMax);
        viewportZoomSpeed = JsonGetOrDefault<float>(data, "viewportZoomSpeed", viewportZoomSpeed);
    viewportShowGrid = JsonGetOrDefault<bool>(data, "viewportShowGrid", viewportShowGrid);
    viewportShowAxes = JsonGetOrDefault<bool>(data, "viewportShowAxes", viewportShowAxes);
    viewportShowGizmos = JsonGetOrDefault<bool>(data, "viewportShowGizmos", viewportShowGizmos);
        gridCellSize = JsonGetOrDefault<float>(data, "gridCellSize", gridCellSize);
        gridLineWidth = JsonGetOrDefault<float>(data, "gridLineWidth", gridLineWidth);
        snapToGridDefault = JsonGetOrDefault<bool>(data, "snapToGridDefault", snapToGridDefault);
        snapGridSize = JsonGetOrDefault<float>(data, "snapGridSize", snapGridSize);
        maxThumbnailCacheSize = JsonGetOrDefault<int>(data, "maxThumbnailCacheSize", maxThumbnailCacheSize);
        maxUndoHistorySize = JsonGetOrDefault<int>(data, "maxUndoHistorySize", maxUndoHistorySize);
        languageCode = JsonGetOrDefault<std::string>(data, "language", languageCode);

        if (data.contains("recentProjects") && data["recentProjects"].is_array()) {
            recentProjects.clear();
            for (const auto& entry : data["recentProjects"]) {
                if (entry.is_string()) {
                    recentProjects.push_back(entry.get<std::string>());
                }
            }
        }

        hierarchyHeightRatio = std::clamp(hierarchyHeightRatio, kMinRatio, kMaxRatio);
        sidebarWidth = std::max(sidebarWidth, minSidebarWidth);
        minPanelHeight = std::max(0.0f, minPanelHeight);
        padding = std::max(0.0f, padding);
        minViewportSize = std::max(0.0f, minViewportSize);
        minAssetBrowserHeight = std::max(0.0f, minAssetBrowserHeight);
        assetBrowserHeight = std::max(assetBrowserHeight, minAssetBrowserHeight);
        assetThumbnailSizeMin = std::max(8.0f, assetThumbnailSizeMin);
        assetThumbnailSizeMax = std::max(assetThumbnailSizeMin, assetThumbnailSizeMax);
        assetThumbnailSizeDefault = std::clamp(assetThumbnailSizeDefault, assetThumbnailSizeMin, assetThumbnailSizeMax);
        assetGridCellPadding = std::max(0.0f, assetGridCellPadding);
        assetSelectionBorderWidth = std::max(0.0f, assetSelectionBorderWidth);
        gizmoHandleSize = std::max(1.0f, gizmoHandleSize);
        gizmoRotationHandleDistance = std::max(0.0f, gizmoRotationHandleDistance);
        gizmoMinSize = std::clamp(gizmoMinSize, 0.1f, gizmoHandleSize);
        gizmoLineWidth = std::max(0.1f, gizmoLineWidth);
        viewportZoomMin = std::max(0.001f, viewportZoomMin);
        viewportZoomMax = std::max(viewportZoomMin, viewportZoomMax);
        viewportZoomSpeed = std::clamp(viewportZoomSpeed, 0.001f, 10.0f);
        gridCellSize = std::max(1.0f, gridCellSize);
        gridLineWidth = std::max(0.1f, gridLineWidth);
        snapGridSize = std::max(1.0f, snapGridSize);
        maxThumbnailCacheSize = std::max(1, maxThumbnailCacheSize);
        maxUndoHistorySize = std::max(1, maxUndoHistorySize);

        SAGE_INFO("EditorConfig loaded from '{}'", path);
        return true;
    } catch (const std::exception& e) {
        SAGE_ERROR("Failed to parse EditorConfig '{}': {}", path, e.what());
        return false;
    }
}

bool EditorConfig::Save(const std::string& path) const {
    if (path.empty()) {
        SAGE_WARNING("EditorConfig::Save received empty path");
        return false;
    }

    try {
        std::filesystem::path fsPath(path);
        const std::filesystem::path directory = fsPath.parent_path();
        if (!directory.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(directory, ec);
            if (ec) {
                SAGE_WARNING("Failed to create editor config directory '{}': {}", directory.string(), ec.message());
            }
        }

        json data = json::object();
        data["sidebarWidth"] = sidebarWidth;
        data["hierarchyHeightRatio"] = hierarchyHeightRatio;
        data["padding"] = padding;
        data["minPanelHeight"] = minPanelHeight;
        data["minSidebarWidth"] = minSidebarWidth;
        data["minViewportSize"] = minViewportSize;
        data["assetBrowserHeight"] = assetBrowserHeight;
        data["minAssetBrowserHeight"] = minAssetBrowserHeight;
        data["assetThumbnailSizeDefault"] = assetThumbnailSizeDefault;
        data["assetThumbnailSizeMin"] = assetThumbnailSizeMin;
        data["assetThumbnailSizeMax"] = assetThumbnailSizeMax;
        data["assetGridCellPadding"] = assetGridCellPadding;
        data["assetSelectionBorderWidth"] = assetSelectionBorderWidth;
        data["gizmoHandleSize"] = gizmoHandleSize;
        data["gizmoRotationHandleDistance"] = gizmoRotationHandleDistance;
        data["gizmoMinSize"] = gizmoMinSize;
        data["gizmoLineWidth"] = gizmoLineWidth;
        data["viewportZoomMin"] = viewportZoomMin;
        data["viewportZoomMax"] = viewportZoomMax;
        data["viewportZoomSpeed"] = viewportZoomSpeed;
    data["viewportShowGrid"] = viewportShowGrid;
    data["viewportShowAxes"] = viewportShowAxes;
    data["viewportShowGizmos"] = viewportShowGizmos;
        data["gridCellSize"] = gridCellSize;
        data["gridLineWidth"] = gridLineWidth;
        data["snapToGridDefault"] = snapToGridDefault;
        data["snapGridSize"] = snapGridSize;
        data["maxThumbnailCacheSize"] = maxThumbnailCacheSize;
        data["maxUndoHistorySize"] = maxUndoHistorySize;
        data["language"] = languageCode;

        json recentArray = json::array();
        for (const auto& entry : recentProjects) {
            recentArray.push_back(entry);
        }
        data["recentProjects"] = std::move(recentArray);

        std::ofstream file(path);
        if (!file.is_open()) {
            SAGE_ERROR("Failed to open '{}' for writing editor config", path);
            return false;
        }

        file << data.dump(4);
        SAGE_INFO("EditorConfig saved to '{}'", path);
        return true;
    } catch (const std::exception& e) {
        SAGE_ERROR("EditorConfig::Save failed for '{}': {}", path, e.what());
        return false;
    }
}

} // namespace Editor
} // namespace SAGE
