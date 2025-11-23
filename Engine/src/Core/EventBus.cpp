#include "SAGE/Core/EventBus.h"

namespace SAGE {

EventBus& EventBus::Get() {
    static EventBus instance;
    return instance;
}

} // namespace SAGE
