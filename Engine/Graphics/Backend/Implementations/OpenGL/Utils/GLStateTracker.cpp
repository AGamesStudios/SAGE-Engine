#include "GLStateTracker.h"

#include <algorithm>
#include <glad/glad.h>

#ifndef GL_VERSION_1_0
#include <GL/gl.h>
#endif

#include "GLDebug.h"
#include "Core/Logger.h"

namespace SAGE {

std::vector<GLStateTracker::RenderState> GLStateTracker::s_StateStack{};
std::size_t GLStateTracker::s_TextureSlotCount = 8;
bool GLStateTracker::s_Initialized = false;

void GLStateTracker::Init(std::size_t textureSlots) {
    s_TextureSlotCount = std::max<std::size_t>(1, textureSlots);
    s_StateStack.clear();
    s_StateStack.reserve(4);
    s_Initialized = true;
}

void GLStateTracker::Shutdown() {
    s_StateStack.clear();
    s_TextureSlotCount = 8;
    s_Initialized = false;
}

void GLStateTracker::PushState() {
    if (!s_Initialized) {
        Init();
    }

    s_StateStack.emplace_back(Capture());
}

void GLStateTracker::PopState() {
    if (s_StateStack.empty()) {
        return;
    }

    const RenderState state = s_StateStack.back();
    s_StateStack.pop_back();
    Restore(state);
}

bool GLStateTracker::ValidateState(const char* context) {
    if (s_StateStack.empty()) {
        return true;
    }

    const char* label = context ? context : "GLStateTracker";

    const RenderState& expected = s_StateStack.back();
    RenderState current = Capture();

    bool mismatchDetected = false;

    if (expected.program != current.program) {
        SAGE_WARNING("{}: Shader program mismatch (expected {}, actual {})", label, expected.program, current.program);
        mismatchDetected = true;
    }

    if (expected.vertexArray != current.vertexArray) {
        SAGE_WARNING("{}: VAO mismatch (expected {}, actual {})", label, expected.vertexArray, current.vertexArray);
        mismatchDetected = true;
    }

    if (expected.activeTexture != current.activeTexture) {
        SAGE_WARNING("{}: active texture unit mismatch (expected {}, actual {})", label, expected.activeTexture, current.activeTexture);
        mismatchDetected = true;
    }

    if (expected.framebuffer != current.framebuffer) {
        SAGE_WARNING("{}: framebuffer mismatch (expected {}, actual {})", label, expected.framebuffer, current.framebuffer);
        mismatchDetected = true;
    }

    if (expected.blend.enabled != current.blend.enabled ||
        expected.blend.srcRGB != current.blend.srcRGB ||
        expected.blend.dstRGB != current.blend.dstRGB ||
        expected.blend.srcAlpha != current.blend.srcAlpha ||
        expected.blend.dstAlpha != current.blend.dstAlpha ||
        expected.blend.equationRGB != current.blend.equationRGB ||
        expected.blend.equationAlpha != current.blend.equationAlpha) {
        SAGE_WARNING("{}: Blend state mismatch detected", label);
        mismatchDetected = true;
    }

    if (expected.textureBindings.size() != current.textureBindings.size()) {
        SAGE_WARNING("{}: Texture binding count mismatch (expected {}, actual {})",
                     label,
                     expected.textureBindings.size(),
                     current.textureBindings.size());
        mismatchDetected = true;
    } else {
        for (std::size_t i = 0; i < expected.textureBindings.size(); ++i) {
            if (expected.textureBindings[i] != current.textureBindings[i]) {
                SAGE_WARNING("{}: Texture unit {} mismatch (expected {}, actual {})",
                             label,
                             i,
                             expected.textureBindings[i],
                             current.textureBindings[i]);
                mismatchDetected = true;
                break;
            }
        }
    }

    if (mismatchDetected) {
        SAGE_WARNING("{}: Restoring expected OpenGL state", label);
        Restore(expected);
    }

    return !mismatchDetected;
}

GLStateTracker::RenderState GLStateTracker::Capture() {
    RenderState state{};
    state.textureBindings.assign(s_TextureSlotCount, 0);

    GLint program = 0;
    SAGE_GL_CALL(glGetIntegerv(GL_CURRENT_PROGRAM, &program));
    state.program = static_cast<unsigned int>(program);

    GLint vao = 0;
    SAGE_GL_CALL(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao));
    state.vertexArray = static_cast<unsigned int>(vao);

