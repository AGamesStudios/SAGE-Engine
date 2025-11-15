#include "../TestFramework.h"
#include "Graphics/Rendering/Graph/RenderGraph.h"
#include "Graphics/Rendering/Graph/IRenderPass.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include <vector>
#include <string>

using namespace SAGE;

// Mock pass that records its execution
class MockPass : public IRenderPass {
public:
    MockPass(const std::string& name, std::vector<std::string>* executionLog, bool shouldFail = false)
        : m_Name(name), m_ExecutionLog(executionLog), m_ShouldFail(shouldFail) {}
    
    const char* GetName() const override { return m_Name.c_str(); }
    
    void Initialize(IRenderBackend* backend) override {
        m_Backend = backend;
        m_Initialized = true;
    }
    
    void Shutdown() override {
        m_Backend = nullptr;
        m_Initialized = false;
    }
    
    bool IsInitialized() const override { return m_Initialized; }
    
    bool Execute(const FrameContext& ctx) override {
        if (m_ExecutionLog) {
            m_ExecutionLog->push_back(m_Name);
        }
        return !m_ShouldFail; // Return false if this pass should fail
    }

private:
    std::string m_Name;
    std::vector<std::string>* m_ExecutionLog = nullptr;
    IRenderBackend* m_Backend = nullptr;
    bool m_Initialized = false;
    bool m_ShouldFail = false;
};

// Mock backend for testing
class MockBackend : public IRenderBackend {
public:
    void Configure(const Graphics::RenderSystemConfig& config) override {}
    void Init() override { m_Initialized = true; }
    void Shutdown() override { m_Initialized = false; }
    bool IsInitialized() const override { return m_Initialized; }
    Graphics::BackendType GetBackendType() const override { return Graphics::BackendType::OpenGL; }
    
    void SetViewport(int x, int y, int width, int height) override {}
    void Clear(float r, float g, float b, float a) override {}
    void SetClearColor(float r, float g, float b, float a) override {}
    
    void EnableDepthTest(bool enable) override {}
    void SetDepthFunction(DepthFunction func) override {}
    void EnableDepthWrite(bool enable) override {}
    void SetDepthBias(float constantFactor, float slopeFactor) override {}
    
    void EnableBlending(bool enable) override {}
    void SetBlendFunction(BlendFactor src, BlendFactor dst) override {}
    void SetBlendFunctionSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha) override {}
    
    void EnableCulling(bool enable) override {}
    void SetCullFace(CullFace face) override {}
    
    void EnableScissorTest(bool enable) override {}
    void SetScissorRect(int x, int y, int width, int height) override {}
    
    void SetColorMask(bool r, bool g, bool b, bool a) override {}
    
    void DrawArrays(PrimitiveType type, int first, int count) override {}
    void DrawIndexed(PrimitiveType type, int indexCount, int indexOffset) override {}
    
    DrawStats GetDrawStats() const override { return DrawStats{}; }
    void ResetDrawStats() override {}

private:
    bool m_Initialized = false;
};

TEST_CASE("RenderGraph: Pass Execution Order", "[RenderGraph]") {
    std::vector<std::string> executionLog;
    auto graph = std::make_unique<RenderGraph>();
    
    // Add passes in specific order
    graph->AddPass(std::make_unique<MockPass>("Pass1", &executionLog));
    graph->AddPass(std::make_unique<MockPass>("Pass2", &executionLog));
    graph->AddPass(std::make_unique<MockPass>("Pass3", &executionLog));
    
    MockBackend backend;
    backend.Init();
    graph->InitializeAll(&backend);
    
    // Execute graph
    FrameContext ctx{};
    ctx.deltaTime = 0.016f;
    ctx.backend = &backend;
    
    bool result = graph->ExecuteAll(ctx);
    
    REQUIRE(result == true);
    REQUIRE(executionLog.size() == 3);
    REQUIRE(executionLog[0] == "Pass1");
    REQUIRE(executionLog[1] == "Pass2");
    REQUIRE(executionLog[2] == "Pass3");
    
    graph->ShutdownAll();
}

TEST_CASE("RenderGraph: Failure Propagation", "[RenderGraph]") {
    std::vector<std::string> executionLog;
    auto graph = std::make_unique<RenderGraph>();
    
    // Add passes where Pass2 will fail
    graph->AddPass(std::make_unique<MockPass>("Pass1", &executionLog, false));
    graph->AddPass(std::make_unique<MockPass>("Pass2", &executionLog, true)); // This one fails
    graph->AddPass(std::make_unique<MockPass>("Pass3", &executionLog, false));
    
    MockBackend backend;
    backend.Init();
    graph->InitializeAll(&backend);
    
    // Execute graph
    FrameContext ctx{};
    ctx.deltaTime = 0.016f;
    ctx.backend = &backend;
    
    bool result = graph->ExecuteAll(ctx);
    
    // Graph should stop at failed pass
    REQUIRE(result == false);
    REQUIRE(executionLog.size() == 2); // Only Pass1 and Pass2 executed
    REQUIRE(executionLog[0] == "Pass1");
    REQUIRE(executionLog[1] == "Pass2");
    // Pass3 should not execute after Pass2 failure
    
    graph->ShutdownAll();
}

