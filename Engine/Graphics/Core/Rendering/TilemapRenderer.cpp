#include "TilemapRenderer.h"
#include "ECS/Components/TilemapComponent.h"
#include "Graphics/Core/Camera2D.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Resources/Font.h"
#include "Graphics/API/Renderer.h"
#include "Core/Log.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

namespace SAGE {

    namespace {
        constexpr uint32_t kFlipHorizontalFlag = 0x80000000u;
        constexpr uint32_t kFlipVerticalFlag   = 0x40000000u;
        constexpr uint32_t kFlipDiagonalFlag   = 0x20000000u;
        constexpr uint32_t kFlipHexRotationFlag = 0x10000000u; // 120° rotations (hex/iso staggered maps)
        constexpr uint32_t kFlipMask = kFlipHorizontalFlag | kFlipVerticalFlag | kFlipDiagonalFlag | kFlipHexRotationFlag;

        // Simple time source (milliseconds since start); could be replaced by engine clock
        inline uint64_t GetTimeMs() {
            using namespace std::chrono;
            static const auto start = steady_clock::now();
            return duration_cast<milliseconds>(steady_clock::now() - start).count();
        }

        struct TileTransform {
            float rotationDeg = 0.0f;
            bool flipX = false;
            bool flipY = false;
        };

        bool IsStaggeredIndex(int index, ECS::TilemapStaggerIndex mode) {
            if (mode == ECS::TilemapStaggerIndex::None) {
                return false;
            }
            const bool isOdd = (index & 1) != 0;
            return (mode == ECS::TilemapStaggerIndex::Odd) ? isOdd : !isOdd;
        }

        Float2 ComputeTileWorldPosition(const ECS::TilemapComponent& tilemap,
                                        int tileX,
                                        int tileY,
                                        const Float2& basePosition) {
            using Orientation = ECS::TilemapOrientation;
            using StaggerAxis = ECS::TilemapStaggerAxis;

            const float tileWidth = static_cast<float>(tilemap.tileWidth);
            const float tileHeight = static_cast<float>(tilemap.tileHeight);

            switch (tilemap.orientation) {
                case Orientation::Orthogonal: {
                    return { basePosition.x + tileX * tileWidth,
                             basePosition.y + tileY * tileHeight };
                }
                case Orientation::Isometric: {
                    const float halfWidth = tileWidth * 0.5f;
                    const float halfHeight = tileHeight * 0.5f;
                    const float originX = (static_cast<float>(tilemap.mapHeight) - 1.0f) * halfWidth;
                    return {
                        basePosition.x + (static_cast<float>(tileX - tileY) * halfWidth) + originX,
                        basePosition.y + (static_cast<float>(tileX + tileY) * halfHeight)
                    };
                }
                case Orientation::Staggered: {
                    const StaggerAxis axis = tilemap.staggerAxis;
                    if (axis == StaggerAxis::Y) {
                        const float halfWidth = tileWidth * 0.5f;
                        const float rowHeight = tileHeight * 0.5f;
                        const bool staggered = IsStaggeredIndex(tileY, tilemap.staggerIndex);
                        const float offsetX = staggered ? halfWidth : 0.0f;
                        return {
                            basePosition.x + tileX * tileWidth + offsetX,
                            basePosition.y + tileY * rowHeight
                        };
                    } else if (axis == StaggerAxis::X) {
                        const float halfHeight = tileHeight * 0.5f;
                        const float columnWidth = tileWidth * 0.5f;
                        const bool staggered = IsStaggeredIndex(tileX, tilemap.staggerIndex);
                        const float offsetY = staggered ? halfHeight : 0.0f;
                        return {
                            basePosition.x + tileX * columnWidth,
                            basePosition.y + tileY * tileHeight + offsetY
                        };
                    }
                    break;
                }
                case Orientation::Hexagonal: {
                    const StaggerAxis axis = tilemap.staggerAxis;
                    const float sideLength = static_cast<float>(std::max(0, tilemap.hexSideLength));

                    if (axis == StaggerAxis::Y) {
                        const float sideOffset = (tileHeight - sideLength) * 0.5f;
                        const float rowHeight = sideLength + sideOffset;
                        const bool staggered = IsStaggeredIndex(tileY, tilemap.staggerIndex);
                        const float offsetX = staggered ? tileWidth * 0.5f : 0.0f;
                        return {
                            basePosition.x + tileX * tileWidth + offsetX,
                            basePosition.y + tileY * rowHeight
                        };
                    }

                    if (axis == StaggerAxis::X) {
                        const float sideOffset = (tileWidth - sideLength) * 0.5f;
                        const float columnWidth = sideLength + sideOffset;
                        const bool staggered = IsStaggeredIndex(tileX, tilemap.staggerIndex);
                        const float offsetY = staggered ? tileHeight * 0.5f : 0.0f;
                        return {
                            basePosition.x + tileX * columnWidth,
                            basePosition.y + tileY * tileHeight + offsetY
                        };
                    }
                    break;
                }
            }

            return { basePosition.x + tileX * tileWidth,
                     basePosition.y + tileY * tileHeight };
        }

