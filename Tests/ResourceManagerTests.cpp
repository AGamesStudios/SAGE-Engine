#include "catch2.hpp"
#include "SAGE/Core/ResourceManager.h"
#include "SAGE/Graphics/Texture.h"
#include <memory>

using namespace SAGE;

// Mock resource for testing
class MockResource : public IResource {
public:
    bool Load(const std::string& path) override {
        m_Path = path;
        m_Loaded = true;
        return true;
    }
    
    void Unload() override {
        m_Loaded = false;
    }
    
    bool IsLoaded() const override {
        return m_Loaded;
    }
    
    const std::string& GetPath() const override {
        return m_Path;
    }

private:
    std::string m_Path;
    bool m_Loaded = false;
};

TEST_CASE("ResourceManager caching", "[resource][core]") {
    auto& manager = ResourceManager::Get();
    
    SECTION("Load resource") {
        auto resource = manager.Load<MockResource>("test.res");
        REQUIRE(resource != nullptr);
        REQUIRE(resource->IsLoaded());
        REQUIRE(resource->GetPath() == "test.res");
    }
    
    SECTION("Cache hit returns same instance") {
        auto res1 = manager.Load<MockResource>("cached.res");
        auto res2 = manager.Load<MockResource>("cached.res");
        
        REQUIRE(res1.get() == res2.get());
    }
    
    SECTION("Different paths create different instances") {
        auto res1 = manager.Load<MockResource>("file1.res");
        auto res2 = manager.Load<MockResource>("file2.res");
        
        REQUIRE(res1.get() != res2.get());
    }
    
    SECTION("Unload specific resource") {
        auto resource = manager.Load<MockResource>("unload.res");
        REQUIRE(resource->IsLoaded());
        
        manager.Unload<MockResource>("unload.res");
        
        // Loading again should create new instance
        auto resource2 = manager.Load<MockResource>("unload.res");
        REQUIRE(resource.get() != resource2.get());
    }
    
    SECTION("Cleanup unused resources") {
        {
            auto temp = manager.Load<MockResource>("temp.res");
        } // temp goes out of scope
        
        manager.CleanupUnused();
        
        // After cleanup, loading should create new instance
        auto resource = manager.Load<MockResource>("temp.res");
        REQUIRE(resource != nullptr);
    }
    
    SECTION("GetLoadedCount") {
        manager.UnloadAll();
        
        size_t initialCount = manager.GetLoadedCount();
        
        auto res1 = manager.Load<MockResource>("count1.res");
        auto res2 = manager.Load<MockResource>("count2.res");
        
        REQUIRE(manager.GetLoadedCount() >= initialCount + 2);
    }
}
