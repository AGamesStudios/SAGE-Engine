#include <SAGE/Editor/EditorAPI.h>
#include "../Adapters/EngineImpl.h"
#include "../Adapters/SceneImpl.h"
#include <Core/Logger.h>
#include <Core/FileSystem.h>
#include <nlohmann/json.hpp>
#include <fstream>

namespace {
    template <typename... TArgs>
    nlohmann::json MakeArray(TArgs... values) {
        nlohmann::json arr = nlohmann::json::array();
        (arr.push_back(values), ...);
        return arr;
    }
}

namespace SAGE {
namespace Editor {

class EditorAPIImpl : public EditorAPI {
public:
    explicit EditorAPIImpl(IEngine* engine)
        : m_EngineImpl(static_cast<Internal::EngineImpl*>(engine))
    {
    }
    
    ~EditorAPIImpl() override = default;
    
    // Scene operations
    IScene* CreateScene(const std::string& name) override {
        return m_EngineImpl->CreateScene(name);
    }
    
    void DestroyScene(IScene* scene) override {
        m_EngineImpl->DestroyScene(scene);
    }
    
    bool SaveScene(IScene* scene, const std::string& path) override {
        if (!scene) return false;
        
        nlohmann::json sceneJson = SerializeScene(scene);
        
        try {
            std::ofstream file(path);
            if (!file.is_open()) {
                SAGE_ERROR("Failed to open file for writing: {}", path);
                return false;
            }
            
            file << sceneJson.dump(4);  // Pretty print with 4 spaces
            file.close();
            
            SAGE_INFO("Scene saved to: {}", path);
            return true;
        }
        catch (const std::exception& e) {
            SAGE_ERROR("Failed to save scene: {}", e.what());
            return false;
        }
    }
    
    IScene* LoadScene(const std::string& path) override {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                SAGE_ERROR("Failed to open file for reading: {}", path);
                return nullptr;
            }
            
            nlohmann::json sceneJson;
            file >> sceneJson;
            file.close();
            
            IScene* scene = DeserializeScene(sceneJson);
            
            if (scene) {
                SAGE_INFO("Scene loaded from: {}", path);
            }
            
            return scene;
        }
        catch (const std::exception& e) {
            SAGE_ERROR("Failed to load scene: {}", e.what());
            return nullptr;
        }
    }
    
    nlohmann::json SerializeScene(IScene* scene) override {
        nlohmann::json json;
        
        json["version"] = 1;
        json["name"] = scene->GetName();
        
        nlohmann::json entitiesJson = nlohmann::json::array();
        
        auto entities = scene->GetAllEntities();
        for (EntityHandle entity : entities) {
            nlohmann::json entityJson;
            entityJson["name"] = scene->GetEntityName(entity);
            
            // Serialize components
            nlohmann::json componentsJson = nlohmann::json::object();
            
            // Transform (always present)
            {
                TransformData transform = scene->GetTransform(entity);
                nlohmann::json transformJson;
                transformJson["position"] = MakeArray(transform.position.x, transform.position.y, transform.position.z);
                transformJson["rotation"] = MakeArray(transform.rotation.x, transform.rotation.y, transform.rotation.z);
                transformJson["scale"] = MakeArray(transform.scale.x, transform.scale.y, transform.scale.z);
                componentsJson["Transform"] = transformJson;
            }
            
            // Sprite (optional)
            if (scene->HasSprite(entity)) {
                SpriteData sprite = scene->GetSprite(entity);
                nlohmann::json spriteJson;
                spriteJson["color"] = MakeArray(sprite.color.r, sprite.color.g, sprite.color.b, sprite.color.a);
                spriteJson["size"] = MakeArray(sprite.size.x, sprite.size.y);
                spriteJson["layer"] = sprite.layer;
                componentsJson["Sprite"] = spriteJson;
            }
            
            entityJson["components"] = componentsJson;
            entitiesJson.push_back(entityJson);
        }
        
        json["entities"] = entitiesJson;
        
        return json;
    }
    
