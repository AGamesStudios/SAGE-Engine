#pragma once

#include <glad/glad.h>
#include "Core/Logger.h"
#include <string>

namespace SAGE {

// RAII scope that checks GL errors that occurred within its lifetime.
// Usage: GLErrorScope scope("Texture::Allocate");
// Errors are logged once on destruction.
class GLErrorScope {
public:
    explicit GLErrorScope(const char* label, bool enabled = true)
        : m_Label(label ? label : "GLErrorScope"), m_Enabled(enabled) {
        if (m_Enabled) {
            Drain(); // clear any pre-existing errors so we only report new ones
        }
    }

    ~GLErrorScope() {
        if (!m_Enabled) return;
        Report();
    }

    // Manually report mid-scope (continues capturing afterwards)
    void CheckPoint(const char* note = nullptr) {
        if (!m_Enabled) return;
        Report(note);
    }

private:
    void Drain() {
        if (!m_Enabled) return;
        while (glGetError() != GL_NO_ERROR) {}
    }

    void Report(const char* note = nullptr) {
        GLenum err;
        bool any = false;
        while ((err = glGetError()) != GL_NO_ERROR) {
            any = true;
            SAGE_ERROR("[GL] Error 0x{:04X} captured in scope '{}'{}", err, m_Label,
                       note ? std::string(" (checkpoint: ") + note + ")" : std::string());
        }
        if (any) {
            SAGE_ERROR("[GL] Above errors occurred inside '{}'.", m_Label);
        }
    }

    std::string m_Label;
    bool m_Enabled = true;
};

} // namespace SAGE
