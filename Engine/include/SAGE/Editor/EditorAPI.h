#pragma once

#include "../Types.h"
#include "../IScene.h"
#include <nlohmann/json.hpp>
#include <string>

namespace SAGE {
namespace Editor {

/**
 * @brief High-level API specifically designed for editor tools
 * 
 * Provides simplified, editor-friendly interface for scene manipulation,
 * serialization, and asset management.
 */
class EditorAPI {
public:
    virtual ~EditorAPI() = default;
    
    // ==================== Scene Operations ====================
    
    /**
     * @brief Create a new empty scene
     */
    virtual IScene* CreateScene(const std::string& name) = 0;
    
    /**
     * @brief Destroy a scene (does not delete the active scene)
     */
    virtual void DestroyScene(IScene* scene) = 0;
    
    /**
     * @brief Save scene to JSON file
     * @return true if successful
     */
    virtual bool SaveScene(IScene* scene, const std::string& path) = 0;
    
    /**
     * @brief Load scene from JSON file
     * @return Loaded scene or nullptr on failure
     */
    virtual IScene* LoadScene(const std::string& path) = 0;
    
    /**
     * @brief Serialize scene to JSON object
     */
    virtual nlohmann::json SerializeScene(IScene* scene) = 0;
    
    /**
     * @brief Deserialize scene from JSON object
     */
    virtual IScene* DeserializeScene(const nlohmann::json& json) = 0;
    
    // ==================== Entity Operations ====================
    
    /**
     * @brief Create entity with name
     */
    virtual EntityHandle CreateEntity(IScene* scene, const std::string& name) = 0;
    
    /**
     * @brief Destroy entity from scene
     */
    virtual void DestroyEntity(IScene* scene, EntityHandle entity) = 0;
    
    /**
     * @brief Duplicate entity (deep copy with all components)
     */
    virtual EntityHandle DuplicateEntity(IScene* scene, EntityHandle entity) = 0;
    
    // ==================== Component Operations ====================
    
    /**
     * @brief Add component by type name
     * @param type "Transform", "Sprite", "Camera", "RigidBody", etc.
     */
    virtual void AddComponent(IScene* scene, EntityHandle entity, const std::string& type) = 0;
    
    /**
     * @brief Remove component by type name
     */
    virtual void RemoveComponent(IScene* scene, EntityHandle entity, const std::string& type) = 0;
    
    /**
     * @brief Check if entity has component
     */
    virtual bool HasComponent(IScene* scene, EntityHandle entity, const std::string& type) = 0;
    
    /**
     * @brief Get component data as JSON (for inspector editing)
     */
    virtual nlohmann::json GetComponentData(IScene* scene, EntityHandle entity, const std::string& type) = 0;
    
    /**
     * @brief Set component data from JSON (after inspector edit)
     */
    virtual void SetComponentData(IScene* scene, EntityHandle entity, const std::string& type, const nlohmann::json& data) = 0;
    
    /**
     * @brief Get list of all available component types
     */
    virtual std::vector<std::string> GetAvailableComponentTypes() const = 0;
    
    // ==================== Resource Operations ====================
    
    /**
     * @brief Load texture from file path
     * @return Texture handle or NullTexture on failure
     */
    virtual TextureHandle LoadTexture(const std::string& path) = 0;
    
    /**
     * @brief Unload texture
     */
    virtual void UnloadTexture(TextureHandle handle) = 0;
    
    /**
     * @brief Get texture dimensions
     */
    virtual Vector2 GetTextureSize(TextureHandle handle) = 0;
    
    /**
     * @brief Load shader from vertex and fragment paths
     */
    virtual ShaderHandle LoadShader(const std::string& vertexPath, const std::string& fragmentPath) = 0;
    
    /**
     * @brief Unload shader
     */
    virtual void UnloadShader(ShaderHandle handle) = 0;
    
    // ==================== Rendering (for viewport preview) ====================
    
    /**
     * @brief Render scene to current framebuffer
     */
    virtual void RenderScene(IScene* scene) = 0;
    
    /**
     * @brief Render scene with custom camera
     */
    virtual void RenderSceneWithCamera(IScene* scene, EntityHandle cameraEntity) = 0;
    
    /**
     * @brief Begin frame rendering
     */
    virtual void BeginFrame() = 0;
    
    /**
     * @brief End frame rendering
     */
    virtual void EndFrame() = 0;
    
    // ==================== Utilities ====================
    
    /**
     * @brief Create template entity (empty, sprite, camera, etc.)
     */
    virtual EntityHandle CreateTemplateEntity(IScene* scene, const std::string& templateName) = 0;
    
    /**
     * @brief Get engine instance (for advanced use)
     */
    virtual IEngine* GetEngine() = 0;
};

/**
 * @brief Factory function to create EditorAPI
 * @param engine The engine instance to wrap
 */
EditorAPI* CreateEditorAPI(IEngine* engine);

/**
 * @brief Destroy EditorAPI instance
 */
void DestroyEditorAPI(EditorAPI* api);

} // namespace Editor
} // namespace SAGE