    IScene* DeserializeScene(const nlohmann::json& json) override {
        std::string name = json.value("name", "Untitled Scene");
        IScene* scene = CreateScene(name);
        
        if (json.contains("entities")) {
            for (const auto& entityJson : json["entities"]) {
                std::string entityName = entityJson.value("name", "Entity");
                EntityHandle entity = scene->CreateEntity(entityName);
                
                if (entityJson.contains("components")) {
                    const auto& componentsJson = entityJson["components"];
                    
                    // Transform
                    if (componentsJson.contains("Transform")) {
                        const auto& transformJson = componentsJson["Transform"];
                        TransformData transform;
                        
                        if (transformJson.contains("position")) {
                            auto pos = transformJson["position"];
                            transform.position = {
                                pos[0].get<float>(),
                                pos[1].get<float>(),
                                pos[2].get<float>()
                            };
                        }
                        
                        if (transformJson.contains("rotation")) {
                            auto rot = transformJson["rotation"];
                            transform.rotation = {
                                rot[0].get<float>(),
                                rot[1].get<float>(),
                                rot[2].get<float>()
                            };
                        }
                        
                        if (transformJson.contains("scale")) {
                            auto scl = transformJson["scale"];
                            transform.scale = {
                                scl[0].get<float>(),
                                scl[1].get<float>(),
                                scl[2].get<float>()
                            };
                        }
                        
                        scene->SetTransform(entity, transform);
                    }
                    
                    // Sprite
                    if (componentsJson.contains("Sprite")) {
                        scene->AddSprite(entity);
                        
                        const auto& spriteJson = componentsJson["Sprite"];
                        SpriteData sprite;
                        
                        if (spriteJson.contains("color")) {
                            auto col = spriteJson["color"];
                            sprite.color = Color(
                                col[0].get<float>(),
                                col[1].get<float>(),
                                col[2].get<float>(),
                                col[3].get<float>()
                            );
                        }
                        
                        if (spriteJson.contains("size")) {
                            auto sz = spriteJson["size"];
                            sprite.size = Vector2(
                                sz[0].get<float>(),
                                sz[1].get<float>()
                            );
                        }
                        
                        if (spriteJson.contains("layer")) {
                            sprite.layer = spriteJson["layer"].get<int>();
                        }
                        
                        scene->SetSprite(entity, sprite);
                    }
                }
            }
        }
        
        return scene;
    }
    
    // Entity operations
    EntityHandle CreateEntity(IScene* scene, const std::string& name) override {
        return scene ? scene->CreateEntity(name) : NullEntity;
    }
    
    void DestroyEntity(IScene* scene, EntityHandle entity) override {
        if (scene) scene->DestroyEntity(entity);
    }
    
    EntityHandle DuplicateEntity(IScene* scene, EntityHandle entity) override {
        return scene ? scene->DuplicateEntity(entity) : NullEntity;
    }
    
    // Component operations
    void AddComponent(IScene* scene, EntityHandle entity, const std::string& type) override {
        if (!scene) return;
        
        if (type == "Sprite") {
            scene->AddSprite(entity);
        }
        else if (type == "Camera") {
            scene->AddCamera(entity);
        }
        else {
            SAGE_WARN("Unknown component type: {}", type);
        }
    }
    
    void RemoveComponent(IScene* scene, EntityHandle entity, const std::string& type) override {
        if (!scene) return;
        
        if (type == "Sprite") {
            scene->RemoveSprite(entity);
        }
        else if (type == "Camera") {
            scene->RemoveCamera(entity);
        }
    }
    
    bool HasComponent(IScene* scene, EntityHandle entity, const std::string& type) override {
        return scene ? scene->HasComponent(entity, type) : false;
    }
    