        Float2 ConvertWorldToTileIndices(const ECS::TilemapComponent& tilemap,
                                         const Float2& worldPosition,
                                         const Float2& basePosition) {
            using Orientation = ECS::TilemapOrientation;
            using StaggerAxis = ECS::TilemapStaggerAxis;

            const float tileWidth = static_cast<float>(tilemap.tileWidth);
            const float tileHeight = static_cast<float>(tilemap.tileHeight);
            if (tileWidth <= 0.0f || tileHeight <= 0.0f) {
                return {0.0f, 0.0f};
            }

            const float relX = worldPosition.x - basePosition.x;
            const float relY = worldPosition.y - basePosition.y;

            switch (tilemap.orientation) {
                case Orientation::Orthogonal: {
                    return { relX / tileWidth, relY / tileHeight };
                }
                case Orientation::Isometric: {
                    const float halfWidth = tileWidth * 0.5f;
                    const float halfHeight = tileHeight * 0.5f;
                    const float originX = (static_cast<float>(tilemap.mapHeight) - 1.0f) * halfWidth;
                    const float adjustedX = relX - originX;
                    const float tx = (adjustedX / halfWidth + relY / halfHeight) * 0.5f;
                    const float ty = (relY / halfHeight - adjustedX / halfWidth) * 0.5f;
                    return { tx, ty };
                }
                case Orientation::Staggered: {
                    const StaggerAxis axis = tilemap.staggerAxis;
                    if (axis == StaggerAxis::Y) {
                        const float halfWidth = tileWidth * 0.5f;
                        const float rowHeight = tileHeight * 0.5f;
                        const float tileY = relY / rowHeight;
                        const int intY = static_cast<int>(std::floor(tileY));
                        const bool staggered = IsStaggeredIndex(intY, tilemap.staggerIndex);
                        const float offsetX = staggered ? halfWidth : 0.0f;
                        const float tileX = (relX - offsetX) / tileWidth;
                        return { tileX, tileY };
                    } else if (axis == StaggerAxis::X) {
                        const float halfHeight = tileHeight * 0.5f;
                        const float columnWidth = tileWidth * 0.5f;
                        const float tileX = relX / columnWidth;
                        const int intX = static_cast<int>(std::floor(tileX));
                        const bool staggered = IsStaggeredIndex(intX, tilemap.staggerIndex);
                        const float offsetY = staggered ? halfHeight : 0.0f;
                        const float tileY = (relY - offsetY) / tileHeight;
                        return { tileX, tileY };
                    }
                    break;
                }
                case Orientation::Hexagonal: {
                    const StaggerAxis axis = tilemap.staggerAxis;
                    const float sideLength = static_cast<float>(std::max(0, tilemap.hexSideLength));

                    if (axis == StaggerAxis::Y) {
                        const float sideOffset = (tileHeight - sideLength) * 0.5f;
                        const float rowHeight = sideLength + sideOffset;
                        const float tileY = relY / rowHeight;
                        const int intY = static_cast<int>(std::floor(tileY));
                        const bool staggered = IsStaggeredIndex(intY, tilemap.staggerIndex);
                        const float offsetX = staggered ? tileWidth * 0.5f : 0.0f;
                        const float tileX = (relX - offsetX) / tileWidth;
                        return { tileX, tileY };
                    }

                    if (axis == StaggerAxis::X) {
                        const float sideOffset = (tileWidth - sideLength) * 0.5f;
                        const float columnWidth = sideLength + sideOffset;
                        const float tileX = relX / columnWidth;
                        const int intX = static_cast<int>(std::floor(tileX));
                        const bool staggered = IsStaggeredIndex(intX, tilemap.staggerIndex);
                        const float offsetY = staggered ? tileHeight * 0.5f : 0.0f;
                        const float tileY = (relY - offsetY) / tileHeight;
                        return { tileX, tileY };
                    }
                    break;
                }
            }

            return { relX / tileWidth, relY / tileHeight };
        }

