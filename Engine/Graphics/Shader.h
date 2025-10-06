#pragma once

#include <string>
#include <unordered_map>
#include "MathTypes.h"

namespace SAGE {

    class Shader {
    public:
        Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
        ~Shader();
        
        void Bind() const;
        void Unbind() const;
        
        void SetInt(const std::string& name, int value);
        void SetFloat(const std::string& name, float value);
    void SetFloat2(const std::string& name, const Float2& value);
        void SetFloat3(const std::string& name, float v0, float v1, float v2);
        void SetFloat4(const std::string& name, float v0, float v1, float v2, float v3);
        void SetMat4(const std::string& name, const float* value);
        
    private:
        unsigned int m_RendererID;
        std::unordered_map<std::string, int> m_UniformLocationCache;
        
        int GetUniformLocation(const std::string& name);
    };

}
