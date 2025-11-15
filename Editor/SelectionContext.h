#pragma once

#include <SAGE/Types.h>

namespace SAGE {
namespace Editor {

struct SelectionContext {
    EntityHandle selectedEntity = NullEntity;

    bool HasSelection() const { return selectedEntity != NullEntity && selectedEntity != 0; }
    void Clear() { selectedEntity = NullEntity; }
};

} // namespace Editor
} // namespace SAGE
