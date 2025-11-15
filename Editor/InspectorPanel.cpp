#include "InspectorPanel.h"
#include "Localization.h"
#include "FileUtils.h"
#include "Notifications/NotificationBus.h"
#include "Graphics/Core/Resources/Texture.h"

#include <ECS/Components/ColliderComponent.h>
#include <ECS/Components/ParticleSystemComponent.h>
// #include <ECS/Components/CameraComponent.h> // TODO: Create CameraComponent
#include <ECS/Components/BoxColliderComponent.h>
#include <ECS/Components/CircleColliderComponent.h>

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <filesystem>

namespace SAGE {
namespace Editor {

InspectorPanel::InspectorPanel() = default;
InspectorPanel::~InspectorPanel() = default;

void InspectorPanel::SetContext(EditorScene* scene, SelectionContext* selection) {
    m_Scene = scene;
    m_Selection = selection;
    m_NameBufferEntity = ECS::NullEntity;
    m_TexturePopupEntity = ECS::NullEntity;
    m_RequestTexturePopup = false;
    m_TexturePopupFocusPending = false;
    m_TexturePopupError.clear();
}

void InspectorPanel::Render(bool* pOpen, ImGuiWindowFlags windowFlags, ImVec2* outWindowSize) {
    auto& loc = Localization::Instance();
    std::string windowLabel = loc.Get(TextID::Inspector_WindowTitle);
    windowLabel += "##Inspector";
    ImGui::Begin(windowLabel.c_str(), pOpen, windowFlags);

    if (!m_Scene || !m_Selection || !m_Selection->HasSelection()) {
        ImGui::TextDisabled("%s", loc.Get(TextID::Inspector_NoEntitySelected).c_str());
        if (outWindowSize) {
            *outWindowSize = ImGui::GetWindowSize();
        }
        ImGui::End();
        return;
    }

    EntityHandle entity = m_Selection->selectedEntity;
    auto* record = m_Scene->FindRecord(entity);
    if (!record) {
        ImGui::TextDisabled("%s", loc.Get(TextID::Inspector_SelectedEntityMissing).c_str());
        if (outWindowSize) {
            *outWindowSize = ImGui::GetWindowSize();
        }
        ImGui::End();
        return;
    }

    RenderEntityHeader(*record);
    ImGui::Separator();

    // Render all components
    RenderTransformComponent(entity);
    RenderSpriteComponent(entity);
    RenderRigidBodyComponent(entity);
    RenderColliderComponent(entity);
    RenderParticleSystemComponent(entity);
    // RenderCameraComponent(entity); // TODO: Implement CameraComponent
    
    // Add Component button
    ImGui::Separator();
    ImGui::Spacing();
    RenderAddComponentMenu(entity);
    
    RenderTextureDialog();

    if (outWindowSize) {
        *outWindowSize = ImGui::GetWindowSize();
    }

    ImGui::End();
}

void InspectorPanel::RenderEntityHeader(EditorScene::EntityRecord& record) {
    auto& loc = Localization::Instance();
    if (m_NameBufferEntity != record.id) {
        std::strncpy(m_NameBuffer, record.name.c_str(), sizeof(m_NameBuffer));
        m_NameBuffer[sizeof(m_NameBuffer) - 1] = '\0';
        m_NameBufferEntity = record.id;
    }

    ImGui::Text("%s", loc.Get(TextID::Inspector_EntityLabel).c_str());
    ImGui::SameLine();
    if (m_FocusNameField) {
        ImGui::SetKeyboardFocusHere();
        m_FocusNameField = false;
    }

    ImGuiInputTextFlags nameFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
    bool submitted = ImGui::InputText("##EntityNameInput", m_NameBuffer, sizeof(m_NameBuffer), nameFlags);
    if (submitted) {
        m_Scene->RenameEntity(record.id, m_NameBuffer);
        record = *m_Scene->FindRecord(record.id);
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        if (!ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_Scene->RenameEntity(record.id, m_NameBuffer);
            record = *m_Scene->FindRecord(record.id);
        }
    }

    ImGui::TextDisabled("%s: %llu", loc.Get(TextID::Inspector_IDLabel).c_str(), static_cast<unsigned long long>(record.id));
}

void InspectorPanel::RenderTransformComponent(ECS::Entity entity) {
    if (!m_Scene) return;
    
    auto& loc = Localization::Instance();
    auto& registry = m_Scene->GetECS().GetRegistry();
    
    if (!registry.HasComponent<ECS::TransformComponent>(entity)) {
        return;
    }
    
    auto* transform = registry.GetComponent<ECS::TransformComponent>(entity);
    if (!transform) return;
    
    ImGui::PushID("TransformComponent");
    bool headerOpen = ImGui::CollapsingHeader("Transform Component", ImGuiTreeNodeFlags_DefaultOpen);
    
    if (headerOpen) {
        float position[2] = {transform->position.x, transform->position.y};
        if (ImGui::DragFloat2("Position", position, 1.0f)) {
            transform->position.x = position[0];
            transform->position.y = position[1];
            m_Scene->MarkDirty();
        }

        float rotation = transform->GetRotation();
        if (ImGui::DragFloat("Rotation", &rotation, 1.0f)) {
            transform->SetRotation(rotation);
            m_Scene->MarkDirty();
        }

        float scale[2] = {transform->scale.x, transform->scale.y};
        if (ImGui::DragFloat2("Scale", scale, 0.05f, 0.01f, 1000.0f)) {
            transform->scale.x = scale[0];
            transform->scale.y = scale[1];
            m_Scene->MarkDirty();
        }
        
        // Reset button
        if (ImGui::Button("Reset Transform")) {
            transform->position = Vector2::Zero();
            transform->SetRotation(0.0f);
            transform->scale = Vector2::One();
            m_Scene->MarkDirty();
        }
    }
    
    ImGui::PopID();
}

void InspectorPanel::RenderRigidBodyComponent(ECS::Entity entity) {
    if (!m_Scene) return;
    
    auto& registry = m_Scene->GetECS().GetRegistry();
    
    if (!registry.HasComponent<ECS::RigidBodyComponent>(entity)) {
        return;
    }
    
    auto* body = registry.GetComponent<ECS::RigidBodyComponent>(entity);
    if (!body) return;
    
    ImGui::PushID("RigidBodyComponent");
    bool headerOpen = ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    if (ImGui::SmallButton("X##RemoveRigidBody")) {
        registry.RemoveComponent<ECS::RigidBodyComponent>(entity);
        m_Scene->MarkDirty();
        NotificationBus::Get().Notify("Rigid Body Component removed", NotificationLevel::Info);
        ImGui::PopID();
        return;
    }
    
    if (headerOpen) {
        int overrideIterations = body->solverIterationsOverride;
        if (ImGui::SliderInt("Solver Iterations Override", &overrideIterations, 0, 32)) {
            overrideIterations = std::clamp(overrideIterations, 0, 32);
            body->SetSolverIterationsOverride(overrideIterations);
            m_Scene->MarkDirty();
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset##SolverIterations")) {
            body->SetSolverIterationsOverride(0);
            m_Scene->MarkDirty();
        }

        ImGui::TextDisabled("0 = use world default iterations");
    }
    
    ImGui::PopID();
}

void InspectorPanel::RenderSpriteComponent(ECS::Entity entity) {
    if (!m_Scene) return;
    
    auto& loc = Localization::Instance();
    auto& registry = m_Scene->GetECS().GetRegistry();
    
    if (!registry.HasComponent<ECS::SpriteComponent>(entity)) {
        return;
    }
    
    auto* sprite = registry.GetComponent<ECS::SpriteComponent>(entity);
    if (!sprite) return;
    
    ImGui::PushID("SpriteComponent");
    bool headerOpen = ImGui::CollapsingHeader("Sprite Renderer", ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    if (ImGui::SmallButton("X##RemoveSprite")) {
        registry.RemoveComponent<ECS::SpriteComponent>(entity);
        m_Scene->MarkDirty();
        NotificationBus::Get().Notify("Sprite Component removed", NotificationLevel::Info);
        ImGui::PopID();
        return;
    }
    
    if (headerOpen) {
        // Visibility
        if (ImGui::Checkbox(loc.Get(TextID::Inspector_Visible).c_str(), &sprite->visible)) {
            m_Scene->MarkDirty();
        }
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.2f, 1.0f), "Texture");
        
        // Texture info
        const std::string& textureDisplay = sprite->texturePath.empty()
            ? loc.Get(TextID::Inspector_TextureNone)
            : sprite->texturePath;
        ImGui::TextDisabled("Path: %s", textureDisplay.c_str());
        
        if (sprite->texture) {
            ImGui::Text("Size: %dx%d", sprite->texture->GetWidth(), sprite->texture->GetHeight());
        }
        
        // Texture buttons
        if (ImGui::Button(loc.Get(TextID::Inspector_LoadTexture).c_str(), ImVec2(-1, 0))) {
            OpenTextureDialog(entity, sprite->texturePath);
        }
        
        const bool canClear = sprite->texture && !sprite->texturePath.empty();
        if (ImGui::Button(loc.Get(TextID::Inspector_ClearTexture).c_str(), ImVec2(-1, 0))) {
            if (canClear && m_Scene) {
                m_Scene->SetSpriteTexture(entity, "");
            }
        }
        if (!canClear) {
            ImGui::SameLine();
            ImGui::TextDisabled("(no texture)");
        }
        
        // Drag & Drop target for texture
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
                const char* droppedPath = static_cast<const char*>(payload->Data);
                std::string pathStr(droppedPath);
                
                if (!std::filesystem::exists(pathStr)) {
                    NotificationBus::Get().Notify("File does not exist: " + pathStr, NotificationLevel::Warning);
                } else if (!FileUtils::IsImageFile(pathStr)) {
                    NotificationBus::Get().Notify("Invalid file type. Expected image file.", NotificationLevel::Warning);
                } else {
                    if (m_Scene->SetSpriteTexture(entity, pathStr)) {
                        NotificationBus::Get().Notify("Texture assigned: " + pathStr, NotificationLevel::Info);
                    } else {
                        NotificationBus::Get().Notify("Failed to load texture: " + pathStr, NotificationLevel::Error);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.2f, 1.0f), "UV Coordinates");
        
        // UV Min/Max
        float uvMin[2] = {sprite->uvMin.x, sprite->uvMin.y};
        float uvMax[2] = {sprite->uvMax.x, sprite->uvMax.y};
        bool uvChanged = false;
        
        if (ImGui::DragFloat2("UV Min", uvMin, 0.01f, 0.0f, 1.0f)) {
            sprite->uvMin.x = std::clamp(uvMin[0], 0.0f, 1.0f);
            sprite->uvMin.y = std::clamp(uvMin[1], 0.0f, 1.0f);
            uvChanged = true;
        }
        
        if (ImGui::DragFloat2("UV Max", uvMax, 0.01f, 0.0f, 1.0f)) {
            sprite->uvMax.x = std::clamp(uvMax[0], 0.0f, 1.0f);
            sprite->uvMax.y = std::clamp(uvMax[1], 0.0f, 1.0f);
            uvChanged = true;
        }
        
        if (uvChanged) {
            m_Scene->MarkDirty();
        }
        
        // Quick UV reset button
        if (ImGui::Button("Reset UV", ImVec2(-1, 0))) {
            sprite->uvMin = Vector2(0.0f, 0.0f);
            sprite->uvMax = Vector2(1.0f, 1.0f);
            m_Scene->MarkDirty();
        }
        
        // Pixel region editor (if texture loaded)
        if (sprite->texture) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.2f, 1.0f), "Pixel Region");
            
