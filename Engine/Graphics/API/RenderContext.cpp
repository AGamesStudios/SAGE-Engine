#include "Graphics/API/RenderContext.hpp"

#include "Graphics/API/RenderSystemConfig.h"
#include "Graphics/API/RenderSystemRegistry.h"
#include "Graphics/Backend/Common/BackendType.h"
#include "Graphics/Backend/Implementations/OpenGL/OpenGLAdapters.h"
#include "Core/Logger.h"

#include <memory>
#include <utility>

namespace SAGE::Graphics {

namespace {

class DefaultRenderContext final : public RenderContext {
public:
    DefaultRenderContext(RenderSystemConfig config, RenderSystemRegistry& registry)
        : m_Config(std::move(config)), m_Registry(registry) {}

    void Initialize() override {
        if (m_Initialized) {
            return;
        }

        switch (m_Config.backendType) {
        case BackendType::OpenGL:
            InitializeOpenGL();
            break;
        default:
            SAGE_WARNING("RenderContext: backend type {} not supported by default context", static_cast<int>(m_Config.backendType));
            break;
        }

        if (m_Device) {
            m_Registry.SetDevice(m_Device);
        }
        if (m_Context) {
            m_Registry.SetContext(m_Context);
        }
        if (m_ResourceManager) {
            m_Registry.SetResourceManager(m_ResourceManager);
        }

        m_Initialized = true;
    }

    void Shutdown() override {
        if (!m_Initialized) {
            return;
        }

        if (m_ResourceManager) {
            m_ResourceManager->Shutdown();
            m_Registry.SetResourceManager(nullptr);
            m_ResourceManager.reset();
        }

        if (m_Context) {
            m_Registry.SetContext(nullptr);
            m_Context.reset();
        }

        if (m_Device) {
            m_Device->Shutdown();
            m_Registry.SetDevice(nullptr);
            m_Device.reset();
        }

        m_Initialized = false;
    }

    [[nodiscard]] IRenderDevice* Device() const override { return m_Device.get(); }
    [[nodiscard]] IRenderContext* Context() const override { return m_Context.get(); }
    [[nodiscard]] IResourceManager* Resources() const override { return m_ResourceManager.get(); }

private:
    void InitializeOpenGL() {
        auto device = std::make_shared<OpenGLDeviceAdapter>();
        device->Initialize();

        auto context = std::make_shared<OpenGLContextAdapter>();

        auto resources = std::make_shared<OpenGLResourceManagerAdapter>();
        resources->Initialize(*device);

        m_Device = std::move(device);
        m_Context = std::move(context);
        m_ResourceManager = std::move(resources);
    }

    RenderSystemConfig m_Config{};
    RenderSystemRegistry& m_Registry;
    std::shared_ptr<IRenderDevice> m_Device{};
    std::shared_ptr<IRenderContext> m_Context{};
    std::shared_ptr<IResourceManager> m_ResourceManager{};
    bool m_Initialized = false;
};

} // namespace

std::unique_ptr<RenderContext> CreateDefaultRenderContext(const RenderSystemConfig& config,
                                                          RenderSystemRegistry& registry) {
    return std::make_unique<DefaultRenderContext>(config, registry);
}

} // namespace SAGE::Graphics
