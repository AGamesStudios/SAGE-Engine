#pragma once

#include "Input/InputManager.h"
#include "Core/Logger.h"
#include <memory>

namespace SAGE {

/**
 * @file EngineContext.h
 * @brief Central context that owns all engine subsystem instances
 * 
 * SOLUTION TO PROBLEM #21: Singleton Hell
 * 
 * Instead of using Meyer's Singletons (static local variables) for managers,
 * we create a single EngineContext that owns all subsystems and manages their
 * lifetime explicitly. This provides:
 * 
 * - Controlled initialization order
 * - Controlled destruction order (reverse of init)
 * - Easy mocking for unit tests
 * - No static initialization order fiasco
 * - Clear dependency injection points
 * 
 * Usage:
 * ```cpp
 * EngineContext ctx;
 * ctx.Initialize();
 * 
 * // Access subsystems via context
 * ctx.GetInputManager().IsKeyPressed(Key::Space);
 * 
 * ctx.Shutdown();
 * ```
 * 
 * For backward compatibility, singletons can delegate to a global context:
 * ```cpp
 * InputManager& InputManager::Get() {
 *     return EngineContext::GetGlobal().GetInputManager();
 * }
 * ```
 */
class EngineContext {
public:
    EngineContext() = default;
    ~EngineContext() { Shutdown(); }
    
    // Non-copyable, non-movable (owns resources)
    EngineContext(const EngineContext&) = delete;
    EngineContext& operator=(const EngineContext&) = delete;
    EngineContext(EngineContext&&) = delete;
    EngineContext& operator=(EngineContext&&) = delete;
    
    /**
     * @brief Initialize all engine subsystems in correct order
     * @param logDir Directory for log files
     */
    void Initialize(const std::string& logDir = "logs");
    
    /**
     * @brief Shutdown all subsystems in reverse order
     */
    void Shutdown();
    
    /**
     * @brief Check if engine is initialized
     */
    bool IsInitialized() const { return m_Initialized; }
    
    // ========================================================================
    // Subsystem Access (Instance-based, not singletons)
    // ========================================================================
    
    /**
     * @brief Get input manager instance
     */
    InputManager& GetInputManager() { return m_InputManager; }
    const InputManager& GetInputManager() const { return m_InputManager; }
    
    /**
     * @brief Get logger instance
     * @note Logger is still static for now (Problem #22)
     */
    // Future: Logger& GetLogger() { return *m_Logger; }
    
    // ========================================================================
    // Global Instance (for backward compatibility during migration)
    // ========================================================================
    
    /**
     * @brief Get global engine context
     * @note This is a temporary solution during migration from singletons.
     *       Eventually, applications should create and own their own context.
     * @deprecated Use dependency injection instead
     */
    static EngineContext& GetGlobal();
    
    /**
     * @brief Set global engine context (for custom contexts)
     * @param ctx Pointer to context (nullptr to reset)
     */
    static void SetGlobal(EngineContext* ctx);

private:
    bool m_Initialized = false;
    
    // Subsystems (owned by context, not singletons)
    InputManager m_InputManager;
    
    // Future subsystems to migrate from singletons:
    // std::unique_ptr<Logger> m_Logger;
    // std::unique_ptr<QuestManager> m_QuestManager;
    // std::unique_ptr<ItemDatabase> m_ItemDatabase;
    // std::unique_ptr<DragDropManager> m_DragDropManager;
    // std::unique_ptr<ScriptRegistry> m_ScriptRegistry;
    // std::unique_ptr<ProjectManager> m_ProjectManager;
    // std::unique_ptr<EventPoolManager> m_EventPoolManager;
    
    // Global instance pointer (for backward compatibility)
    static EngineContext* s_GlobalContext;
};

} // namespace SAGE
