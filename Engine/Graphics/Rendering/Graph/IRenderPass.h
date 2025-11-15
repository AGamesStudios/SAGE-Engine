#pragma once

#include <string>
#include <vector>
#include <memory>

namespace SAGE {

class Camera2D; // forward
class IRenderBackend; // forward

enum class RenderDomain : uint8_t { World = 0, UI = 1, PostFX = 2 };

struct RenderPassContext {
    RenderDomain domain = RenderDomain::World;
    // Future: layer info, command buffers, visibility lists
};

struct FrameContext {
    float deltaTime = 0.0f;
    Camera2D* camera = nullptr; // non-owning
    IRenderBackend* backend = nullptr; // non-owning
    RenderPassContext pass; // domain-specific context
    // Future: command buffers, resource managers, profiling hooks
};

class IRenderPass {
public:
    virtual ~IRenderPass() = default;
    virtual const char* GetName() const = 0;
    // Called once after creation when backend/context available
    virtual void Initialize(IRenderBackend* backend) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;
    // Execute pass for frame; return false on failure
    virtual bool Execute(const FrameContext& ctx) = 0;
};

} // namespace SAGE
