// Engine/Debug/SceneHierarchy.cpp

#include "SceneHierarchy.h"

#if __has_include("imgui.h")

#include "imgui.h"
#include <sstream>

namespace SAGE {

void SceneHierarchy::Render() {
    if (!m_IsOpen) return;

    if (!ImGui::Begin("Scene Hierarchy", &m_IsOpen)) {
        ImGui::End();
        return;
    }

    // Get all entities
    auto& entities = EntityManager::Get().GetAllEntities();
    
    ImGui::Text("Entities: %zu", entities.size());
    ImGui::Separator();

    // List entities
    for (const auto& entityID : entities) {
        std::ostringstream oss;
        oss << "Entity " << entityID;
        
        bool isSelected = (m_SelectedEntity == entityID);
        
        if (ImGui::Selectable(oss.str().c_str(), isSelected)) {
            m_SelectedEntity = entityID;
            if (m_SelectionCallback) {
                m_SelectionCallback(entityID);
            }
        }
    }

    ImGui::End();
}

} // namespace SAGE

#else

// Stub
namespace SAGE {

void SceneHierarchy::Render() {}

} // namespace SAGE

#endif
