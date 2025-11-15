#pragma once

#include <glad/glad.h>

#if !defined(SAGE_GL_CALL)
#define SAGE_GL_CALL(x) x
#endif

#if !defined(SAGE_GL_CHECK)
#define SAGE_GL_CHECK(x)                                                                 \
    do {                                                                                  \
        x;                                                                               \
        GLenum err = glGetError();                                                       \
        if (err != GL_NO_ERROR) {                                                        \
            const char* msg = "UNKNOWN";                                                \
            switch (err) {                                                               \
            case GL_INVALID_ENUM: msg = "GL_INVALID_ENUM"; break;                        \
            case GL_INVALID_VALUE: msg = "GL_INVALID_VALUE"; break;                      \
            case GL_INVALID_OPERATION: msg = "GL_INVALID_OPERATION"; break;              \
            case GL_INVALID_FRAMEBUFFER_OPERATION: msg = "GL_INVALID_FRAMEBUFFER_OPERATION"; break; \
            case GL_OUT_OF_MEMORY: msg = "GL_OUT_OF_MEMORY"; break;                     \
            }                                                                            \
            /* Lightweight stderr fallback to avoid logger dependency if early */       \
            fprintf(stderr, "[SAGE_GL_CHECK] OpenGL error 0x%04X (%s) at %s:%d\n",       \
                    err, msg, __FILE__, __LINE__);                                       \
        }                                                                                \
    } while (0)
#endif

namespace SAGE {

// Simple GL debug helpers placeholder. Extend with KHR_debug bindings if needed.
class GLDebug {
public:
    static void Init();
    static void Shutdown();
};

} // namespace SAGE
