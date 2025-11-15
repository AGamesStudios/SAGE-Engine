#include "ServiceLocator.h"
#include "Core/Logger.h"
#include "Graphics/Interfaces/IShaderManager.h"
#include "Graphics/Interfaces/IRenderStateManager.h"
#include "Audio/AudioSystem.h"
#include "UI/DragDropManager.h"
#include "Core/LocalizationManager.h"
#include "Core/RPGSaveManager.h"

#include <stdexcept>

namespace SAGE {

ServiceLocator* ServiceLocator::s_GlobalInstance = nullptr;

ServiceLocator::~ServiceLocator() {
    Shutdown();
    if (s_GlobalInstance == this) {
        s_GlobalInstance = nullptr;
    }
}

// ========== Service Registration ==========

void ServiceLocator::RegisterShaderManager(Scope<IShaderManager> manager) {
    if (m_Initialized) {
        SAGE_ERROR("Cannot register ShaderManager after initialization");
        return;
    }
    m_ShaderManager = std::move(manager);
}

void ServiceLocator::RegisterRenderStateManager(Scope<IRenderStateManager> manager) {
    if (m_Initialized) {
        SAGE_ERROR("Cannot register RenderStateManager after initialization");
        return;
    }
    m_RenderStateManager = std::move(manager);
}

void ServiceLocator::RegisterAudioSystem(Scope<AudioSystem> audioSystem) {
    if (m_Initialized) {
        SAGE_ERROR("Cannot register AudioSystem after initialization");
        return;
    }
    m_AudioSystem = std::move(audioSystem);
}

void ServiceLocator::RegisterDragDropManager(Scope<UI::DragDropManager> manager) {
    if (m_Initialized) {
        SAGE_ERROR("Cannot register DragDropManager after initialization");
        return;
    }
    m_DragDropManager = std::move(manager);
}

void ServiceLocator::RegisterLocalizationManager(Scope<LocalizationManager> manager) {
    if (m_Initialized) {
        SAGE_ERROR("Cannot register LocalizationManager after initialization");
        return;
    }
    m_LocalizationManager = std::move(manager);
}

void ServiceLocator::RegisterRPGSaveManager(Scope<RPGSaveManager> manager) {
    if (m_Initialized) {
        SAGE_ERROR("Cannot register RPGSaveManager after initialization");
        return;
    }
    m_RPGSaveManager = std::move(manager);
}

// ========== Service Access ==========

IShaderManager& ServiceLocator::GetShaderManager() {
    if (!m_ShaderManager) {
        throw std::runtime_error("ShaderManager service not registered in ServiceLocator");
    }
    return *m_ShaderManager;
}

const IShaderManager& ServiceLocator::GetShaderManager() const {
    if (!m_ShaderManager) {
        throw std::runtime_error("ShaderManager service not registered in ServiceLocator");
    }
    return *m_ShaderManager;
}

IRenderStateManager& ServiceLocator::GetRenderStateManager() {
    if (!m_RenderStateManager) {
        throw std::runtime_error("RenderStateManager service not registered in ServiceLocator");
    }
    return *m_RenderStateManager;
}

const IRenderStateManager& ServiceLocator::GetRenderStateManager() const {
    if (!m_RenderStateManager) {
        throw std::runtime_error("RenderStateManager service not registered in ServiceLocator");
    }
    return *m_RenderStateManager;
}

AudioSystem& ServiceLocator::GetAudioSystem() {
    if (!m_AudioSystem) {
        throw std::runtime_error("AudioSystem service not registered in ServiceLocator");
    }
    return *m_AudioSystem;
}

const AudioSystem& ServiceLocator::GetAudioSystem() const {
    if (!m_AudioSystem) {
        throw std::runtime_error("AudioSystem service not registered in ServiceLocator");
    }
    return *m_AudioSystem;
}

UI::DragDropManager& ServiceLocator::GetDragDropManager() {
    if (!m_DragDropManager) {
        throw std::runtime_error("DragDropManager service not registered in ServiceLocator");
    }
    return *m_DragDropManager;
}

const UI::DragDropManager& ServiceLocator::GetDragDropManager() const {
    if (!m_DragDropManager) {
        throw std::runtime_error("DragDropManager service not registered in ServiceLocator");
    }
    return *m_DragDropManager;
}

LocalizationManager& ServiceLocator::GetLocalizationManager() {
    if (!m_LocalizationManager) {
        throw std::runtime_error("LocalizationManager service not registered in ServiceLocator");
    }
    return *m_LocalizationManager;
}

const LocalizationManager& ServiceLocator::GetLocalizationManager() const {
    if (!m_LocalizationManager) {
        throw std::runtime_error("LocalizationManager service not registered in ServiceLocator");
    }
    return *m_LocalizationManager;
}

RPGSaveManager& ServiceLocator::GetRPGSaveManager() {
    if (!m_RPGSaveManager) {
        throw std::runtime_error("RPGSaveManager service not registered in ServiceLocator");
    }
    return *m_RPGSaveManager;
}

const RPGSaveManager& ServiceLocator::GetRPGSaveManager() const {
    if (!m_RPGSaveManager) {
        throw std::runtime_error("RPGSaveManager service not registered in ServiceLocator");
    }
    return *m_RPGSaveManager;
}

// ========== Lifecycle Management ==========

void ServiceLocator::Initialize() {
    if (m_Initialized) {
        SAGE_WARNING("ServiceLocator::Initialize called multiple times");
        return;
    }

    SAGE_INFO("Initializing ServiceLocator...");

    // Initialize services in dependency order
    if (m_ShaderManager) {
        m_ShaderManager->Init();
        SAGE_INFO("  - ShaderManager initialized");
    }

    if (m_RenderStateManager) {
        m_RenderStateManager->Init();
        SAGE_INFO("  - RenderStateManager initialized");
    }

    if (m_AudioSystem) {
        m_AudioSystem->Init();
        SAGE_INFO("  - AudioSystem initialized");
    }

    if (m_DragDropManager) {
        // DragDropManager doesn't need Init (stateless)
        SAGE_INFO("  - DragDropManager registered");
    }

    if (m_LocalizationManager) {
        // LocalizationManager doesn't need Init (loads on demand)
        SAGE_INFO("  - LocalizationManager registered");
    }

    if (m_RPGSaveManager) {
        // RPGSaveManager doesn't need Init (stateful but lazy)
        SAGE_INFO("  - RPGSaveManager registered");
    }

    m_Initialized = true;
    SAGE_INFO("ServiceLocator initialization complete");
}

void ServiceLocator::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    SAGE_INFO("Shutting down ServiceLocator...");

    // Shutdown in reverse order
    m_RPGSaveManager.reset();
    m_LocalizationManager.reset();
    m_DragDropManager.reset();

    if (m_AudioSystem) {
        m_AudioSystem->Shutdown();
        SAGE_INFO("  - AudioSystem shutdown");
    }

    if (m_RenderStateManager) {
        m_RenderStateManager->Shutdown();
        SAGE_INFO("  - RenderStateManager shutdown");
    }

    if (m_ShaderManager) {
        m_ShaderManager->Shutdown();
        SAGE_INFO("  - ShaderManager shutdown");
    }

    m_Initialized = false;
    SAGE_INFO("ServiceLocator shutdown complete");
}

void ServiceLocator::SetGlobalInstance(ServiceLocator* instance) {
    s_GlobalInstance = instance;
}

bool ServiceLocator::HasGlobalInstance() {
    return s_GlobalInstance != nullptr;
}

ServiceLocator& ServiceLocator::GetGlobalInstance() {
    if (!s_GlobalInstance) {
        throw std::runtime_error("Global ServiceLocator instance not set");
    }
    return *s_GlobalInstance;
}

const ServiceLocator& ServiceLocator::GetGlobalInstanceConst() {
    return GetGlobalInstance();
}

} // namespace SAGE
