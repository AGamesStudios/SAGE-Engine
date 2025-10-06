#pragma once

// Forward declare GLenum to avoid including glad.h in header
typedef unsigned int GLenum;

namespace SAGE {

    class GLDebug {
    public:
        static void CheckError(const char* file, int line, const char* function);
        static void ClearErrors();
        static bool HasErrors();
    };

#ifdef SAGE_DEBUG
    #define GL_CALL(x) \
        do { \
            SAGE::GLDebug::ClearErrors(); \
            x; \
            SAGE::GLDebug::CheckError(__FILE__, __LINE__, #x); \
        } while(0)
#else
    #define GL_CALL(x) x
#endif

} // namespace SAGE
