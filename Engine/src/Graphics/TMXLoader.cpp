#include "SAGE/Graphics/TMXLoader.h"
#include "SAGE/Core/ResourceManager.h"
#include "SAGE/Log.h"
#include <tinyxml2.h>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace SAGE {

using namespace tinyxml2;

std::shared_ptr<Tilemap> TMXLoader::LoadTMX(const std::string& path, TextureFilter filter) {
    XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        SAGE_ERROR("TMXLoader: Failed to load file '{}'", path);
        return nullptr;
    }

    XMLElement* mapNode = doc.FirstChildElement("map");
    if (!mapNode) {
        SAGE_ERROR("TMXLoader: Invalid TMX file '{}' (no map node)", path);
        return nullptr;
    }

    int width = mapNode->IntAttribute("width");
    int height = mapNode->IntAttribute("height");
    int tileWidth = mapNode->IntAttribute("tilewidth");
    int tileHeight = mapNode->IntAttribute("tileheight");

    auto tilemap = std::make_shared<Tilemap>(width, height, tileWidth, tileHeight);

    // Parse Tilesets
    XMLElement* tilesetNode = mapNode->FirstChildElement("tileset");
    while (tilesetNode) {
        Tileset ts;
        ts.firstGID = tilesetNode->IntAttribute("firstgid");
        
        // Check if it's an external tileset
        const char* source = tilesetNode->Attribute("source");
        if (source) {
            std::filesystem::path tmxPath(path);
            std::filesystem::path tsxPath = tmxPath.parent_path() / source;
            
            XMLDocument tsxDoc;
            if (tsxDoc.LoadFile(tsxPath.string().c_str()) == XML_SUCCESS) {
                XMLElement* tsxRoot = tsxDoc.FirstChildElement("tileset");
                if (tsxRoot) {
                    const char* name = tsxRoot->Attribute("name");
                    ts.name = name ? name : "";
                    ts.tileWidth = tsxRoot->IntAttribute("tilewidth");
                    ts.tileHeight = tsxRoot->IntAttribute("tileheight");
                    ts.spacing = tsxRoot->IntAttribute("spacing");
                    ts.margin = tsxRoot->IntAttribute("margin");
                    ts.tileCount = tsxRoot->IntAttribute("tilecount");
                    ts.columns = tsxRoot->IntAttribute("columns");
                    
                    XMLElement* imageNode = tsxRoot->FirstChildElement("image");
                    if (imageNode) {
                        const char* imgSource = imageNode->Attribute("source");
                        if (imgSource) {
                            // Image path is relative to TSX file
                            std::filesystem::path imgPath = tsxPath.parent_path() / imgSource;
                            
                            // Configure texture for pixel-perfect tilemap rendering
                            auto configTexture = [filter](std::shared_ptr<Texture> tex) {
                                TextureSpec spec;
                                spec.minFilter = filter;
                                spec.magFilter = filter;
                                spec.wrapS = TextureWrap::Clamp;
                                spec.wrapT = TextureWrap::Clamp;
                                spec.generateMipmaps = false; // Disable mipmaps to prevent bleeding
                                spec.flipVertically = false;  // Use standard image coordinates
                                tex->SetSpec(spec);
                            };

                            ts.texture = ResourceManager::Get().Load<Texture>(imgPath.string(), configTexture);

                            if (!ts.texture) {
                                // Fallback: Try finding the image in the same directory as the TSX
                                std::filesystem::path filename = std::filesystem::path(imgSource).filename();
                                std::filesystem::path fallbackPath = tsxPath.parent_path() / filename;
                                SAGE_WARN("TMXLoader: Failed to load texture at '{}', trying fallback '{}'", imgPath.string(), fallbackPath.string());
                                ts.texture = ResourceManager::Get().Load<Texture>(fallbackPath.string(), configTexture);
                            }
                        }
                    }
                }
            } else {
                SAGE_ERROR("TMXLoader: Failed to load external tileset '{}'", tsxPath.string());
            }
        } else {
            // Embedded tileset
            const char* name = tilesetNode->Attribute("name");
            ts.name = name ? name : "";
            ts.tileWidth = tilesetNode->IntAttribute("tilewidth");
            ts.tileHeight = tilesetNode->IntAttribute("tileheight");
            ts.spacing = tilesetNode->IntAttribute("spacing");
            ts.margin = tilesetNode->IntAttribute("margin");
            ts.tileCount = tilesetNode->IntAttribute("tilecount");
            ts.columns = tilesetNode->IntAttribute("columns");

            XMLElement* imageNode = tilesetNode->FirstChildElement("image");
            if (imageNode) {
                const char* imgSource = imageNode->Attribute("source");
                if (imgSource) {
                    std::filesystem::path imgPath = std::filesystem::path(path).parent_path() / imgSource;
                    
                    // Configure texture for pixel-perfect tilemap rendering
                    auto configTexture = [filter](std::shared_ptr<Texture> tex) {
                        TextureSpec spec;
                        spec.minFilter = filter;
                        spec.magFilter = filter;
                        spec.wrapS = TextureWrap::Clamp;
                        spec.wrapT = TextureWrap::Clamp;
                        spec.generateMipmaps = false; // Disable mipmaps to prevent bleeding
                        spec.flipVertically = false;  // Use standard image coordinates
                        tex->SetSpec(spec);
                    };

                    ts.texture = ResourceManager::Get().Load<Texture>(imgPath.string(), configTexture);

                    if (!ts.texture) {
                        // Fallback: Try finding the image in the same directory as the TMX
                        std::filesystem::path filename = std::filesystem::path(imgSource).filename();
                        std::filesystem::path fallbackPath = std::filesystem::path(path).parent_path() / filename;
                        SAGE_WARN("TMXLoader: Failed to load texture at '{}', trying fallback '{}'", imgPath.string(), fallbackPath.string());
                        ts.texture = ResourceManager::Get().Load<Texture>(fallbackPath.string());
                    }

                    if (ts.texture) {
                        ts.texture->SetFilter(filter, filter);
                    }
                }
            }
        }

        if (ts.texture) {
            tilemap->AddTileset(ts);
        }

        tilesetNode = tilesetNode->NextSiblingElement("tileset");
    }

    // Parse Layers
    XMLElement* layerNode = mapNode->FirstChildElement("layer");
    while (layerNode) {
        const char* name = layerNode->Attribute("name");
        // int layerWidth = layerNode->IntAttribute("width");
        // int layerHeight = layerNode->IntAttribute("height");
        float opacity = layerNode->FloatAttribute("opacity", 1.0f);
        bool visible = true;
        if (layerNode->Attribute("visible")) {
            visible = layerNode->BoolAttribute("visible");
        }

        TilemapLayer& layer = tilemap->AddLayer(name ? name : "Layer");
        layer.opacity = opacity;
        layer.visible = visible;

        XMLElement* dataNode = layerNode->FirstChildElement("data");
        if (dataNode) {
            const char* encoding = dataNode->Attribute("encoding");
            
            if (encoding && std::string(encoding) == "csv") {
                // Check for chunks (Infinite map)
                XMLElement* chunkNode = dataNode->FirstChildElement("chunk");
                if (chunkNode) {
                    while (chunkNode) {
                        int chunkX = chunkNode->IntAttribute("x");
                        int chunkY = chunkNode->IntAttribute("y");
                        int chunkWidth = chunkNode->IntAttribute("width");
                        // int chunkHeight = chunkNode->IntAttribute("height");
                        
                        const char* text = chunkNode->GetText();
                        std::string dataStr = text ? text : "";
                        std::stringstream ss(dataStr);
                        std::string segment;
                        int x = 0, y = 0;

                        while (std::getline(ss, segment, ',')) {
                            segment.erase(std::remove_if(segment.begin(), segment.end(), ::isspace), segment.end());
                            if (segment.empty()) continue;

                            unsigned int gid = std::stoul(segment);
                            
                            // Handle flipping flags
                            const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                            const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;
                            const unsigned FLIPPED_DIAGONALLY_FLAG   = 0x20000000;

                            bool flipX = (gid & FLIPPED_HORIZONTALLY_FLAG);
                            bool flipY = (gid & FLIPPED_VERTICALLY_FLAG);
                            bool flipDiag = (gid & FLIPPED_DIAGONALLY_FLAG);

                            gid &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);

                            if (gid > 0) {
                                // Calculate absolute position
                                int absX = chunkX + x;
                                int absY = chunkY + y;
                                
                                // Ensure within bounds (Tilemap is fixed size for now)
                                if (absX >= 0 && absX < tilemap->GetWidth() && absY >= 0 && absY < tilemap->GetHeight()) {
                                    Tile& tile = layer.GetTile(absX, absY, tilemap->GetWidth());
                                    tile = Tile((int)gid, false);
                                    tile.flipX = flipX;
                                    tile.flipY = flipY;
                                    tile.flipDiagonal = flipDiag;
                                }
                            }

                            x++;
                            if (x >= chunkWidth) {
                                x = 0;
                                y++;
                            }
                        }
                        chunkNode = chunkNode->NextSiblingElement("chunk");
                    }
                } else {
                    // Standard fixed map
                    const char* text = dataNode->GetText();
                    std::string dataStr = text ? text : "";
                    std::stringstream ss(dataStr);
                    std::string segment;
                    int x = 0, y = 0;

                    while (std::getline(ss, segment, ',')) {
                        // Remove whitespace
                        segment.erase(std::remove_if(segment.begin(), segment.end(), ::isspace), segment.end());
                        if (segment.empty()) continue;

                        unsigned int gid = std::stoul(segment);
                        
                        // Handle flipping flags
                        const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                        const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;
                        const unsigned FLIPPED_DIAGONALLY_FLAG   = 0x20000000;
                        
                        bool flipX = (gid & FLIPPED_HORIZONTALLY_FLAG);
                        bool flipY = (gid & FLIPPED_VERTICALLY_FLAG);
                        bool flipDiag = (gid & FLIPPED_DIAGONALLY_FLAG);

                        // Store flip flags in Tile struct
                        gid &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);

                        if (gid > 0) {
                            Tile& tile = layer.GetTile(x, y, tilemap->GetWidth());
                            tile = Tile((int)gid, false);
                            tile.flipX = flipX;
                            tile.flipY = flipY;
                            tile.flipDiagonal = flipDiag;
                        }

                        x++;
                        if (x >= tilemap->GetWidth()) {
                            x = 0;
                            y++;
                        }
                    }
                }
            } else {
                SAGE_ERROR("TMXLoader: Unsupported encoding '{}' (only CSV supported for now)", encoding ? encoding : "xml");
            }
        }

        layerNode = layerNode->NextSiblingElement("layer");
    }

    return tilemap;
}

} // namespace SAGE