        TileTransform ResolveTileTransform(bool flipHorizontal, bool flipVertical, bool flipDiagonal) {
            // Lookup table for 8 combinations of flip flags (3 bits = 8 states)
            // Format: [diagonal][vertical][horizontal]
            static const TileTransform kFlipTransforms[8] = {
                {0.0f,   false, false}, // 000: no flips
                {0.0f,   true,  false}, // 001: flip H
                {0.0f,   false, true},  // 010: flip V
                {180.0f, false, false}, // 011: flip H+V = rotate 180
                {270.0f, false, false}, // 100: flip D = rotate 270 (swap UV)
                {90.0f,  false, true},  // 101: flip D+H = rotate 90 + flip V
                {270.0f, false, true},  // 110: flip D+V = rotate 270 + flip V
                {90.0f,  false, false}  // 111: flip D+H+V = rotate 90
            };
            
            const int index = (flipDiagonal ? 4 : 0) | (flipVertical ? 2 : 0) | (flipHorizontal ? 1 : 0);
            return kFlipTransforms[index];
        }

        int ResolveAnimatedLocalID(const ECS::TilemapComponent& tilemap, const ECS::TilesetInfo& tileset, int localID, int globalTileID) {
            if (localID < 0 || localID >= (int)tileset.tiles.size()) return localID;
            const auto* def = tileset.GetTileDefinition(localID);
            if (!def || !def->IsAnimated()) return localID;

            // Time-based animation
            static int s_invalidAnimFrameWarnings = 0;
            const int maxWarnings = 16;
            uint64_t timeMs = GetTimeMs();

            int total = 0;
            for (const auto& frame : def->animation) {
                if (frame.localTileID < 0 || frame.localTileID >= tileset.tileCount) {
                    if (s_invalidAnimFrameWarnings < maxWarnings) {
                        SAGE_WARN("TilemapRenderer: Tileset '{}' animation references out-of-range tile {}", tileset.name, frame.localTileID);
                        ++s_invalidAnimFrameWarnings;
                        if (s_invalidAnimFrameWarnings == maxWarnings) {
                            SAGE_WARN("TilemapRenderer: Further animation frame warnings suppressed");
                        }
                    }
                    continue;
                }
                total += std::max(1, frame.durationMs);
            }
            if (total <= 0) return localID;

            int t = (int)(timeMs % total);
            int accum = 0;
            for (const auto& frame : def->animation) {
                if (frame.localTileID < 0 || frame.localTileID >= tileset.tileCount) {
                    continue;
                }
                int dur = std::max(1, frame.durationMs);
                if (t < accum + dur) {
                    return frame.localTileID; // localTileID is local id within same tileset
                }
                accum += dur;
            }
            return localID;
        }
        Color MultiplyColor(const Color& a, const Color& b) {
            return Color(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
        }
    }

    void TilemapRenderer::Render(const ECS::TilemapComponent& tilemap, const Float2& position, const Camera2D& camera) {
        if (!tilemap.IsValid()) return;

        // Render all tile layers in order
        for (int i = 0; i < static_cast<int>(tilemap.layers.size()); ++i) {
            RenderTileLayer(tilemap, i, position, camera);
        }
        
        // Render object layers
        for (int i = 0; i < static_cast<int>(tilemap.objectLayers.size()); ++i) {
            RenderObjectLayer(tilemap, i, position, camera);
        }
        
        // Render image layers
        for (int i = 0; i < static_cast<int>(tilemap.imageLayers.size()); ++i) {
            RenderImageLayer(tilemap, i, position, camera);
        }
    }

    void TilemapRenderer::RenderLayer(const ECS::TilemapComponent& tilemap, int layerIndex, const Float2& position, const Camera2D& camera) {
        RenderTileLayer(tilemap, layerIndex, position, camera);
    }