            static int regionX = 0, regionY = 0, regionW = 0, regionH = 0;
            int texW = (int)sprite->texture->GetWidth();
            int texH = (int)sprite->texture->GetHeight();
            
            ImGui::InputInt("X", &regionX);
            ImGui::InputInt("Y", &regionY);
            ImGui::InputInt("Width", &regionW);
            ImGui::InputInt("Height", &regionH);
            
            if (ImGui::Button("Apply Region", ImVec2(-1, 0))) {
                regionX = std::clamp(regionX, 0, texW);
                regionY = std::clamp(regionY, 0, texH);
                regionW = std::clamp(regionW, 0, texW - regionX);
                regionH = std::clamp(regionH, 0, texH - regionY);
                
                if (regionW > 0 && regionH > 0) {
                    sprite->SetUVRegion((float)texW, (float)texH, (float)regionX, (float)regionY, (float)regionW, (float)regionH);
                    m_Scene->MarkDirty();
                    NotificationBus::Get().Notify("UV region applied", NotificationLevel::Info);
                }
            }
        }
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.2f, 1.0f), "Transform");
        
        // Flip options
        bool flipChanged = false;
        if (ImGui::Checkbox(loc.Get(TextID::Inspector_FlipX).c_str(), &sprite->flipX)) {
            flipChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Checkbox(loc.Get(TextID::Inspector_FlipY).c_str(), &sprite->flipY)) {
            flipChanged = true;
        }
        
        if (flipChanged) {
            m_Scene->MarkDirty();
        }

        // Pivot editing (0..1 normalized)
        float pivot[2] = {sprite->pivot.x, sprite->pivot.y};
        if (ImGui::DragFloat2("Pivot", pivot, 0.01f, 0.0f, 1.0f)) {
            sprite->pivot.x = std::clamp(pivot[0], 0.0f, 1.0f);
            sprite->pivot.y = std::clamp(pivot[1], 0.0f, 1.0f);
            m_Scene->MarkDirty();
        }
        
        // Quick pivot presets
        ImGui::Text("Presets:");
        ImGui::SameLine();
        if (ImGui::SmallButton("Center")) {
            sprite->pivot = Vector2(0.5f, 0.5f);
            m_Scene->MarkDirty();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Top-Left")) {
            sprite->pivot = Vector2(0.0f, 0.0f);
            m_Scene->MarkDirty();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Bottom")) {
            sprite->pivot = Vector2(0.5f, 1.0f);
            m_Scene->MarkDirty();
        }
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.2f, 1.0f), "Color & Opacity");

        // Tint color with alpha
        float color[4] = {
            sprite->tint.r,
            sprite->tint.g,
            sprite->tint.b,
            sprite->tint.a
        };
        if (ImGui::ColorEdit4(loc.Get(TextID::Inspector_Tint).c_str(), color, ImGuiColorEditFlags_AlphaBar)) {
            sprite->tint.r = std::clamp(color[0], 0.0f, 1.0f);
            sprite->tint.g = std::clamp(color[1], 0.0f, 1.0f);
            sprite->tint.b = std::clamp(color[2], 0.0f, 1.0f);
            sprite->tint.a = std::clamp(color[3], 0.0f, 1.0f);
            m_Scene->MarkDirty();
        }
        
        // Opacity slider (separate from color picker)
        float opacity = sprite->tint.a;
        if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f)) {
            sprite->tint.a = opacity;
            m_Scene->MarkDirty();
        }
        
        ImGui::Separator();
        
        // Quick action buttons
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.2f, 1.0f), "Quick Actions");
        
        if (ImGui::Button("Reset Transform", ImVec2(-1, 0))) {
            ResetTransform(entity);
        }
        
        if (ImGui::Button("Reset Size to Texture", ImVec2(-1, 0))) {
            ResetSpriteSize(entity);
        }
        
        if (ImGui::Button("Reset Color", ImVec2(-1, 0))) {
            sprite->tint = Color(1.0f, 1.0f, 1.0f, 1.0f);
            m_Scene->MarkDirty();
        }
    }
    
    ImGui::PopID();
}

