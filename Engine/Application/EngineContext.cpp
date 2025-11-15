#include "EngineContext.h"
#include "Core/Logger.h"

namespace SAGE {

// Global context pointer (initialized to nullptr)
EngineContext* EngineContext::s_GlobalContext = nullptr;

void EngineContext::Initialize(const std::string& logDir) {
    if (m_Initialized) {
        SAGE_WARN("EngineContext already initialized");
        return;
    }
    
    // Initialize subsystems in dependency order
    // 1. Logger (no dependencies)
    Logger::Init(logDir);
    SAGE_INFO("EngineContext: Logger initialized");
    
    // 2. InputManager (depends on window, initialized later)
    // Note: InputManager::Initialize(window) called separately by application
    
    // Future: Initialize other subsystems
    // m_QuestManager = std::make_unique<QuestManager>();
    // m_ItemDatabase = std::make_unique<ItemDatabase>();
    // etc.
    
    m_Initialized = true;
    SAGE_INFO("EngineContext initialized successfully");
}

void EngineContext::Shutdown() {
    if (!m_Initialized) return;
    
    SAGE_INFO("EngineContext shutting down...");
    
    // Shutdown in reverse order of initialization
    m_InputManager.Shutdown();
    
    // Future: Shutdown other subsystems
    // m_EventPoolManager.reset();
    // m_ProjectManager.reset();
    // m_ScriptRegistry.reset();
    // m_DragDropManager.reset();
    // m_ItemDatabase.reset();
    // m_QuestManager.reset();
    
    Logger::Shutdown();
    
    m_Initialized = false;
    SAGE_INFO("EngineContext shutdown complete");
}

EngineContext& EngineContext::GetGlobal() {
    if (!s_GlobalContext) {
        // Lazy initialization of global context
        static EngineContext globalInstance;
        s_GlobalContext = &globalInstance;
        
        // Auto-initialize if not already done
        if (!s_GlobalContext->IsInitialized()) {
            s_GlobalContext->Initialize();
        }
    }
    return *s_GlobalContext;
}

void EngineContext::SetGlobal(EngineContext* ctx) {
    s_GlobalContext = ctx;
}

} // namespace SAGE