    void TilemapRenderer::RenderTileLayer(const ECS::TilemapComponent& tilemap, int layerIndex, const Float2& position, const Camera2D& camera) {
        if (layerIndex < 0 || layerIndex >= (int)tilemap.layers.size()) return;
        const auto& layer = tilemap.layers[layerIndex];
        if (!layer.visible || layer.tiles.empty()) return;

        // Apply parallax offset to position
        Float2 parallaxOffset;
        parallaxOffset.x = camera.GetPosition().x * (1.0f - layer.parallaxFactor.x);
        parallaxOffset.y = camera.GetPosition().y * (1.0f - layer.parallaxFactor.y);
    Float2 adjustedPosition = position;
    adjustedPosition.x += parallaxOffset.x + layer.offset.x;
    adjustedPosition.y += parallaxOffset.y + layer.offset.y;

        int minX, minY, maxX, maxY;
        ComputeVisibleRange(tilemap, layerIndex, adjustedPosition, camera, minX, minY, maxX, maxY);

        // Clamp to layer bounds (for non-infinite maps)
        if (!tilemap.infinite) {
            minX = std::max(0, minX);
            minY = std::max(0, minY);
            maxX = std::min(layer.width - 1, maxX);
            maxY = std::min(layer.height - 1, maxY);
        }

        static bool s_WarnedHexRotation = false;
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                // Use GetTile for chunk-aware access
                int tileValue = layer.GetTile(x, y);
                if (tileValue < 0) continue; // empty or out of bounds
                
                const uint32_t rawGid = static_cast<uint32_t>(tileValue);
                const uint32_t normalizedGid = rawGid & ~kFlipMask;
                if (normalizedGid == 0u) {
                    continue; // Empty tile
                }

                if (!s_WarnedHexRotation && (rawGid & kFlipHexRotationFlag)) {
                    SAGE_WARNING("TilemapRenderer: encountered tile with 120-degree rotation flag. Hexagonal staggered rotations are not supported yet.");
                    s_WarnedHexRotation = true;
                }

                if (normalizedGid > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
                    SAGE_WARNING("TilemapRenderer: normalized GID {} exceeds supported range", normalizedGid);
                    continue;
                }

                int gid = static_cast<int>(normalizedGid);
                const bool flipHorizontal = (rawGid & kFlipHorizontalFlag) != 0u;
                const bool flipVertical = (rawGid & kFlipVerticalFlag) != 0u;
                const bool flipDiagonal = (rawGid & kFlipDiagonalFlag) != 0u;

                Float2 uvMin, uvMax;
                Ref<Texture> texture;
                // Animated resolution: find tileset, compute local id, substitute animated frame id
                int tilesetIdx = FindTilesetIndex(gid, tilemap);
                const ECS::TilesetInfo* activeTileset = nullptr;
                if (tilesetIdx >= 0) {
                    const auto& ts = tilemap.tilesets[tilesetIdx];
                    activeTileset = &ts;
                    int localID = gid - ts.firstGID;
                    localID = ResolveAnimatedLocalID(tilemap, ts, localID, gid);
                    gid = ts.firstGID + localID;
                }
                if (!GetTileUV(gid, tilemap, uvMin, uvMax, texture)) continue;
                if (!texture) continue;

                // Compute tile origin based on map orientation
                Float2 tilePos = ComputeTileWorldPosition(tilemap, x, y, adjustedPosition);
                
                // Apply tileset tile offset if available
                if (activeTileset) {
                    tilePos.x += activeTileset->tileOffset.x;
                    tilePos.y += activeTileset->tileOffset.y;
                }

                Float2 tileSize((float)tilemap.tileWidth, (float)tilemap.tileHeight);

                const TileTransform transform = ResolveTileTransform(flipHorizontal, flipVertical, flipDiagonal);
                float rotation = transform.rotationDeg;
                bool applyFlipH = transform.flipX;
                bool applyFlipV = transform.flipY;

                if (flipDiagonal) {
                    const float halfDiff = 0.5f * (tileSize.y - tileSize.x);
                    tilePos.x += halfDiff;
                    tilePos.y += halfDiff;
                }

                if (applyFlipH) {
                    std::swap(uvMin.x, uvMax.x);
                }
                if (applyFlipV) {
                    std::swap(uvMin.y, uvMax.y);
                }

                QuadDesc quad{};
                quad.position = tilePos;
                quad.size = tileSize;
                const float clampedOpacity = std::clamp(layer.opacity, 0.0f, 1.0f);
                Color color = MultiplyColor(layer.tint, Color(1.0f, 1.0f, 1.0f, clampedOpacity));
                quad.color = color;
                quad.texture = texture;
                quad.uvMin = uvMin;
                quad.uvMax = uvMax;
                quad.rotation = rotation;
                quad.screenSpace = false;
                quad.source = QuadDesc::QuadSource::Tile; // explicit tile classification
                

                Renderer::DrawQuad(quad);
            }
        }
    }

    void TilemapRenderer::RenderObjectLayer(const ECS::TilemapComponent& tilemap, int objectLayerIndex, const Float2& position, const Camera2D& camera) {
        if (objectLayerIndex < 0 || objectLayerIndex >= static_cast<int>(tilemap.objectLayers.size())) return;
        const auto& layer = tilemap.objectLayers[objectLayerIndex];
        if (!layer.visible || layer.sprites.empty()) return;

        Float2 parallaxOffset;
        parallaxOffset.x = camera.GetPosition().x * (1.0f - layer.parallaxFactor.x);
        parallaxOffset.y = camera.GetPosition().y * (1.0f - layer.parallaxFactor.y);
        Float2 adjustedPosition = position;
        adjustedPosition.x += parallaxOffset.x + layer.offset.x;
        adjustedPosition.y += parallaxOffset.y + layer.offset.y;

        auto bounds = camera.GetWorldBounds();
        const float clampedOpacity = std::clamp(layer.opacity, 0.0f, 1.0f);
        Color layerColor = MultiplyColor(layer.tint, Color(1.0f, 1.0f, 1.0f, clampedOpacity));

        static bool s_WarnedHexRotationObjects = false;
        for (const auto& sprite : layer.sprites) {
            if (!sprite.visible) {
                continue;
            }

            const uint32_t rawGid = sprite.gid;
            const uint32_t normalizedGid = rawGid & ~kFlipMask;
            Color spriteColor = MultiplyColor(layerColor, sprite.tint);
            if (normalizedGid == 0u) {
                Float2 spritePos;
                spritePos.x = adjustedPosition.x + sprite.position.x;
                spritePos.y = adjustedPosition.y + sprite.position.y;
                Float2 spriteSize = sprite.size;

                if (sprite.shape == ECS::TilemapObjectShape::Rectangle) {
                    QuadDesc quad{};
                    quad.position = spritePos;
                    quad.size = spriteSize;
                    quad.color = spriteColor;
                    quad.color.a *= 0.5f;
                    quad.rotation = sprite.rotation;
                    quad.screenSpace = false;
                    Renderer::DrawQuad(quad);
                } else if (sprite.shape == ECS::TilemapObjectShape::Ellipse) {
                    QuadDesc quad{};
                    quad.position = spritePos;
                    quad.size = spriteSize;
                    quad.color = spriteColor;
                    quad.color.a *= 0.5f;
                    quad.rotation = sprite.rotation;
                    quad.screenSpace = false;
                    Renderer::DrawQuad(quad);
                } else if (sprite.shape == ECS::TilemapObjectShape::Polygon || sprite.shape == ECS::TilemapObjectShape::Polyline) {
                    float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;
                    bool first = true;
                    for (const auto& pt : sprite.points) {
                        if (first) {
                            minX = maxX = pt.x;
                            minY = maxY = pt.y;
                            first = false;
                        } else {
                            minX = std::min(minX, pt.x);
                            minY = std::min(minY, pt.y);
                            maxX = std::max(maxX, pt.x);
                            maxY = std::max(maxY, pt.y);
                        }
                    }
                    if (first) {
                        continue;
                    }
                    QuadDesc quad{};
                    quad.position = { spritePos.x + minX, spritePos.y + minY };
                    quad.size = { std::max(0.0f, maxX - minX), std::max(0.0f, maxY - minY) };
                    quad.color = spriteColor;
                    quad.color.a *= 0.35f;
                    quad.rotation = sprite.rotation;
                    quad.screenSpace = false;
                    Renderer::DrawQuad(quad);
                } else if (sprite.shape == ECS::TilemapObjectShape::Point) {
                    QuadDesc quad{};
                    quad.position = { spritePos.x - 2.0f, spritePos.y - 2.0f };
                    quad.size = { 4.0f, 4.0f };
                    quad.color = Color(1.0f, 0.2f, 0.2f, spriteColor.a);
                    quad.screenSpace = false;
                    Renderer::DrawQuad(quad);
                } else if (sprite.shape == ECS::TilemapObjectShape::Text) {
                    // Render text object using Font system
                    if (!sprite.text.empty()) {
                        // Load or use default font
                        // TODO: Cache fonts by fontFamily for performance
                        Ref<Font> font = nullptr;
                        
                        // Try to load font from assets/fonts/<fontFamily>.ttf
                        std::string fontPath = "assets/fonts/" + sprite.fontFamily + ".ttf";
                        try {
                            font = CreateRef<Font>(fontPath, static_cast<float>(sprite.pixelSize));
                        } catch (...) {
                            // Fallback to default font if available
                            // For now, skip text rendering if font not found
                            LOG_WARN("TilemapRenderer: Failed to load font '{}' for text object", sprite.fontFamily);
                        }
                        
                        if (font && font->IsLoaded()) {
                            TextDesc textDesc;
                            textDesc.text = sprite.text;
                            textDesc.position = spritePos;
                            textDesc.font = font;
                            textDesc.scale = 1.0f;
                            textDesc.color = sprite.textColor;
                            textDesc.screenSpace = false;
                            
                            Renderer::DrawText(textDesc);
                        } else {
                            // Fallback: draw background rectangle to show text area
                            float textW = std::max(1.0f, static_cast<float>(sprite.text.size()) * sprite.pixelSize * 0.5f);
                            QuadDesc quad{};
                            quad.position = spritePos;
                            quad.size = { textW, static_cast<float>(sprite.pixelSize) * 1.2f };
                            Color background = spriteColor;
                            background.r = 0.1f;
                            background.g = 0.1f;
                            background.b = 0.1f;
                            background.a *= 0.6f;
                            quad.color = background;
                            quad.screenSpace = false;
                            Renderer::DrawQuad(quad);
                        }
                    }
                }
                continue;
            }

            if (!s_WarnedHexRotationObjects && (rawGid & kFlipHexRotationFlag)) {
                SAGE_WARNING("TilemapRenderer: encountered object with 120-degree rotation flag. Hexagonal staggered rotations are not supported yet.");
                s_WarnedHexRotationObjects = true;
            }

            Float2 uvMin, uvMax;
            Ref<Texture> texture;
            if (!GetTileUV(static_cast<int>(normalizedGid), tilemap, uvMin, uvMax, texture)) {
                continue;
            }
            if (!texture) {
                continue;
            }

            Float2 spritePos;
            spritePos.x = adjustedPosition.x + sprite.position.x;
            spritePos.y = adjustedPosition.y + sprite.position.y;

            Float2 spriteSize = sprite.size;
            if (spriteSize.x <= 0.0f) spriteSize.x = static_cast<float>(tilemap.tileWidth);
            if (spriteSize.y <= 0.0f) spriteSize.y = static_cast<float>(tilemap.tileHeight);

            spritePos.y -= spriteSize.y;

            Float2 worldMin = spritePos;
            Float2 worldMax = { spritePos.x + spriteSize.x, spritePos.y + spriteSize.y };
            if (worldMax.x < bounds.left || worldMin.x > bounds.right || worldMax.y < bounds.bottom || worldMin.y > bounds.top) {
                continue;
            }

            const bool flipHorizontal = (rawGid & kFlipHorizontalFlag) != 0u;
            const bool flipVertical = (rawGid & kFlipVerticalFlag) != 0u;
            const bool flipDiagonal = (rawGid & kFlipDiagonalFlag) != 0u;

            const TileTransform transform = ResolveTileTransform(flipHorizontal, flipVertical, flipDiagonal);
            float rotation = sprite.rotation + transform.rotationDeg;
            bool applyFlipH = transform.flipX;
            bool applyFlipV = transform.flipY;

            if (flipDiagonal) {
                const float halfDiff = 0.5f * (spriteSize.y - spriteSize.x);
                spritePos.x += halfDiff;
                spritePos.y += halfDiff;
            }

            if (applyFlipH) {
                std::swap(uvMin.x, uvMax.x);
            }
            if (applyFlipV) {
                std::swap(uvMin.y, uvMax.y);
            }

            QuadDesc quad{};
            quad.position = spritePos;
            quad.size = spriteSize;
            quad.color = spriteColor;
            quad.texture = texture;
            quad.uvMin = uvMin;
            quad.uvMax = uvMax;
            quad.rotation = rotation;
            quad.screenSpace = false;
            quad.source = QuadDesc::QuadSource::Tile;

            Renderer::DrawQuad(quad);
        }
    }

    void TilemapRenderer::ComputeVisibleRange(const ECS::TilemapComponent& tilemap, int layerIndex,
                                             const Float2& position, const Camera2D& camera,
                                             int& outMinX, int& outMinY, int& outMaxX, int& outMaxY) {
        (void)layerIndex;

        auto computeFullMap = [&]() {
            outMinX = 0;
            outMinY = 0;
            outMaxX = std::max(0, tilemap.mapWidth - 1);
            outMaxY = std::max(0, tilemap.mapHeight - 1);
        };

        const int tileWidth = std::max(1, tilemap.tileWidth);
        const int tileHeight = std::max(1, tilemap.tileHeight);

        switch (tilemap.orientation) {
            case ECS::TilemapOrientation::Orthogonal: {
                if (tilemap.mapWidth <= 0 || tilemap.mapHeight <= 0) {
                    computeFullMap();
                    return;
                }

                auto bounds = camera.GetWorldBounds();
                const float startX = bounds.left - position.x;
                const float startY = bounds.bottom - position.y;
                const float endX = bounds.right - position.x;
                const float endY = bounds.top - position.y;

                const float invTileWidth = 1.0f / static_cast<float>(tileWidth);
                const float invTileHeight = 1.0f / static_cast<float>(tileHeight);

                outMinX = static_cast<int>(std::floor(startX * invTileWidth)) - 1;
                outMinY = static_cast<int>(std::floor(startY * invTileHeight)) - 1;
                outMaxX = static_cast<int>(std::ceil(endX * invTileWidth)) + 1;
                outMaxY = static_cast<int>(std::ceil(endY * invTileHeight)) + 1;
                return;
            }
            case ECS::TilemapOrientation::Isometric: {
                if (tilemap.mapWidth <= 0 || tilemap.mapHeight <= 0) {
                    computeFullMap();
                    return;
                }

                auto bounds = camera.GetWorldBounds();
                const Float2 corners[4] = {
                    {bounds.left,  bounds.bottom},
                    {bounds.left,  bounds.top},
                    {bounds.right, bounds.bottom},
                    {bounds.right, bounds.top}
                };

                float minTileX = static_cast<float>(tilemap.mapWidth);
                float maxTileX = -1.0f;
                float minTileY = static_cast<float>(tilemap.mapHeight);
                float maxTileY = -1.0f;

                for (const Float2& corner : corners) {
                    const Float2 tileCoord = ConvertWorldToTileIndices(tilemap, corner, position);
                    minTileX = std::min(minTileX, tileCoord.x);
                    maxTileX = std::max(maxTileX, tileCoord.x);
                    minTileY = std::min(minTileY, tileCoord.y);
                    maxTileY = std::max(maxTileY, tileCoord.y);
                }

                const int startX = static_cast<int>(std::floor(minTileX)) - 1;
                const int endX = static_cast<int>(std::ceil(maxTileX)) + 1;
                const int startY = static_cast<int>(std::floor(minTileY)) - 1;
                const int endY = static_cast<int>(std::ceil(maxTileY)) + 1;

                outMinX = startX;
                outMinY = startY;
                outMaxX = endX;
                outMaxY = endY;
                return;
            }
            case ECS::TilemapOrientation::Staggered:
            case ECS::TilemapOrientation::Hexagonal: {
                if (tilemap.mapWidth <= 0 || tilemap.mapHeight <= 0) {
                    computeFullMap();
                    return;
                }

                auto bounds = camera.GetWorldBounds();
                const float tileW = static_cast<float>(tileWidth);
                const float tileH = static_cast<float>(tileHeight);
                
                // For hex/staggered, we need to account for the stagger offset
                // Use conservative bounds with extra margin
                const float margin = std::max(tileW, tileH) * 2.0f;
                
                const Float2 corners[4] = {
                    {bounds.left - margin,  bounds.bottom - margin},
                    {bounds.left - margin,  bounds.top + margin},
                    {bounds.right + margin, bounds.bottom - margin},
                    {bounds.right + margin, bounds.top + margin}
                };

                float minTileX = static_cast<float>(tilemap.mapWidth);
                float maxTileX = -1.0f;
                float minTileY = static_cast<float>(tilemap.mapHeight);
                float maxTileY = -1.0f;

                for (const Float2& corner : corners) {
                    const Float2 tileCoord = ConvertWorldToTileIndices(tilemap, corner, position);
                    minTileX = std::min(minTileX, tileCoord.x);
                    maxTileX = std::max(maxTileX, tileCoord.x);
                    minTileY = std::min(minTileY, tileCoord.y);
                    maxTileY = std::max(maxTileY, tileCoord.y);
                }

                const int startX = std::max(0, static_cast<int>(std::floor(minTileX)) - 1);
                const int endX = std::min(tilemap.mapWidth - 1, static_cast<int>(std::ceil(maxTileX)) + 1);
                const int startY = std::max(0, static_cast<int>(std::floor(minTileY)) - 1);
                const int endY = std::min(tilemap.mapHeight - 1, static_cast<int>(std::ceil(maxTileY)) + 1);

                outMinX = startX;
                outMinY = startY;
                outMaxX = endX;
                outMaxY = endY;
                return;
            }
            default:
                break;
        }

        static bool s_warnedFallback = false;
        if (!s_warnedFallback) {
            SAGE_WARN("TilemapRenderer: Falling back to full-map culling for unsupported orientation");
            s_warnedFallback = true;
        }
        computeFullMap();
    }

    bool TilemapRenderer::GetTileUV(int gid, const ECS::TilemapComponent& tilemap,
                                   Float2& outUVMin, Float2& outUVMax, Ref<Texture>& outTexture) {
        int tilesetIdx = FindTilesetIndex(gid, tilemap);
        if (tilesetIdx < 0) return false;

        const auto& tileset = tilemap.tilesets[tilesetIdx];
        static int s_missingTextureWarnings = 0;
        if (!tileset.texture) {
            if (s_missingTextureWarnings < 8) {
                SAGE_WARN("TilemapRenderer: Tileset '{}' requested for GID {} has no texture loaded", tileset.name, gid);
                ++s_missingTextureWarnings;
                if (s_missingTextureWarnings == 8) {
                    SAGE_WARN("TilemapRenderer: Further missing texture warnings suppressed");
                }
            }
            return false;
        }

        if (tileset.tileCount <= 0 || tileset.columns <= 0) {
            SAGE_WARN("TilemapRenderer: Tileset '{}' metadata incomplete (tileCount={}, columns={})", tileset.name, tileset.tileCount, tileset.columns);
            return false;
        }

        int localID = gid - tileset.firstGID;
        if (localID < 0 || localID >= tileset.tileCount) {
            static int s_outOfRangeWarnings = 0;
            if (s_outOfRangeWarnings < 16) {
                SAGE_WARN("TilemapRenderer: GID {} falls outside tileset '{}' range (firstGID={}, tileCount={})", gid, tileset.name, tileset.firstGID, tileset.tileCount);
                ++s_outOfRangeWarnings;
                if (s_outOfRangeWarnings == 16) {
                    SAGE_WARN("TilemapRenderer: Further out-of-range GID warnings suppressed");
                }
            }
            return false;
        }

        const int col = localID % tileset.columns;
        const int row = localID / tileset.columns;

        // Validate texture dimensions first
        const float texWidth = static_cast<float>(tileset.texture->GetWidth());
        const float texHeight = static_cast<float>(tileset.texture->GetHeight());
        if (texWidth <= 0.0f || texHeight <= 0.0f) {
            static bool warnedInvalidTexture = false;
            if (!warnedInvalidTexture) {
                SAGE_ERROR("TilemapRenderer::GetTileUV - Invalid texture dimensions: {} x {}", 
                           texWidth, texHeight);
                warnedInvalidTexture = true;
            }
            return false;
        }

        // Account for optional margin/spacing in atlases exported from Tiled.
        const int margin = tileset.margin;
        const int spacing = tileset.spacing;
        const int tileW = tileset.tileWidth;
        const int tileH = tileset.tileHeight;

        const int pixelX = margin + col * (tileW + spacing);
        const int pixelY = margin + row * (tileH + spacing);

        const float texelOffsetX = 0.5f;
        const float texelOffsetY = 0.5f;

        const float u0 = (pixelX + texelOffsetX) / texWidth;
        const float v0 = (pixelY + texelOffsetY) / texHeight;
        const float u1 = (pixelX + tileW - texelOffsetX) / texWidth;
        const float v1 = (pixelY + tileH - texelOffsetY) / texHeight;

        // Flip vertically because Tiled uses a top-left origin while OpenGL expects bottom-left.
        outUVMin = {
            std::clamp(u0, 0.0f, 1.0f),
            std::clamp(1.0f - v1, 0.0f, 1.0f)
        };
        outUVMax = {
            std::clamp(u1, 0.0f, 1.0f),
            std::clamp(1.0f - v0, 0.0f, 1.0f)
        };
        outTexture = tileset.texture;
        return true;
    }

    int TilemapRenderer::FindTilesetIndex(int gid, const ECS::TilemapComponent& tilemap) {
        // Find tileset that contains this GID
        for (int i = (int)tilemap.tilesets.size() - 1; i >= 0; --i) {
            if (gid >= tilemap.tilesets[i].firstGID) {
                return i;
            }
        }
        return -1;
    }

    void TilemapRenderer::RenderImageLayer(const ECS::TilemapComponent& tilemap, int imageLayerIndex, const Float2& position, const Camera2D& camera) {
        if (imageLayerIndex < 0 || imageLayerIndex >= static_cast<int>(tilemap.imageLayers.size())) {
            return;
        }

        const auto& layer = tilemap.imageLayers[imageLayerIndex];
        if (!layer.visible || layer.opacity <= 0.0f || !layer.texture) {
            return;
        }

        // Apply parallax to layer position
        Float2 parallaxOffset = {
            camera.GetPosition().x * (1.0f - layer.parallaxFactor.x),
            camera.GetPosition().y * (1.0f - layer.parallaxFactor.y)
        };

        Float2 layerPos = {
            position.x + layer.offset.x + parallaxOffset.x,
            position.y + layer.offset.y + parallaxOffset.y
        };

        // Calculate image size from texture
        float imageWidth = static_cast<float>(layer.texture->GetWidth());
        float imageHeight = static_cast<float>(layer.texture->GetHeight());

        // Apply opacity and tint
        Color finalColor = layer.tint;
        finalColor.a *= layer.opacity;

        // Render image
        // If repeatX/repeatY are enabled, we would tile the image here
        // For now, render single image
        QuadDesc quad;
        quad.position = layerPos;
        quad.size = {imageWidth, imageHeight};
        quad.texture = layer.texture;
        quad.color = finalColor;
        quad.rotation = 0.0f;
        quad.screenSpace = false;
        Renderer::DrawQuad(quad);

        // TODO: Implement repeatX/repeatY tiling if needed
        // This would require calculating how many tiles fit in the viewport
        // and rendering multiple instances with appropriate offsets
    }

} // namespace SAGE
