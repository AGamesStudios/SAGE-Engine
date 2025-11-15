#include "HierarchyPanel.h"
#include "Localization.h"

#include <imgui.h>

#include <cstring>
#include <string>

namespace SAGE {
namespace Editor {

HierarchyPanel::HierarchyPanel() = default;
HierarchyPanel::~HierarchyPanel() = default;

void HierarchyPanel::SetContext(EditorScene* scene, SelectionContext* selection) {
    m_Scene = scene;
    m_Selection = selection;
    m_RenamingEntity = ECS::NullEntity;
}

void HierarchyPanel::BeginRename(const EditorScene::EntityRecord& record) {
    std::strncpy(m_RenameBuffer, record.name.c_str(), sizeof(m_RenameBuffer));
    m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
    m_RenamingEntity = record.id;
    m_FocusRenameField = true;
}

void HierarchyPanel::Render(bool* pOpen, ImGuiWindowFlags windowFlags, ImVec2* outWindowSize) {
    auto& loc = Localization::Instance();
    std::string windowLabel = loc.Get(TextID::Hierarchy_WindowTitle);
    windowLabel += "##Hierarchy";
    ImGui::Begin(windowLabel.c_str(), pOpen, windowFlags);

    if (!m_Scene) {
        ImGui::TextUnformatted(loc.Get(TextID::Hierarchy_NoScene).c_str());
        if (outWindowSize) {
            *outWindowSize = ImGui::GetWindowSize();
        }
        ImGui::End();
        return;
    }

    ECS::Entity entityToDelete = ECS::NullEntity;

    if (ImGui::Button(loc.Get(TextID::Hierarchy_CreateEntity).c_str())) {
        ECS::Entity newEntity = m_Scene->CreateEntity(loc.Get(TextID::Hierarchy_DefaultEntityName));
        if (m_Selection) {
            m_Selection->selectedEntity = newEntity;
        }
        if (auto* record = m_Scene->FindRecord(newEntity)) {
            BeginRename(*record);
        }
    }

    ImGui::Separator();

    const auto& entities = m_Scene->GetEntities();
    if (entities.empty()) {
        ImGui::TextDisabled("%s", loc.Get(TextID::Hierarchy_NoEntities).c_str());
    } else {
        for (const auto& record : entities) {
            ImGui::PushID(reinterpret_cast<const void*>(&record));
            const bool isSelected = m_Selection && m_Selection->selectedEntity == record.id;
            const bool isRenaming = m_RenamingEntity == record.id;

            if (isRenaming) {
                if (m_FocusRenameField) {
                    ImGui::SetKeyboardFocusHere();
                    m_FocusRenameField = false;
                }

                ImGuiInputTextFlags renameFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
                bool submitted = ImGui::InputText("##RenameEntity", m_RenameBuffer, sizeof(m_RenameBuffer), renameFlags);
                if (submitted) {
                    m_Scene->RenameEntity(record.id, m_RenameBuffer);
                    m_RenamingEntity = ECS::NullEntity;
                }

                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    if (!ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                        m_Scene->RenameEntity(record.id, m_RenameBuffer);
                    }
                    m_RenamingEntity = ECS::NullEntity;
                }
            } else {
                if (ImGui::Selectable(record.name.c_str(), isSelected)) {
                    if (m_Selection) {
                        m_Selection->selectedEntity = record.id;
                    }
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    BeginRename(record);
                }
            }

            if (ImGui::BeginPopupContextItem("EntityContext")) {
                if (!isRenaming && ImGui::MenuItem(loc.Get(TextID::Hierarchy_ContextRename).c_str())) {
                    BeginRename(record);
                }
                if (ImGui::MenuItem(loc.Get(TextID::Hierarchy_ContextDelete).c_str())) {
                    entityToDelete = record.id;
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();

            if (entityToDelete != ECS::NullEntity) {
                break;
            }
        }
    }

    if (outWindowSize) {
        *outWindowSize = ImGui::GetWindowSize();
    }

    ImGui::End();

    if (entityToDelete != ECS::NullEntity) {
        if (m_Selection && m_Selection->selectedEntity == entityToDelete) {
            m_Selection->Clear();
        }
        m_Scene->DestroyEntity(entityToDelete);
        if (m_RenamingEntity == entityToDelete) {
            m_RenamingEntity = ECS::NullEntity;
        }
    }
}

} // namespace Editor
} // namespace SAGE
