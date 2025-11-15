#pragma once

#include "Graphics/Rendering/Graph/IRenderPass.h"
#include <vector>
#include <memory>

namespace SAGE {

class RenderGraph {
public:
    void AddPass(std::unique_ptr<IRenderPass> pass) {
        m_Passes.emplace_back(std::move(pass));
    }

    void InitializeAll(IRenderBackend* backend) {
        for (auto& p : m_Passes) {
            p->Initialize(backend);
        }
    }

    void ShutdownAll() {
        for (auto& p : m_Passes) {
            if (p->IsInitialized()) {
                p->Shutdown();
            }
        }
    }

    bool Execute(const FrameContext& ctx) {
        for (auto& p : m_Passes) {
            if (!p->IsInitialized()) {
                continue; // skip silently
            }
            if (!p->Execute(ctx)) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<std::unique_ptr<IRenderPass>> m_Passes;
};

} // namespace SAGE
