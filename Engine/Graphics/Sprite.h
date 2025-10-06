#pragma once

#include "../Graphics/MathTypes.h"
#include "../Memory/Ref.h"
#include "Texture.h"

namespace SAGE {

    class Sprite {
    public:
    Sprite(const Ref<Texture>& texture);
    Sprite(const Float2& size, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
        
    void SetPosition(const Float2& position) { m_Position = position; }
    void SetSize(const Float2& size) { m_Size = size; }
        void SetColor(float r, float g, float b, float a) { 
            m_Color[0] = r; m_Color[1] = g; m_Color[2] = b; m_Color[3] = a; 
        }
        
    const Float2& GetPosition() const { return m_Position; }
    const Float2& GetSize() const { return m_Size; }
        
        void Draw();
        
    private:
    Float2 m_Position;
    Float2 m_Size;
        float m_Color[4];
        Ref<Texture> m_Texture;
    };

}
