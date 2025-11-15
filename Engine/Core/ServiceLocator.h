#pragma once

#include "Core/Core.h"
#include "Memory/Ref.h"
#include "Core/RPGSaveManager.h"
#include "UI/DragDropManager.h"

#include <memory>
#include <stdexcept>

namespace SAGE {

// Forward declarations
class IShaderManager;
class IRenderStateManager;
class AudioSystem;

class LocalizationManager;
class RPGSaveManager;

/// @brief Service Locator pattern implementation
/// Provides centralized access to engine services/subsystems
/// Replaces static singletons with explicit lifetime management
/// 
/// Usage:
/// @code
/// // Register services at startup
/// ServiceLocator locator;
/// locator.RegisterShaderManager(CreateScope<ShaderManager>());
/// locator.RegisterRenderStateManager(CreateScope<RenderStateManager>());
/// locator.Initialize();
/// 
/// // Access services
/// auto& shaderMgr = locator.GetShaderManager();
/// 
/// // Cleanup at shutdown
/// locator.Shutdown();
/// @endcode
class ServiceLocator {
public:
    ServiceLocator() = default;
    ~ServiceLocator();

    // Non-copyable, movable
    ServiceLocator(const ServiceLocator&) = delete;
    ServiceLocator& operator=(const ServiceLocator&) = delete;
    ServiceLocator(ServiceLocator&&) noexcept = default;
    ServiceLocator& operator=(ServiceLocator&&) noexcept = default;

    // ========== Service Registration ==========

    /// Register the shader manager service
    /// @param manager Unique ownership of the shader manager
    void RegisterShaderManager(Scope<IShaderManager> manager);

    /// Register the render state manager service
    /// @param manager Unique ownership of the render state manager
    void RegisterRenderStateManager(Scope<IRenderStateManager> manager);

    /// Register the audio system service
    /// @param audioSystem Unique ownership of the audio system
    void RegisterAudioSystem(Scope<AudioSystem> audioSystem);

    /// Register the drag-drop manager service
    /// @param manager Unique ownership of the drag-drop manager
    void RegisterDragDropManager(Scope<UI::DragDropManager> manager);

    /// Register the localization manager service
    /// @param manager Unique ownership of the localization manager
    void RegisterLocalizationManager(Scope<LocalizationManager> manager);

    /// Register the RPG save manager service
    /// @param manager Unique ownership of the RPG save manager
    void RegisterRPGSaveManager(Scope<RPGSaveManager> manager);

    // ========== Service Access ==========

    /// Get the shader manager service
    /// @return Reference to the shader manager
    /// @throws std::runtime_error if service is not registered
    [[nodiscard]] IShaderManager& GetShaderManager();
    [[nodiscard]] const IShaderManager& GetShaderManager() const;

    /// Get the render state manager service
    /// @return Reference to the render state manager
    /// @throws std::runtime_error if service is not registered
    [[nodiscard]] IRenderStateManager& GetRenderStateManager();
    [[nodiscard]] const IRenderStateManager& GetRenderStateManager() const;

    /// Get the audio system service
    /// @return Reference to the audio system
    /// @throws std::runtime_error if service is not registered
    [[nodiscard]] AudioSystem& GetAudioSystem();
    [[nodiscard]] const AudioSystem& GetAudioSystem() const;

    /// Get the drag-drop manager service
    /// @return Reference to the drag-drop manager
    /// @throws std::runtime_error if service is not registered
    [[nodiscard]] UI::DragDropManager& GetDragDropManager();
    [[nodiscard]] const UI::DragDropManager& GetDragDropManager() const;

    /// Get the localization manager service
    /// @return Reference to the localization manager
    /// @throws std::runtime_error if service is not registered
    [[nodiscard]] LocalizationManager& GetLocalizationManager();
    [[nodiscard]] const LocalizationManager& GetLocalizationManager() const;

    /// Get the RPG save manager service
    /// @return Reference to the RPG save manager
    /// @throws std::runtime_error if service is not registered
    [[nodiscard]] RPGSaveManager& GetRPGSaveManager();
    [[nodiscard]] const RPGSaveManager& GetRPGSaveManager() const;

    // Optional service presence
    [[nodiscard]] bool HasShaderManager() const { return static_cast<bool>(m_ShaderManager); }
    [[nodiscard]] bool HasRenderStateManager() const { return static_cast<bool>(m_RenderStateManager); }
    [[nodiscard]] bool HasAudioSystem() const { return static_cast<bool>(m_AudioSystem); }
    [[nodiscard]] bool HasDragDropManager() const { return static_cast<bool>(m_DragDropManager); }
    [[nodiscard]] bool HasLocalizationManager() const { return static_cast<bool>(m_LocalizationManager); }
    [[nodiscard]] bool HasRPGSaveManager() const { return static_cast<bool>(m_RPGSaveManager); }

    // ========== Lifecycle Management ==========

    /// Initialize all registered services
    /// Must be called after all services are registered
    void Initialize();

    /// Shutdown all services in reverse order
    void Shutdown();

    /// Check if services are initialized
    [[nodiscard]] bool IsInitialized() const { return m_Initialized; }

    // ========== Global Access ==========

    /// Set the globally accessible service locator instance (non-owning)
    static void SetGlobalInstance(ServiceLocator* instance);

    /// Check if a global service locator instance is registered
    [[nodiscard]] static bool HasGlobalInstance();

    /// Get the global service locator instance
    /// @throws std::runtime_error if no instance registered
    [[nodiscard]] static ServiceLocator& GetGlobalInstance();
    [[nodiscard]] static const ServiceLocator& GetGlobalInstanceConst();

private:
    // Service storage
    Scope<IShaderManager> m_ShaderManager;
    Scope<IRenderStateManager> m_RenderStateManager;
    Scope<AudioSystem> m_AudioSystem;
    Scope<UI::DragDropManager> m_DragDropManager;
    Scope<LocalizationManager> m_LocalizationManager;
    Scope<RPGSaveManager> m_RPGSaveManager;

    bool m_Initialized = false;

    static ServiceLocator* s_GlobalInstance;
};

} // namespace SAGE