void InspectorPanel::RenderSpriteAdvanced(ECS::Entity entity) {
    auto* sprite = m_Scene->GetSprite(entity);
    if (!sprite) return;
    if (ImGui::CollapsingHeader("UV / Region", ImGuiTreeNodeFlags_None)) {
        float uvMin[2] = {sprite->uvMin.x, sprite->uvMin.y};
        float uvMax[2] = {sprite->uvMax.x, sprite->uvMax.y};
        bool changed = false;
        if (ImGui::DragFloat2("UV Min", uvMin, 0.005f, 0.0f, 1.0f)) changed = true;
        if (ImGui::DragFloat2("UV Max", uvMax, 0.005f, 0.0f, 1.0f)) changed = true;
        if (changed) {
            // Ensure ordering uvMin < uvMax per axis
            if (uvMin[0] > uvMax[0]) std::swap(uvMin[0], uvMax[0]);
            if (uvMin[1] > uvMax[1]) std::swap(uvMin[1], uvMax[1]);
            sprite->uvMin.x = uvMin[0]; sprite->uvMin.y = uvMin[1];
            sprite->uvMax.x = uvMax[0]; sprite->uvMax.y = uvMax[1];
            m_Scene->MarkDirty();
        }

        if (sprite->texture) {
            ImGui::Separator();
            ImGui::TextDisabled("Region (px)");
            static int region[4] = {0,0,0,0}; // x,y,w,h
            ImGui::InputInt4("XYWH", region);
            if (ImGui::Button("Apply Region")) {
                int texW = (int)sprite->texture->GetWidth();
                int texH = (int)sprite->texture->GetHeight();
                int x = std::clamp(region[0], 0, texW);
                int y = std::clamp(region[1], 0, texH);
                int w = std::clamp(region[2], 0, texW - x);
                int h = std::clamp(region[3], 0, texH - y);
                if (w > 0 && h > 0) {
                    sprite->SetUVRegion((float)texW, (float)texH, (float)x, (float)y, (float)w, (float)h);
                    // Optionally adjust size to region if size matches previous full texture
                    m_Scene->MarkDirty();
                } else {
                    NotificationBus::Get().Notify("Неверный регион UV", NotificationLevel::Error);
                }
            }
        }
    }
}

