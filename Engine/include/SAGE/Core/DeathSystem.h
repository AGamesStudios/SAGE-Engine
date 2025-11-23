#pragma once

#include "SAGE/Core/ECS.h"
#include "SAGE/Core/ECSComponents.h"

namespace SAGE::ECS {

class DeathSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

} // namespace SAGE::ECS
