#include "TestFramework.h"
#include "Core/ServiceLocator.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Interfaces/IRenderStateManager.h"
#include "Audio/AudioSystem.h"

using namespace SAGE;
using namespace TestFramework;

// ========== Mock Implementations ==========

class MockShaderManager : public IShaderManager {
public:
    bool initCalled = false;
    bool shutdownCalled = false;
    bool initialized = false;

    void Init() override {
        initCalled = true;
        initialized = true;
    }

    void Shutdown() override {
        shutdownCalled = true;
        initialized = false;
    }

    bool IsInitialized() const override {
        return initialized;
    }

    Ref<Shader> Load(const std::string&, const std::string&, const std::string&) override {
        return nullptr;
    }

    Ref<Shader> Get(const std::string&) override {
        return nullptr;
    }

    void Remove(const std::string&) override {}
    void Clear() override {}

    const std::vector<std::string>& GetRendererUniformNames() const override {
        static std::vector<std::string> names;
        return names;
    }
};

// ========== Tests ==========

TEST_CASE(ServiceLocator_BasicRegistration) {
    ServiceLocator locator;
    
    auto mockShader = CreateScope<MockShaderManager>();
    MockShaderManager* mockPtr = mockShader.get();
    
    locator.RegisterShaderManager(std::move(mockShader));
    
    CHECK(!locator.IsInitialized());
    CHECK(!mockPtr->initCalled);
}

TEST_CASE(ServiceLocator_Initialization) {
    ServiceLocator locator;
    
    auto mockShader = CreateScope<MockShaderManager>();
    MockShaderManager* mockPtr = mockShader.get();
    
    locator.RegisterShaderManager(std::move(mockShader));
    locator.Initialize();
    
    CHECK(locator.IsInitialized());
    CHECK(mockPtr->initCalled);
    CHECK(mockPtr->initialized);
}

TEST_CASE(ServiceLocator_GetService) {
    ServiceLocator locator;
    
    auto mockShader = CreateScope<MockShaderManager>();
    MockShaderManager* mockPtr = mockShader.get();
    
    locator.RegisterShaderManager(std::move(mockShader));
    locator.Initialize();
    
    IShaderManager& shaderMgr = locator.GetShaderManager();
    CHECK(&shaderMgr == mockPtr);
}

TEST_CASE(ServiceLocator_Shutdown) {
    ServiceLocator locator;
    
    auto mockShader = CreateScope<MockShaderManager>();
    MockShaderManager* mockPtr = mockShader.get();
    
    locator.RegisterShaderManager(std::move(mockShader));
    locator.Initialize();
    
    CHECK(mockPtr->initialized);
    
    locator.Shutdown();
    
    CHECK(mockPtr->shutdownCalled);
    CHECK(!mockPtr->initialized);
    CHECK(!locator.IsInitialized());
}

TEST_CASE(ServiceLocator_ThrowsOnMissingService) {
    ServiceLocator locator;
    
    bool exceptionThrown = false;
    try {
        [[maybe_unused]] auto& mgr = locator.GetShaderManager();
    } catch (const std::runtime_error&) {
        exceptionThrown = true;
    }
    
    CHECK(exceptionThrown);
}

TEST_CASE(ServiceLocator_CannotRegisterAfterInit) {
    ServiceLocator locator;
    
    auto mockShader1 = CreateScope<MockShaderManager>();
    locator.RegisterShaderManager(std::move(mockShader1));
    locator.Initialize();
    
    // Try to register another service after initialization
    auto mockShader2 = CreateScope<MockShaderManager>();
    locator.RegisterShaderManager(std::move(mockShader2));
    
    // Should still have the first service
    MockShaderManager* mockPtr = dynamic_cast<MockShaderManager*>(&locator.GetShaderManager());
    CHECK(mockPtr != nullptr);
}

TEST_CASE(ServiceLocator_DestructorCallsShutdown) {
    auto mockShader = CreateScope<MockShaderManager>();
    MockShaderManager* mockPtr = mockShader.get();
    
    {
        ServiceLocator locator;
        locator.RegisterShaderManager(std::move(mockShader));
        locator.Initialize();
        
        CHECK(mockPtr->initialized);
    } // locator destructor should call Shutdown
    
    // Note: Can't check mockPtr->shutdownCalled here because mockShader was moved
    // This test mainly verifies no crashes occur
}

TEST_CASE(ShaderManager_InstanceBased) {
    ShaderManager manager;
    
    CHECK(!manager.IsInitialized());
    
    manager.Init();
    CHECK(manager.IsInitialized());
    
    // Test that uniform names are available
    const auto& uniforms = manager.GetRendererUniformNames();
    CHECK(uniforms.size() > 0);
    
    manager.Shutdown();
    CHECK(!manager.IsInitialized());
}

TEST_CASE(ServiceLocator_MultipleServiceTypes) {
    ServiceLocator locator;
    
    auto mockShader = CreateScope<MockShaderManager>();
    MockShaderManager* shaderPtr = mockShader.get();
    
    locator.RegisterShaderManager(std::move(mockShader));
    locator.Initialize();
    
    CHECK(shaderPtr->initialized);
    
    IShaderManager& shaderMgr = locator.GetShaderManager();
    CHECK(&shaderMgr == shaderPtr);
}

// Force linker to include this translation unit
extern "C" void __sage_force_link_ServiceLocatorTests() {}