    GLint activeTexture = 0;
    SAGE_GL_CALL(glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture));
    state.activeTexture = static_cast<unsigned int>(activeTexture);

    GLint framebuffer = 0;
    SAGE_GL_CALL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer));
    state.framebuffer = static_cast<unsigned int>(framebuffer);

    GLboolean blendEnabled = GL_FALSE;
    SAGE_GL_CALL(glGetBooleanv(GL_BLEND, &blendEnabled));
    state.blend.enabled = blendEnabled == GL_TRUE;

    GLint value = 0;
    SAGE_GL_CALL(glGetIntegerv(GL_BLEND_SRC_RGB, &value));
    state.blend.srcRGB = value;
    SAGE_GL_CALL(glGetIntegerv(GL_BLEND_DST_RGB, &value));
    state.blend.dstRGB = value;
    SAGE_GL_CALL(glGetIntegerv(GL_BLEND_SRC_ALPHA, &value));
    state.blend.srcAlpha = value;
    SAGE_GL_CALL(glGetIntegerv(GL_BLEND_DST_ALPHA, &value));
    state.blend.dstAlpha = value;
    SAGE_GL_CALL(glGetIntegerv(GL_BLEND_EQUATION_RGB, &value));
    state.blend.equationRGB = value;
    SAGE_GL_CALL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &value));
    state.blend.equationAlpha = value;

    if (!state.textureBindings.empty()) {
        GLint previous = 0;
        SAGE_GL_CALL(glGetIntegerv(GL_ACTIVE_TEXTURE, &previous));

        for (std::size_t slot = 0; slot < s_TextureSlotCount; ++slot) {
            SAGE_GL_CALL(glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(slot)));
            GLint boundTexture = 0;
            SAGE_GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture));
            state.textureBindings[slot] = static_cast<unsigned int>(boundTexture);
        }

        SAGE_GL_CALL(glActiveTexture(static_cast<GLenum>(previous)));
    }

    return state;
}

void GLStateTracker::Restore(const RenderState& state) {
    SAGE_GL_CALL(glUseProgram(state.program));
    SAGE_GL_CALL(glBindVertexArray(state.vertexArray));

    if (state.blend.enabled) {
        SAGE_GL_CALL(glEnable(GL_BLEND));
    } else {
        SAGE_GL_CALL(glDisable(GL_BLEND));
    }

    SAGE_GL_CALL(glBlendFuncSeparate(static_cast<GLenum>(state.blend.srcRGB),
                                     static_cast<GLenum>(state.blend.dstRGB),
                                     static_cast<GLenum>(state.blend.srcAlpha),
                                     static_cast<GLenum>(state.blend.dstAlpha)));
    SAGE_GL_CALL(glBlendEquationSeparate(static_cast<GLenum>(state.blend.equationRGB),
                                         static_cast<GLenum>(state.blend.equationAlpha)));

    if (!state.textureBindings.empty()) {
        for (std::size_t slot = 0; slot < state.textureBindings.size(); ++slot) {
            SAGE_GL_CALL(glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(slot)));
            SAGE_GL_CALL(glBindTexture(GL_TEXTURE_2D, state.textureBindings[slot]));
        }
        SAGE_GL_CALL(glActiveTexture(static_cast<GLenum>(state.activeTexture)));
    } else {
        SAGE_GL_CALL(glActiveTexture(static_cast<GLenum>(state.activeTexture)));
    }

    SAGE_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, state.framebuffer));
}

} // namespace SAGE

