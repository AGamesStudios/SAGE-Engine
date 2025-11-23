#pragma once

#include "SAGE/Core/ECS.h"
#include "SAGE/Core/Scene.h"
#include <string>
#include <memory>

namespace SAGE {

class Prefab {
public:
    Prefab() = default;
    
    // Create a prefab from an existing entity
    static std::shared_ptr<Prefab> Create(ECS::Entity entity, ECS::Registry& registry);
    
    // Save/Load
    bool Save(const std::string& filepath);
    static std::shared_ptr<Prefab> Load(const std::string& filepath);

    // Instantiate this prefab into a scene
    ECS::Entity Instantiate(Scene* scene);

private:
    std::string m_Data; // JSON string representation of the entity
};

} // namespace SAGE
