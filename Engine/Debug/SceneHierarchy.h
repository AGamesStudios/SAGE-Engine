// Engine/Debug/SceneHierarchy.h
#pragma once

#include "ECS/ECS.h"
#include <functional>

namespace SAGE {

/**
 * @brief Scene Hierarchy Window
 * 
 * Displays all entities in the scene with their names/IDs.
 * Click to select entity (updates EntityInspector).
 * Shows entity count and active state.
 */
class SceneHierarchy {
public:
    using SelectionCallback = std::function<void(SAGE::ECS::Entity)>;

    SceneHierarchy() = default;

    /**
     * @brief Render scene hierarchy window
     */
    void Render();

    /**
     * @brief Set callback when entity is selected
     */
    void SetSelectionCallback(SelectionCallback callback) { m_SelectionCallback = callback; }

    /**
     * @brief Get selected entity
     */
    SAGE::ECS::Entity GetSelectedEntity() const { return m_SelectedEntity; }

    /**
     * @brief Check if window is open
     */
    bool IsOpen() const { return m_IsOpen; }

    /**
     * @brief Set window open state
     */
    void SetOpen(bool open) { m_IsOpen = open; }

private:
    SAGE::ECS::Entity m_SelectedEntity = SAGE::ECS::NullEntity;
    SelectionCallback m_SelectionCallback;
    bool m_IsOpen = true;
};

} // namespace SAGE
