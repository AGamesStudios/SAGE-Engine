#include "ActionContext.h"
#include <algorithm>

namespace SAGE {

ActionContext* ActionContextManager::CreateContext(const std::string& name, int priority) {
    auto context = std::make_unique<ActionContext>(name);
    context->SetPriority(priority);
    auto* ptr = context.get();
    m_Contexts[name] = std::move(context);
    return ptr;
}

ActionContext* ActionContextManager::GetContext(const std::string& name) {
    auto it = m_Contexts.find(name);
    return (it != m_Contexts.end()) ? it->second.get() : nullptr;
}

const ActionContext* ActionContextManager::GetContext(const std::string& name) const {
    auto it = m_Contexts.find(name);
    return (it != m_Contexts.end()) ? it->second.get() : nullptr;
}

void ActionContextManager::ActivateContext(const std::string& name) {
    if (auto* context = GetContext(name)) {
        context->SetActive(true);
    }
}

void ActionContextManager::DeactivateContext(const std::string& name) {
    if (auto* context = GetContext(name)) {
        context->SetActive(false);
    }
}

void ActionContextManager::DeactivateAll() {
    for (auto& [name, context] : m_Contexts) {
        context->SetActive(false);
    }
}

ActionContext* ActionContextManager::GetActiveContext() {
    ActionContext* highestPriority = nullptr;
    int maxPriority = std::numeric_limits<int>::min();
    
    for (auto& [name, context] : m_Contexts) {
        if (context->IsActive() && context->GetPriority() > maxPriority) {
            highestPriority = context.get();
            maxPriority = context->GetPriority();
        }
    }
    
    return highestPriority;
}

const ActionContext* ActionContextManager::GetActiveContext() const {
    const ActionContext* highestPriority = nullptr;
    int maxPriority = std::numeric_limits<int>::min();
    
    for (const auto& [name, context] : m_Contexts) {
        if (context->IsActive() && context->GetPriority() > maxPriority) {
            highestPriority = context.get();
            maxPriority = context->GetPriority();
        }
    }
    
    return highestPriority;
}

bool ActionContextManager::IsActionPressed(const std::string& actionName) const {
    const auto* context = GetActiveContext();
    return context ? context->GetInputMap().IsActionPressed(actionName) : false;
}

bool ActionContextManager::IsActionHeld(const std::string& actionName) const {
    const auto* context = GetActiveContext();
    return context ? context->GetInputMap().IsActionHeld(actionName) : false;
}

bool ActionContextManager::IsActionReleased(const std::string& actionName) const {
    const auto* context = GetActiveContext();
    return context ? context->GetInputMap().IsActionReleased(actionName) : false;
}

float ActionContextManager::GetActionValue(const std::string& actionName) const {
    const auto* context = GetActiveContext();
    return context ? context->GetInputMap().GetActionValue(actionName) : 0.0f;
}

} // namespace SAGE
