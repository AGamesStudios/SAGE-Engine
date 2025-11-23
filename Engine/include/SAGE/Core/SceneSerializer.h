#pragma once

#include "SAGE/Core/Scene.h"
#include <string>

namespace SAGE {

class SceneSerializer {
public:
    explicit SceneSerializer(Scene* scene);

    void Serialize(const std::string& filepath);
    bool Deserialize(const std::string& filepath);

private:
    Scene* m_Scene;
};

} // namespace SAGE
