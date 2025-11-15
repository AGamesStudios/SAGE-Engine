#include "TilemapEditor.h"

#ifdef SAGE_HAS_IMGUI
#include "imgui.h"
#endif

namespace SAGE {

    void TilemapEditor::ShowEditor(ECS::TilemapComponent& tilemap, const char* entityName) {
#ifdef SAGE_HAS_IMGUI
        char windowTitle[256];
        snprintf(windowTitle, sizeof(windowTitle), "Tilemap Editor: %s", entityName);
        
        if (ImGui::Begin(windowTitle)) {
            // Stats section
            if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
                ShowStats(tilemap);
            }

            // Layers section
            if (ImGui::CollapsingHeader("Layers", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (int i = 0; i < (int)tilemap.layers.size(); ++i) {
                    ImGui::PushID(i);
                    ShowLayerControls(tilemap.layers[i], i);
                    ImGui::PopID();
                    ImGui::Separator();
                }
            }

            // Tilesets section
            if (ImGui::CollapsingHeader("Tilesets")) {
                for (size_t i = 0; i < tilemap.tilesets.size(); ++i) {
                    const auto& ts = tilemap.tilesets[i];
                    ImGui::Text("Tileset %zu: %s", i, ts.name.c_str());
                    ImGui::Indent();
                    ImGui::Text("First GID: %d", ts.firstGID);
                    ImGui::Text("Tile Size: %dx%d", ts.tileWidth, ts.tileHeight);
                    ImGui::Text("Columns: %d, Count: %d", ts.columns, ts.tileCount);
                    ImGui::Text("Texture: %s", ts.texturePath.c_str());
                    ImGui::Unindent();
                }
            }
        }
        ImGui::End();
#endif
    }

    void TilemapEditor::ShowLayerControls(ECS::TilemapLayer& layer, int layerIndex) {
#ifdef SAGE_HAS_IMGUI
        ImGui::Text("Layer %d: %s", layerIndex, layer.name.c_str());
        
        ImGui::Checkbox("Visible", &layer.visible);
        ImGui::SameLine();
        ImGui::Checkbox("Collision", &layer.collision);

        ImGui::SliderFloat("Opacity", &layer.opacity, 0.0f, 1.0f);
        
        ImGui::DragFloat2("Parallax Factor", &layer.parallaxFactor.x, 0.01f, 0.0f, 2.0f);
        
        ImGui::Text("Size: %dx%d tiles (%d total)", layer.width, layer.height, (int)layer.tiles.size());

        // VBO cache status
        if (layer.vboCached) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "VBO Cached (ID: %u)", layer.vboID);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "VBO Not Cached");
        }
#endif
    }

    void TilemapEditor::ShowStats(const ECS::TilemapComponent& tilemap) {
#ifdef SAGE_HAS_IMGUI
        ImGui::Text("Map Size: %dx%d tiles", tilemap.mapWidth, tilemap.mapHeight);
        ImGui::Text("Tile Size: %dx%d pixels", tilemap.tileWidth, tilemap.tileHeight);
        ImGui::Text("World Size: %dx%d pixels",
                   tilemap.mapWidth * tilemap.tileWidth,
                   tilemap.mapHeight * tilemap.tileHeight);
        ImGui::Text("Layers: %zu", tilemap.layers.size());
        ImGui::Text("Tilesets: %zu", tilemap.tilesets.size());
        
        int totalTiles = 0;
        int visibleLayers = 0;
        int collisionLayers = 0;
        
        for (const auto& layer : tilemap.layers) {
            totalTiles += (int)layer.tiles.size();
            if (layer.visible) visibleLayers++;
            if (layer.collision) collisionLayers++;
        }
        
        ImGui::Text("Total Tiles: %d", totalTiles);
        ImGui::Text("Visible Layers: %d/%zu", visibleLayers, tilemap.layers.size());
        ImGui::Text("Collision Layers: %d", collisionLayers);
#endif
    }

} // namespace SAGE