void InspectorPanel::ResetTransform(ECS::Entity entity) {
    if (auto* t = m_Scene->GetTransform(entity)) {
        t->position.x = 0.0f; t->position.y = 0.0f; t->SetRotation(0.0f); t->scale.x = 1.0f; t->scale.y = 1.0f;
        m_Scene->MarkDirty();
        NotificationBus::Get().Notify("Трансформ сброшен", NotificationLevel::Info);
    }
}

void InspectorPanel::ResetSpriteSize(ECS::Entity entity) {
    if (auto* t = m_Scene->GetTransform(entity)) {
        if (auto* s = m_Scene->GetSprite(entity)) {
            if (s->texture) {
                t->size.x = (float)s->texture->GetWidth();
                t->size.y = (float)s->texture->GetHeight();
                m_Scene->MarkDirty();
                NotificationBus::Get().Notify("Sprite size reset to texture size", NotificationLevel::Info);
            } else {
                NotificationBus::Get().Notify("No texture available to reset size", NotificationLevel::Warning);
            }
        }
    }
}

void InspectorPanel::RenderTextureDialog() {
    auto& loc = Localization::Instance();
    const std::string popupLabel = loc.Get(TextID::Inspector_TextureDialog_Title) + "##TextureDialog";
    if (m_RequestTexturePopup) {
        ImGui::OpenPopup(popupLabel.c_str());
        m_RequestTexturePopup = false;
    }

    if (ImGui::BeginPopupModal(popupLabel.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted(loc.Get(TextID::Inspector_TextureDialog_Prompt).c_str());
        if (m_TexturePopupFocusPending) {
            ImGui::SetKeyboardFocusHere();
            m_TexturePopupFocusPending = false;
        }

        const ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
        bool submit = ImGui::InputText("##TexturePath", m_TexturePathBuffer, TexturePathCapacity, inputFlags);

        if (!m_TexturePopupError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.45f, 0.45f, 1.0f));
            ImGui::TextWrapped("%s", m_TexturePopupError.c_str());
            ImGui::PopStyleColor();
        }

        if (ImGui::Button(loc.Get(TextID::Inspector_TextureDialog_Submit).c_str()) || submit) {
            if (!m_Scene) {
                m_TexturePopupError = loc.Get(TextID::SceneStatus_NoActiveScene);
            } else if (!m_Scene->SetSpriteTexture(m_TexturePopupEntity, m_TexturePathBuffer)) {
                m_TexturePopupError = loc.Get(TextID::Inspector_TextureDialog_LoadFailed);
            } else {
                m_TexturePopupError.clear();
                m_TexturePopupEntity = ECS::NullEntity;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button(loc.Get(TextID::Inspector_TextureDialog_Cancel).c_str())) {
            m_TexturePopupError.clear();
            m_TexturePopupEntity = ECS::NullEntity;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void InspectorPanel::OpenTextureDialog(ECS::Entity entity, const std::string& currentPath) {
    std::memset(m_TexturePathBuffer, 0, sizeof(m_TexturePathBuffer));
    if (!currentPath.empty()) {
        std::snprintf(m_TexturePathBuffer, TexturePathCapacity, "%s", currentPath.c_str());
    }

    m_TexturePopupEntity = entity;
    m_RequestTexturePopup = true;
    m_TexturePopupFocusPending = true;
    m_TexturePopupError.clear();
}

void InspectorPanel::RenderAddComponentMenu(ECS::Entity entity) {
    if (!m_Scene) return;
    
    auto& loc = Localization::Instance();
    auto& registry = m_Scene->GetECS().GetRegistry();
    
    if (ImGui::Button("+ Add Component", ImVec2(-1, 30))) {
        ImGui::OpenPopup("AddComponentPopup");
    }
    
    if (ImGui::BeginPopup("AddComponentPopup")) {
        ImGui::SeparatorText("Essential Components");
        
        if (!registry.HasComponent<ECS::TransformComponent>(entity)) {
            if (ImGui::MenuItem("Transform")) {
                registry.AddComponent<ECS::TransformComponent>(entity, ECS::TransformComponent{});
                m_Scene->MarkDirty();
                NotificationBus::Get().Notify("Transform Component added", NotificationLevel::Info);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Position, Rotation, Scale");
            }
        }
        
        if (!registry.HasComponent<ECS::SpriteComponent>(entity)) {
            if (ImGui::MenuItem("Sprite Renderer")) {
                registry.AddComponent<ECS::SpriteComponent>(entity, ECS::SpriteComponent{});
                m_Scene->MarkDirty();
                NotificationBus::Get().Notify("Sprite Component added", NotificationLevel::Info);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Render texture/sprite");
            }
        }
        
        // TODO: Implement CameraComponent
        // if (!registry.HasComponent<ECS::CameraComponent>(entity)) {
        //     if (ImGui::MenuItem("Camera")) {
        //         auto camera = ECS::CameraComponent();
        //         camera.width = 800.0f;
        //         camera.height = 600.0f;
        //         registry.AddComponent<ECS::CameraComponent>(entity, camera);
        //         m_Scene->MarkDirty();
        //         NotificationBus::Get().Notify("Camera Component added", NotificationLevel::Success);
        //     }
        //     if (ImGui::IsItemHovered()) {
        //         ImGui::SetTooltip("View camera");
        //     }
        // }
        
        ImGui::SeparatorText("Physics Components");
        
        if (!registry.HasComponent<ECS::RigidBodyComponent>(entity)) {
            if (ImGui::MenuItem("Rigid Body")) {
                registry.AddComponent<ECS::RigidBodyComponent>(entity, ECS::RigidBodyComponent{});
                m_Scene->MarkDirty();
                NotificationBus::Get().Notify("RigidBody Component added", NotificationLevel::Info);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Mass, velocity, forces");
            }
        }
        
        if (!registry.HasComponent<ECS::ColliderComponent>(entity)) {
            if (ImGui::BeginMenu("Collider")) {
                if (ImGui::MenuItem("Box Collider")) {
                    auto collider = ECS::ColliderComponent::CreateBox(Vector2(50, 50));
                    registry.AddComponent<ECS::ColliderComponent>(entity, collider);
                    m_Scene->MarkDirty();
                    NotificationBus::Get().Notify("Box Collider added", NotificationLevel::Info);
                }
                
                if (ImGui::MenuItem("Circle Collider")) {
                    auto collider = ECS::ColliderComponent::CreateCircle(25.0f);
                    registry.AddComponent<ECS::ColliderComponent>(entity, collider);
                    m_Scene->MarkDirty();
                    NotificationBus::Get().Notify("Circle Collider added", NotificationLevel::Info);
                }
                
                if (ImGui::MenuItem("Capsule Collider")) {
                    auto collider = ECS::ColliderComponent::CreatePlayer(16.0f, 48.0f);
                    registry.AddComponent<ECS::ColliderComponent>(entity, collider);
                    m_Scene->MarkDirty();
                    NotificationBus::Get().Notify("Capsule Collider added", NotificationLevel::Info);
                }
                
                ImGui::EndMenu();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Collision shape");
            }
        }
        
        ImGui::SeparatorText("Effects Components");
        
        if (!registry.HasComponent<ECS::ParticleSystemComponent>(entity)) {
            if (ImGui::MenuItem("Particle System")) {
                registry.AddComponent<ECS::ParticleSystemComponent>(entity, ECS::ParticleSystemComponent{});
                m_Scene->MarkDirty();
                NotificationBus::Get().Notify("Particle System Component added", NotificationLevel::Info);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Particle effects (fire, smoke, sparks, etc.)");
            }
        }
        
        ImGui::EndPopup();
    }
}

void InspectorPanel::RenderColliderComponent(ECS::Entity entity) {
    if (!m_Scene) return;
    
    auto& registry = m_Scene->GetECS().GetRegistry();
    if (!registry.HasComponent<ECS::ColliderComponent>(entity)) {
        return;
    }
    
    auto* collider = registry.GetComponent<ECS::ColliderComponent>(entity);
    if (!collider) return;
    
    ImGui::PushID("ColliderComponent");
    bool headerOpen = ImGui::CollapsingHeader("Collider Component", ImGuiTreeNodeFlags_DefaultOpen);
    
    // Remove button
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    if (ImGui::SmallButton("X##RemoveCollider")) {
        registry.RemoveComponent<ECS::ColliderComponent>(entity);
        m_Scene->MarkDirty();
        NotificationBus::Get().Notify("Collider Component removed", NotificationLevel::Info);
        ImGui::PopID();
        return;
    }
    
    if (headerOpen) {
        // Type selection
        const char* typeNames[] = {"Circle", "Box", "Capsule", "Polygon", "Compound"};
        int currentType = static_cast<int>(collider->GetType());
        
        if (ImGui::Combo("Type", &currentType, typeNames, IM_ARRAYSIZE(typeNames))) {
            // Change type (recreate collider)
            ECS::ColliderComponent newCollider;
            switch (currentType) {
                case 0: newCollider = ECS::ColliderComponent::CreateCircle(25.0f); break;
                case 1: newCollider = ECS::ColliderComponent::CreateBox(Vector2(50, 50)); break;
                case 2: newCollider = ECS::ColliderComponent::CreatePlayer(16.0f, 48.0f); break;
                case 3: {
                    std::vector<Vector2> verts = {{-25,-25}, {25,-25}, {25,25}, {-25,25}};
                    newCollider = ECS::ColliderComponent::CreatePolygon(verts);
                    break;
                }
                case 4: {
                    // Create empty compound with single box child
                    std::vector<ECS::ColliderComponent::SubCollider> children;
                    ECS::ColliderComponent::SubCollider child;
                    child.type = ECS::ColliderComponent::Type::Box;
                    child.size = Vector2(25, 25);
                    children.push_back(child);
                    newCollider = ECS::ColliderComponent::CreateCompound(children);
                    break;
                }
            }
            *collider = newCollider;
            m_Scene->MarkDirty();
        }
        
        // Offset
        Vector2 offset = collider->GetOffset();
        float offsetArr[2] = {offset.x, offset.y};
        if (ImGui::DragFloat2("Offset", offsetArr, 1.0f)) {
            collider->SetOffset(Vector2(offsetArr[0], offsetArr[1]));
            m_Scene->MarkDirty();
        }
        
        // Type-specific properties
        auto type = collider->GetType();
        
        if (type == ECS::ColliderComponent::Type::Circle) {
            float radius = collider->GetCircleRadius();
            if (ImGui::DragFloat("Radius", &radius, 1.0f, 1.0f, 1000.0f)) {
                collider->SetCircleRadius(radius);
                m_Scene->MarkDirty();
            }
        }
        else if (type == ECS::ColliderComponent::Type::Box) {
            Vector2 size = collider->GetBoxSize();
            float sizeArr[2] = {size.x, size.y};
            if (ImGui::DragFloat2("Size", sizeArr, 1.0f, 1.0f, 1000.0f)) {
                collider->SetBoxSize(Vector2(sizeArr[0], sizeArr[1]));
                m_Scene->MarkDirty();
            }
        }
        else if (type == ECS::ColliderComponent::Type::Capsule) {
            float radius = collider->GetCapsuleRadius();
            float height = collider->GetCapsuleHeight();
            
            if (ImGui::DragFloat("Radius", &radius, 1.0f, 1.0f, 100.0f)) {
                collider->SetCapsuleRadius(radius);
                m_Scene->MarkDirty();
            }
            if (ImGui::DragFloat("Height", &height, 1.0f, 1.0f, 500.0f)) {
                collider->SetCapsuleHeight(height);
                m_Scene->MarkDirty();
            }
        }
        
        // Physical materials
        ImGui::Separator();
        ImGui::Text("Physical Materials");
        
        float friction = collider->GetFriction();
        if (ImGui::SliderFloat("Friction", &friction, 0.0f, 1.0f)) {
            collider->SetFriction(friction);
            m_Scene->MarkDirty();
        }
        
        float restitution = collider->GetRestitution();
        if (ImGui::SliderFloat("Restitution", &restitution, 0.0f, 1.0f)) {
            collider->SetRestitution(restitution);
            m_Scene->MarkDirty();
        }
        
        float density = collider->GetDensity();
        if (ImGui::DragFloat("Density", &density, 0.1f, 0.01f, 100.0f)) {
            collider->SetDensity(density);
            m_Scene->MarkDirty();
        }
        
        // Trigger
        bool isTrigger = collider->IsTrigger();
        if (ImGui::Checkbox("Is Trigger", &isTrigger)) {
            collider->SetTrigger(isTrigger);
            m_Scene->MarkDirty();
        }
    }
    
    ImGui::PopID();
}

void InspectorPanel::RenderCameraComponent(ECS::Entity entity) {
    // TODO: Implement CameraComponent
    /*
    if (!m_Scene) return;
    
    auto& registry = m_Scene->GetECS().GetRegistry();
    if (!registry.HasComponent<ECS::CameraComponent>(entity)) {
        return;
    }
    
    auto* camera = registry.GetComponent<ECS::CameraComponent>(entity);
    if (!camera) return;
    
    // TODO: Implement CameraComponent
    /*
    ImGui::PushID("CameraComponent");
    bool headerOpen = ImGui::CollapsingHeader("Camera Component", ImGuiTreeNodeFlags_DefaultOpen);
    
    // Remove button
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    if (ImGui::SmallButton("X##RemoveCamera")) {
        registry.RemoveComponent<ECS::CameraComponent>(entity);
        m_Scene->MarkDirty();
        NotificationBus::Get().Notify("Camera Component removed", NotificationLevel::Info);
        ImGui::PopID();
        return;
    }
    
    if (headerOpen) {
        if (ImGui::DragFloat("Width", &camera->width, 10.0f, 100.0f, 4096.0f)) {
            m_Scene->MarkDirty();
        }
        
        if (ImGui::DragFloat("Height", &camera->height, 10.0f, 100.0f, 4096.0f)) {
            m_Scene->MarkDirty();
        }
        
        if (ImGui::DragFloat("Zoom", &camera->zoom, 0.1f, 0.1f, 10.0f)) {
            m_Scene->MarkDirty();
        }
        
        bool isMain = camera->isMainCamera;
        if (ImGui::Checkbox("Main Camera", &isMain)) {
            camera->isMainCamera = isMain;
            m_Scene->MarkDirty();
        }
    }
    
    ImGui::PopID();
    */
}

void InspectorPanel::RenderParticleSystemComponent(ECS::Entity entity) {
    if (!m_Scene) return;
    
    auto& registry = m_Scene->GetECS().GetRegistry();
    if (!registry.HasComponent<ECS::ParticleSystemComponent>(entity)) {
        return;
    }
    
    auto* particleComp = registry.GetComponent<ECS::ParticleSystemComponent>(entity);
    if (!particleComp) return;
    
    ImGui::PushID("ParticleSystemComponent");
    bool headerOpen = ImGui::CollapsingHeader("Particle System Component", ImGuiTreeNodeFlags_DefaultOpen);
    
    // Remove button
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    if (ImGui::SmallButton("X##RemoveParticleSystem")) {
        registry.RemoveComponent<ECS::ParticleSystemComponent>(entity);
        m_Scene->MarkDirty();
        NotificationBus::Get().Notify("Particle System Component removed", NotificationLevel::Info);
        ImGui::PopID();
        return;
    }
    
    if (headerOpen) {
        // Control buttons
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
        if (particleComp->IsPlaying()) {
            if (ImGui::Button("Stop", ImVec2(160, 0))) {
                particleComp->Stop();
            }
        } else {
            if (ImGui::Button("Play", ImVec2(80, 0))) {
                particleComp->Play();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset", ImVec2(80, 0))) {
                particleComp->Reset();
            }
        }
        ImGui::PopStyleVar();
        
        ImGui::Separator();
        
        // Emission settings
        if (ImGui::TreeNodeEx("Emission", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::DragFloat("Emission Rate", &particleComp->config.emissionRate, 1.0f, 0.0f, 1000.0f, "%.1f particles/sec")) {
                m_Scene->MarkDirty();
            }
            
            int maxParticles = static_cast<int>(particleComp->config.maxParticles);
            if (ImGui::DragInt("Max Particles", &maxParticles, 10, 1, 10000)) {
                particleComp->config.maxParticles = static_cast<std::size_t>(maxParticles);
                m_Scene->MarkDirty();
            }
            
            if (ImGui::Checkbox("Looping", &particleComp->config.looping)) {
                m_Scene->MarkDirty();
            }
            
            if (!particleComp->config.looping) {
                if (ImGui::DragFloat("Duration", &particleComp->config.duration, 0.1f, 0.0f, 60.0f, "%.1f sec")) {
                    m_Scene->MarkDirty();
                }
            }
            
            ImGui::TreePop();
        }
        
        // Lifetime settings
        if (ImGui::TreeNodeEx("Lifetime", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::DragFloat("Min Lifetime", &particleComp->config.minLifetime, 0.01f, 0.01f, particleComp->config.maxLifetime, "%.2f sec")) {
                m_Scene->MarkDirty();
            }
            
            if (ImGui::DragFloat("Max Lifetime", &particleComp->config.maxLifetime, 0.01f, particleComp->config.minLifetime, 10.0f, "%.2f sec")) {
                m_Scene->MarkDirty();
            }
            
            ImGui::TreePop();
        }
        
        // Position settings
        if (ImGui::TreeNodeEx("Position")) {
            float posVariance[2] = {particleComp->config.positionVariance.x, particleComp->config.positionVariance.y};
            if (ImGui::DragFloat2("Variance", posVariance, 1.0f, 0.0f, 200.0f)) {
                particleComp->config.positionVariance = Vector2(posVariance[0], posVariance[1]);
                m_Scene->MarkDirty();
            }
            
            if (ImGui::DragFloat("Spawn Radius", &particleComp->config.spawnRadius, 0.5f, 0.0f, 100.0f)) {
                m_Scene->MarkDirty();
            }
            
            ImGui::TreePop();
        }
        
        // Velocity settings
        if (ImGui::TreeNodeEx("Velocity")) {
            float velMin[2] = {particleComp->config.velocityMin.x, particleComp->config.velocityMin.y};
            if (ImGui::DragFloat2("Min Velocity", velMin, 1.0f, -500.0f, 500.0f)) {
                particleComp->config.velocityMin = Vector2(velMin[0], velMin[1]);
                m_Scene->MarkDirty();
            }
            
            float velMax[2] = {particleComp->config.velocityMax.x, particleComp->config.velocityMax.y};
            if (ImGui::DragFloat2("Max Velocity", velMax, 1.0f, -500.0f, 500.0f)) {
                particleComp->config.velocityMax = Vector2(velMax[0], velMax[1]);
                m_Scene->MarkDirty();
            }
            
            float accel[2] = {particleComp->config.acceleration.x, particleComp->config.acceleration.y};
            if (ImGui::DragFloat2("Acceleration", accel, 1.0f, -200.0f, 200.0f)) {
                particleComp->config.acceleration = Vector2(accel[0], accel[1]);
                m_Scene->MarkDirty();
            }
            
            ImGui::TreePop();
        }
        
        // Size settings
        if (ImGui::TreeNodeEx("Size", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::DragFloat("Start Size", &particleComp->config.startSize, 0.5f, 0.1f, 100.0f)) {
                m_Scene->MarkDirty();
            }
            
            if (ImGui::DragFloat("End Size", &particleComp->config.endSize, 0.5f, 0.0f, 100.0f)) {
                m_Scene->MarkDirty();
            }
            
            if (ImGui::DragFloat("Size Variance", &particleComp->config.sizeVariance, 0.1f, 0.0f, 20.0f)) {
                m_Scene->MarkDirty();
            }
            
            ImGui::TreePop();
        }
        
        // Color settings
        if (ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
            float startColor[4] = {
                particleComp->config.startColor.r,
                particleComp->config.startColor.g,
                particleComp->config.startColor.b,
                particleComp->config.startColor.a
            };
            if (ImGui::ColorEdit4("Start Color", startColor)) {
                particleComp->config.startColor = Color(
                    std::clamp(startColor[0], 0.0f, 1.0f),
                    std::clamp(startColor[1], 0.0f, 1.0f),
                    std::clamp(startColor[2], 0.0f, 1.0f),
                    std::clamp(startColor[3], 0.0f, 1.0f)
                );
                m_Scene->MarkDirty();
            }
            
            float endColor[4] = {
                particleComp->config.endColor.r,
                particleComp->config.endColor.g,
                particleComp->config.endColor.b,
                particleComp->config.endColor.a
            };
            if (ImGui::ColorEdit4("End Color", endColor)) {
                particleComp->config.endColor = Color(
                    std::clamp(endColor[0], 0.0f, 1.0f),
                    std::clamp(endColor[1], 0.0f, 1.0f),
                    std::clamp(endColor[2], 0.0f, 1.0f),
                    std::clamp(endColor[3], 0.0f, 1.0f)
                );
                m_Scene->MarkDirty();
            }
            
            ImGui::TreePop();
        }
        
        // Rotation settings
        if (ImGui::TreeNodeEx("Rotation")) {
            if (ImGui::DragFloat("Min Rotation", &particleComp->config.rotationMin, 1.0f, 0.0f, 360.0f, "%.1f°")) {
                m_Scene->MarkDirty();
            }
            
            if (ImGui::DragFloat("Max Rotation", &particleComp->config.rotationMax, 1.0f, 0.0f, 360.0f, "%.1f°")) {
                m_Scene->MarkDirty();
            }
            
            if (ImGui::DragFloat("Min Angular Vel", &particleComp->config.angularVelocityMin, 1.0f, -360.0f, 360.0f, "%.1f°/sec")) {
                m_Scene->MarkDirty();
            }
            
            if (ImGui::DragFloat("Max Angular Vel", &particleComp->config.angularVelocityMax, 1.0f, -360.0f, 360.0f, "%.1f°/sec")) {
                m_Scene->MarkDirty();
            }
            
            ImGui::TreePop();
        }
        
        // Settings
        ImGui::Separator();
        if (ImGui::Checkbox("Play on Start", &particleComp->playOnStart)) {
            m_Scene->MarkDirty();
        }
        
        if (ImGui::Checkbox("Auto Destroy", &particleComp->autoDestroy)) {
            m_Scene->MarkDirty();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Destroy entity when particles finish (non-looping only)");
        }
    }
    
    ImGui::PopID();
}

} // namespace Editor
} // namespace SAGE
