#pragma once

#include <string>
#include <vector>

namespace SAGE {
namespace Editor {

class EditorConfig {
public:
    bool Load(const std::string& path);
    bool Save(const std::string& path) const;

    // Layout
    float sidebarWidth = 320.0f;
    float hierarchyHeightRatio = 0.45f;
    float padding = 8.0f;
    float minPanelHeight = 180.0f;
    float minSidebarWidth = 220.0f;
    float minViewportSize = 200.0f;
    float assetBrowserHeight = 200.0f;
    float minAssetBrowserHeight = 120.0f;
    std::string languageCode = "en";
    
    // Gizmo
    float gizmoHandleSize = 16.0f;
    float gizmoRotationHandleDistance = 50.0f;
    float gizmoMinSize = 4.0f;
    float gizmoLineWidth = 2.0f;
    
    // Asset Browser
    float assetThumbnailSizeDefault = 64.0f;
    float assetThumbnailSizeMin = 32.0f;
    float assetThumbnailSizeMax = 128.0f;
    float assetGridCellPadding = 16.0f;
    float assetSelectionBorderWidth = 2.0f;
    
    // Viewport
    float viewportZoomMin = 0.1f;
    float viewportZoomMax = 10.0f;
    float viewportZoomSpeed = 0.1f;
    bool viewportShowGrid = true;
    bool viewportShowAxes = true;
    bool viewportShowGizmos = true;
    
    // Grid
    float gridCellSize = 32.0f;
    float gridLineWidth = 1.0f;
    bool snapToGridDefault = false;
    float snapGridSize = 16.0f;
    
    // Performance
    int maxThumbnailCacheSize = 100;
    int maxUndoHistorySize = 50;
    std::vector<std::string> recentProjects;
};

} // namespace Editor
} // namespace SAGE