TEST_CASE("RenderGraph: Domain Filtering", "[RenderGraph]") {
    // Mock pass that only executes in World domain
    class DomainFilteredPass : public IRenderPass {
    public:
        DomainFilteredPass(const std::string& name, RenderDomain targetDomain, std::vector<std::string>* log)
            : m_Name(name), m_TargetDomain(targetDomain), m_Log(log) {}
        
        const char* GetName() const override { return m_Name.c_str(); }
        void Initialize(IRenderBackend* backend) override { m_Initialized = true; }
        void Shutdown() override { m_Initialized = false; }
        bool IsInitialized() const override { return m_Initialized; }
        
        bool Execute(const FrameContext& ctx) override {
            if (ctx.pass.domain != m_TargetDomain) {
                return true; // Skip execution for other domains
            }
            if (m_Log) {
                m_Log->push_back(m_Name + ":" + DomainToString(ctx.pass.domain));
            }
            return true;
        }
    
    private:
        std::string DomainToString(RenderDomain domain) {
            switch (domain) {
                case RenderDomain::World: return "World";
                case RenderDomain::UI: return "UI";
                case RenderDomain::PostFX: return "PostFX";
                default: return "Unknown";
            }
        }
        
        std::string m_Name;
        RenderDomain m_TargetDomain;
        std::vector<std::string>* m_Log = nullptr;
        bool m_Initialized = false;
    };
    
    std::vector<std::string> executionLog;
    auto graph = std::make_unique<RenderGraph>();
    
    // Add passes targeting different domains
    graph->AddPass(std::make_unique<DomainFilteredPass>("WorldPass", RenderDomain::World, &executionLog));
    graph->AddPass(std::make_unique<DomainFilteredPass>("UIPass", RenderDomain::UI, &executionLog));
    graph->AddPass(std::make_unique<DomainFilteredPass>("PostFXPass", RenderDomain::PostFX, &executionLog));
    
    MockBackend backend;
    backend.Init();
    graph->InitializeAll(&backend);
    
    // Execute graph in World domain
    FrameContext ctx{};
    ctx.deltaTime = 0.016f;
    ctx.backend = &backend;
    ctx.pass.domain = RenderDomain::World;
    
    graph->ExecuteAll(ctx);
    
    // Only WorldPass should have executed
    REQUIRE(executionLog.size() == 1);
    REQUIRE(executionLog[0] == "WorldPass:World");
    
    executionLog.clear();
    
    // Execute graph in PostFX domain
    ctx.pass.domain = RenderDomain::PostFX;
    graph->ExecuteAll(ctx);
    
    // Only PostFXPass should have executed
    REQUIRE(executionLog.size() == 1);
    REQUIRE(executionLog[0] == "PostFXPass:PostFX");
    
    graph->ShutdownAll();
}

TEST_CASE("RenderGraph: Initialization and Shutdown", "[RenderGraph]") {
    class InitTrackingPass : public IRenderPass {
    public:
        InitTrackingPass() = default;
        
        const char* GetName() const override { return "InitTracker"; }
        void Initialize(IRenderBackend* backend) override {
            m_Backend = backend;
            m_Initialized = true;
            m_InitCount++;
        }
        void Shutdown() override {
            m_Backend = nullptr;
            m_Initialized = false;
            m_ShutdownCount++;
        }
        bool IsInitialized() const override { return m_Initialized; }
        bool Execute(const FrameContext& ctx) override { return true; }
        
        int GetInitCount() const { return m_InitCount; }
        int GetShutdownCount() const { return m_ShutdownCount; }
    
    private:
        IRenderBackend* m_Backend = nullptr;
        bool m_Initialized = false;
        int m_InitCount = 0;
        int m_ShutdownCount = 0;
    };
    
    auto graph = std::make_unique<RenderGraph>();
    auto pass1 = new InitTrackingPass();
    auto pass2 = new InitTrackingPass();
    
    graph->AddPass(std::unique_ptr<IRenderPass>(pass1));
    graph->AddPass(std::unique_ptr<IRenderPass>(pass2));
    
    MockBackend backend;
    backend.Init();
    
    // Initialize all passes
    graph->InitializeAll(&backend);
    
    REQUIRE(pass1->IsInitialized());
    REQUIRE(pass2->IsInitialized());
    REQUIRE(pass1->GetInitCount() == 1);
    REQUIRE(pass2->GetInitCount() == 1);
    
    // Shutdown all passes
    graph->ShutdownAll();
    
    REQUIRE(!pass1->IsInitialized());
    REQUIRE(!pass2->IsInitialized());
    REQUIRE(pass1->GetShutdownCount() == 1);
    REQUIRE(pass2->GetShutdownCount() == 1);
}
