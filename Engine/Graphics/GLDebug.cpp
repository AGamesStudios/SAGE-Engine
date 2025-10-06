#include <glad/glad.h>

#include "GLDebug.h"
#include "../Core/Logger.h"
#include <string>

namespace SAGE {

void GLDebug::CheckError(const char* file, int line, const char* function)
{
    GLenum error = glGetError();
    if (error != 0) // GL_NO_ERROR = 0
    {
        std::string errorString;
        switch (error) {
        case 0x0500: // GL_INVALID_ENUM
            errorString = "GL_INVALID_ENUM";
            break;
        case 0x0501: // GL_INVALID_VALUE
            errorString = "GL_INVALID_VALUE";
            break;
        case 0x0502: // GL_INVALID_OPERATION
            errorString = "GL_INVALID_OPERATION";
            break;
        case 0x0505: // GL_OUT_OF_MEMORY
            errorString = "GL_OUT_OF_MEMORY";
            break;
        case 0x0506: // GL_INVALID_FRAMEBUFFER_OPERATION
            errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            errorString = "UNKNOWN_GL_ERROR";
            break;
        }

        SAGE_ERROR("OpenGL Error: {} (0x{:X}) in '{}' at {}:{}", 
                   errorString, error, function, file, line);
    }
}

void GLDebug::ClearErrors()
{
    while (glGetError() != 0); // GL_NO_ERROR = 0
}

bool GLDebug::HasErrors()
{
    return glGetError() != 0; // GL_NO_ERROR = 0
}

} // namespace SAGE
} // namespace SAGE
