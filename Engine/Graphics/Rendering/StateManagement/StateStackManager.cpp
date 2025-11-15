#include "StateStackManager.h"
#include "Core/Logger.h"

namespace SAGE {
namespace StateManagement {

void StateStackManager::Init() {
    m_TotalStackDepth = 0;
}

void StateStackManager::Shutdown() {
    if (m_TotalStackDepth != 0) {
        SAGE_WARNING("[StateStackManager] Shutdown with non-empty stacks: depth={}", m_TotalStackDepth);
    }
    m_TotalStackDepth = 0;
}

void StateStackManager::Validate() const {
    if (m_TotalStackDepth > 100) {
        SAGE_WARNING("[StateStackManager] Unusually deep state stack: depth={}", m_TotalStackDepth);
    }
}

} // namespace StateManagement
} // namespace SAGE
