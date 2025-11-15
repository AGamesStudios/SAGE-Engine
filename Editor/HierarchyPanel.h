#pragma once

#include <imgui.h>
#include "EditorScene.h"
#include "SelectionContext.h"

namespace SAGE {
namespace Editor {

class HierarchyPanel {
public:
    HierarchyPanel();
    ~HierarchyPanel();

    void SetContext(EditorScene* scene, SelectionContext* selection);

    void Render(bool* pOpen = nullptr, ImGuiWindowFlags windowFlags = 0, ImVec2* outWindowSize = nullptr);

private:
    void BeginRename(const EditorScene::EntityRecord& record);

    EditorScene* m_Scene = nullptr;
    SelectionContext* m_Selection = nullptr;
    ECS::Entity m_RenamingEntity = ECS::NullEntity;
    bool m_FocusRenameField = false;
    char m_RenameBuffer[128]{};
};

} // namespace Editor
} // namespace SAGE