    nlohmann::json GetComponentData(IScene* scene, EntityHandle entity, const std::string& type) override {
        if (!scene) return nlohmann::json::object();
        
        nlohmann::json data;
        
        if (type == "Transform") {
            TransformData transform = scene->GetTransform(entity);
            data["position"] = MakeArray(transform.position.x, transform.position.y, transform.position.z);
            data["rotation"] = MakeArray(transform.rotation.x, transform.rotation.y, transform.rotation.z);
            data["scale"] = MakeArray(transform.scale.x, transform.scale.y, transform.scale.z);
        }
        else if (type == "Sprite" && scene->HasSprite(entity)) {
            SpriteData sprite = scene->GetSprite(entity);
            data["color"] = MakeArray(sprite.color.r, sprite.color.g, sprite.color.b, sprite.color.a);
            data["size"] = MakeArray(sprite.size.x, sprite.size.y);
            data["layer"] = sprite.layer;
        }
        
        return data;
    }
    
    void SetComponentData(IScene* scene, EntityHandle entity, const std::string& type, const nlohmann::json& data) override {
        if (!scene) return;
        
        if (type == "Transform") {
            TransformData transform;
            
            if (data.contains("position")) {
                auto pos = data["position"];
                transform.position = {pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()};
            }
            
            if (data.contains("rotation")) {
                auto rot = data["rotation"];
                transform.rotation = {rot[0].get<float>(), rot[1].get<float>(), rot[2].get<float>()};
            }
            
            if (data.contains("scale")) {
                auto scl = data["scale"];
                transform.scale = {scl[0].get<float>(), scl[1].get<float>(), scl[2].get<float>()};
            }
            
            scene->SetTransform(entity, transform);
        }
        else if (type == "Sprite" && scene->HasSprite(entity)) {
            SpriteData sprite = scene->GetSprite(entity);
            
            if (data.contains("color")) {
                auto col = data["color"];
                sprite.color = {col[0].get<float>(), col[1].get<float>(), col[2].get<float>(), col[3].get<float>()};
            }
            
            if (data.contains("size")) {
                auto sz = data["size"];
                sprite.size = Vector2(sz[0].get<float>(), sz[1].get<float>());
            }
            
            if (data.contains("layer")) {
                sprite.layer = data["layer"].get<int>();
            }
            
            scene->SetSprite(entity, sprite);
        }
    }
    
    std::vector<std::string> GetAvailableComponentTypes() const override {
        return {"Transform", "Sprite", "Camera", "RigidBody", "Collider"};
    }
    
    // Resource operations
    TextureHandle LoadTexture(const std::string&) override {
        // TODO: Implement using ResourceManager
        return NullTexture;
    }
    
    void UnloadTexture(TextureHandle) override {
        // TODO: Implement
    }
    
    Vector2 GetTextureSize(TextureHandle) override {
        // TODO: Implement
        return {0, 0};
    }
    
    ShaderHandle LoadShader(const std::string&, const std::string&) override {
        // TODO: Implement
        return NullShader;
    }
    
    void UnloadShader(ShaderHandle) override {
        // TODO: Implement
    }
    
    // Rendering
    void RenderScene(IScene* scene) override {
        if (scene) {
            scene->Render();
        }
    }
    
    void RenderSceneWithCamera(IScene*, EntityHandle) override {
        // TODO: Implement
    }
    
    void BeginFrame() override {
        // TODO: Implement
    }
    
    void EndFrame() override {
        // TODO: Implement
    }
    
    // Utilities
    EntityHandle CreateTemplateEntity(IScene* scene, const std::string& templateName) override {
        if (!scene) return NullEntity;
        
        EntityHandle entity = scene->CreateEntity(templateName);
        
        if (templateName == "Sprite") {
            scene->AddSprite(entity);
        }
        else if (templateName == "Camera") {
            scene->AddCamera(entity);
        }
        
        return entity;
    }
    
    IEngine* GetEngine() override {
        return m_EngineImpl;
    }
    
private:
    Internal::EngineImpl* m_EngineImpl;
};

// Factory implementation
EditorAPI* CreateEditorAPI(IEngine* engine) {
    return new EditorAPIImpl(engine);
}

void DestroyEditorAPI(EditorAPI* api) {
    delete api;
}

} // namespace Editor
} // namespace SAGE
