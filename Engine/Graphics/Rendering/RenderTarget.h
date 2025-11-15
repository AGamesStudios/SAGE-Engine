#pragma once

#include "Graphics/PostProcessing/PostProcessManager.h"
#include "Graphics/Core/Camera2D.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Resources/Material.h"
#include "Graphics/Core/Types/Color.h"
#include "Graphics/Core/Types/GraphicsTypes.h"
#include "Math/Vector2.h"
#include "Memory/Ref.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace SAGE {

/**
 * @brief Render Target - текстура для рендеринга камеры
 * 
 * Позволяет рендерить сцену с камеры в текстуру,
 * которую можно использовать как материал на объектах
 */
class RenderTarget {
public:
    struct Specification {
        unsigned int width = 1280;
        unsigned int height = 720;
        Graphics::TextureFormat colorFormat = Graphics::TextureFormat::RGBA8;
        bool useHDR = false;           // Use RGBA16F for HDR
        bool useDepth = true;          // Attach depth buffer
        bool useStencil = false;       // Attach stencil buffer
        int samples = 0;               // MSAA samples (0 = no MSAA)
    };

    RenderTarget(const Specification& spec);
    ~RenderTarget();

    // Non-copyable
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    void Bind();
    void Unbind();
    void Resize(unsigned int width, unsigned int height);
    
    // Clear with color
    void Clear(const Color& color = Color(0.0f, 0.0f, 0.0f, 1.0f));
    
    Graphics::TextureHandle GetColorTexture() const { return m_ColorTexture; }
    Graphics::TextureHandle GetDepthTexture() const { return m_DepthTexture; }
    Graphics::FramebufferHandle GetFramebuffer() const { return m_Framebuffer; }
    
    unsigned int GetWidth() const { return m_Specification.width; }
    unsigned int GetHeight() const { return m_Specification.height; }
    
    const Specification& GetSpecification() const { return m_Specification; }
    
    // Convert to SAGE Texture for material system
    Ref<Texture> AsTexture();

private:
    void Invalidate();
    void Release();

    Specification m_Specification;
    Graphics::FramebufferHandle m_Framebuffer = Graphics::InvalidFramebufferHandle;
    Graphics::TextureHandle m_ColorTexture = Graphics::InvalidTextureHandle;
    Graphics::TextureHandle m_DepthTexture = Graphics::InvalidTextureHandle;
    
    Ref<Texture> m_TextureWrapper;
};

/**
 * @brief Camera Render System - управляет рендерингом камер в текстуры
 */
class CameraRenderSystem {
public:
    using RenderCallback = std::function<void()>;
    
    CameraRenderSystem();
    ~CameraRenderSystem();
    
    /**
     * @brief Создать render target для камеры
     * @param name Имя render target
     * @param spec Спецификация (размер, формат и т.д.)
     * @return Указатель на созданный render target
     */
    RenderTarget* CreateRenderTarget(const std::string& name, const RenderTarget::Specification& spec);
    
    /**
     * @brief Получить render target по имени
     */
    RenderTarget* GetRenderTarget(const std::string& name);
    
    /**
     * @brief Удалить render target
     */
    void RemoveRenderTarget(const std::string& name);
    
    /**
     * @brief Рендерить в target с использованием камеры
     * @param targetName Имя render target
     * @param camera Камера для рендеринга
     * @param renderCallback Функция, которая рисует сцену
     */
    void RenderToTarget(const std::string& targetName, Camera2D& camera, RenderCallback renderCallback);
    
    /**
     * @brief Рендерить в target без камеры (для пост-процессинга)
     */
    void RenderToTarget(const std::string& targetName, RenderCallback renderCallback);
    
    /**
     * @brief Получить текстуру из render target для использования в материалах
     */
    Ref<Texture> GetTargetTexture(const std::string& name);
    
    /**
     * @brief Изменить размер всех render targets (полезно при resize окна)
     */
    void ResizeAll(unsigned int width, unsigned int height);
    
    /**
     * @brief Очистить все render targets
     */
    void Clear();

private:
    std::unordered_map<std::string, std::unique_ptr<RenderTarget>> m_RenderTargets;
};

} // namespace SAGE
