#include "TilemapLoader.h"
#include "ECS/Components/TilemapComponent.h"
#include "Core/ResourceManager.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Core/Logger.h"
#include "Core/Compression/ZlibDecompressor.h"
#include "json/json.hpp"

#ifdef SAGE_HAS_TINYXML2
#include <tinyxml2.h>
#endif

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <cstring>
#include <string_view>

using json = nlohmann::json;

namespace SAGE {

namespace {
    constexpr uint32_t kFlipHorizontalFlag = 0x80000000u;
    constexpr uint32_t kFlipVerticalFlag   = 0x40000000u;
    constexpr uint32_t kFlipDiagonalFlag   = 0x20000000u;
    constexpr uint32_t kFlipHexRotationFlag = 0x10000000u;
    constexpr uint32_t kFlipMask = kFlipHorizontalFlag | kFlipVerticalFlag | kFlipDiagonalFlag | kFlipHexRotationFlag;

    bool TryParseFloat(const std::string& text, float& outValue) {
        if (text.empty()) {
            return false;
        }
        char* endPtr = nullptr;
        outValue = std::strtof(text.c_str(), &endPtr);
        return endPtr && endPtr != text.c_str();
    }

    bool DecodeBase64(const std::string& input, std::vector<uint8_t>& outBytes, std::string_view contextLabel) {
        static const auto kDecodeTable = []() {
            std::array<int8_t, 256> table{};
            table.fill(-1);
            const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            for (int i = 0; alphabet[i] != '\0'; ++i) {
                table[static_cast<uint8_t>(alphabet[i])] = static_cast<int8_t>(i);
            }
            return table;
        }();

        if (input.empty()) {
            outBytes.clear();
            return true;
        }

        std::string data = input;
        if ((data.size() % 4) != 0) {
            const size_t remainder = data.size() % 4;
            SAGE_WARN("TilemapLoader::LoadTMX - Base64 data for '{}' has length {} (not divisible by 4). Padding with '='.", contextLabel, data.size());
            data.append(4 - remainder, '=');
        }

        outBytes.clear();
        outBytes.reserve((data.size() / 4) * 3);

        for (size_t i = 0; i < data.size(); i += 4) {
            int values[4] = {0, 0, 0, 0};
            for (int j = 0; j < 4; ++j) {
                const unsigned char c = static_cast<unsigned char>(data[i + j]);
                if (c == '=') {
                    values[j] = 0;
                } else {
                    const int8_t decoded = kDecodeTable[c];
                    if (decoded < 0) {
                        SAGE_WARN("TilemapLoader::LoadTMX - Base64 data for '{}' contains invalid character (byte {})", contextLabel, static_cast<int>(c));
                        outBytes.clear();
                        return false;
                    }
                    values[j] = decoded;
                }
            }

            const uint32_t triple = (values[0] << 18) | (values[1] << 12) | (values[2] << 6) | values[3];
            outBytes.push_back(static_cast<uint8_t>((triple >> 16) & 0xFF));
            if (data[i + 2] != '=') {
                outBytes.push_back(static_cast<uint8_t>((triple >> 8) & 0xFF));
            }
            if (data[i + 3] != '=') {
                outBytes.push_back(static_cast<uint8_t>(triple & 0xFF));
            }
        }

        return true;
    }

    Color MultiplyColor(const Color& a, const Color& b) {
        return Color(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
    }

    Color ParseHexColor(const std::string& hexStr) {
        std::string str = hexStr;
        if (!str.empty() && str.front() == '#') {
            str.erase(str.begin());
        }

        if (str.size() != 6 && str.size() != 8) {
            return Color::White();
        }

        uint32_t parsed = 0;
        std::stringstream ss;
        ss << std::hex << str;
        ss >> parsed;
        if (!ss) {
            return Color::White();
        }

        uint8_t r = 0, g = 0, b = 0, a = 255;

        if (str.size() == 6) {
            r = static_cast<uint8_t>((parsed >> 16) & 0xFF);
            g = static_cast<uint8_t>((parsed >> 8) & 0xFF);
            b = static_cast<uint8_t>(parsed & 0xFF);
        } else {
            a = static_cast<uint8_t>((parsed >> 24) & 0xFF);
            r = static_cast<uint8_t>((parsed >> 16) & 0xFF);
            g = static_cast<uint8_t>((parsed >> 8) & 0xFF);
            b = static_cast<uint8_t>(parsed & 0xFF);
        }

        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    bool ParseTiledColorString(const char* value, Color& outColor) {
        if (!value || !*value) {
            return false;
        }

        std::string tintStr = value;
        if (!tintStr.empty() && tintStr.front() == '#') {
            tintStr.erase(tintStr.begin());
        }

        if (tintStr.size() != 6 && tintStr.size() != 8) {
            return false;
        }

        uint32_t parsed = 0;
        std::stringstream ss;
        ss << std::hex << tintStr;
        ss >> parsed;
        if (!ss) {
            return false;
        }

        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        if (tintStr.size() == 6) {
            r = static_cast<uint8_t>((parsed >> 16) & 0xFF);
            g = static_cast<uint8_t>((parsed >> 8) & 0xFF);
            b = static_cast<uint8_t>(parsed & 0xFF);
        } else {
            a = static_cast<uint8_t>((parsed >> 24) & 0xFF);
            r = static_cast<uint8_t>((parsed >> 16) & 0xFF);
            g = static_cast<uint8_t>((parsed >> 8) & 0xFF);
            b = static_cast<uint8_t>(parsed & 0xFF);
        }

        outColor = Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        return true;
    }

    struct LayerContext {
        bool visible = true;
        float opacity = 1.0f;
        Float2 offset{0.0f, 0.0f};
        Float2 parallax{1.0f, 1.0f};
        Color tint = Color::White();
    };

    void ApplyContextToTileLayer(const LayerContext& context, ECS::TilemapLayer& layer) {
        layer.visible = layer.visible && context.visible;
        layer.opacity *= context.opacity;
        layer.offset.x += context.offset.x;
        layer.offset.y += context.offset.y;
        layer.parallaxFactor.x *= context.parallax.x;
        layer.parallaxFactor.y *= context.parallax.y;
        layer.tint = MultiplyColor(context.tint, layer.tint);
    }

    void ApplyContextToObjectLayer(const LayerContext& context, ECS::TilemapObjectLayer& layer) {
        layer.visible = layer.visible && context.visible;
        layer.opacity *= context.opacity;
        layer.offset.x += context.offset.x;
        layer.offset.y += context.offset.y;
        layer.parallaxFactor.x *= context.parallax.x;
        layer.parallaxFactor.y *= context.parallax.y;
        layer.tint = MultiplyColor(context.tint, layer.tint);
    }

    void ApplyContextToImageLayer(const LayerContext& context, ECS::TilemapImageLayer& layer) {
        layer.visible = layer.visible && context.visible;
        layer.opacity *= context.opacity;
        layer.offset.x += context.offset.x;
        layer.offset.y += context.offset.y;
        layer.parallaxFactor.x *= context.parallax.x;
        layer.parallaxFactor.y *= context.parallax.y;
        layer.tint = MultiplyColor(context.tint, layer.tint);
    }

    // Parse typed custom properties from Tiled JSON
    void ParseCustomProperties(const json& propsJson, std::unordered_map<std::string, ECS::CustomProperty>& outProps) {
        if (!propsJson.is_array()) return;

        for (const auto& propJson : propsJson) {
            if (!propJson.contains("name") || !propJson.contains("type") || !propJson.contains("value")) {
                continue;
            }

            std::string propName = propJson["name"].get<std::string>();
            std::string propType = propJson["type"].get<std::string>();
            
            ECS::CustomProperty prop;

            if (propType == "string") {
                prop.type = ECS::PropertyType::String;
                prop.value = propJson["value"].get<std::string>();
            }
            else if (propType == "int") {
                prop.type = ECS::PropertyType::Int;
                prop.value = propJson["value"].get<int>();
            }
            else if (propType == "float") {
                prop.type = ECS::PropertyType::Float;
                prop.value = propJson["value"].get<float>();
            }
            else if (propType == "bool") {
                prop.type = ECS::PropertyType::Bool;
                prop.value = propJson["value"].get<bool>();
            }
            else if (propType == "color") {
                prop.type = ECS::PropertyType::Color;
                std::string colorStr = propJson["value"].get<std::string>();
                prop.value = ParseHexColor(colorStr);
            }
            else if (propType == "file") {
                prop.type = ECS::PropertyType::File;
                prop.value = propJson["value"].get<std::string>();
            }
            else if (propType == "object") {
                prop.type = ECS::PropertyType::Object;
                // Object references are stored as int IDs
                prop.value = propJson["value"].get<int>();
            }
            else {
                // Unknown type - store as string
                prop.type = ECS::PropertyType::String;
                prop.value = propJson["value"].dump();
            }

            outProps[propName] = prop;
        }
    }

    // Parse collision shapes from tile objectgroup
    void ParseTileCollisionShapes(const json& objGroupJson, std::vector<ECS::TileCollisionShape>& outShapes) {
        if (!objGroupJson.contains("objects") || !objGroupJson["objects"].is_array()) {
            return;
        }

        for (const auto& obj : objGroupJson["objects"]) {
            ECS::TileCollisionShape shape;
            shape.offset.x = obj.value("x", 0.0f);
            shape.offset.y = obj.value("y", 0.0f);
            shape.size.x = obj.value("width", 0.0f);
            shape.size.y = obj.value("height", 0.0f);

            // Determine shape type
            if (obj.contains("ellipse") && obj["ellipse"].get<bool>()) {
                shape.type = ECS::CollisionShapeType::Ellipse;
            }
            else if (obj.contains("polygon") && obj["polygon"].is_array()) {
                shape.type = ECS::CollisionShapeType::Polygon;
                for (const auto& point : obj["polygon"]) {
                    Float2 p;
                    p.x = point.value("x", 0.0f);
                    p.y = point.value("y", 0.0f);
                    shape.points.push_back(p);
                }
            }
            else if (obj.contains("polyline") && obj["polyline"].is_array()) {
                shape.type = ECS::CollisionShapeType::Polygon;
                for (const auto& point : obj["polyline"]) {
                    Float2 p;
                    p.x = point.value("x", 0.0f);
                    p.y = point.value("y", 0.0f);
                    shape.points.push_back(p);
                }
            }
            else {
                // Default rectangle
                shape.type = ECS::CollisionShapeType::Rectangle;
            }

            outShapes.push_back(shape);
        }
    }

    struct TMXContext {
        std::filesystem::path mapDirectory;  // absolute directory containing the TMX/JSON file or external tileset
        std::filesystem::path assetsRoot;    // absolute assets directory (if discoverable)
    };

    std::filesystem::path FindAssetsRoot(const std::filesystem::path& start) {
        std::filesystem::path current = start;
        while (!current.empty()) {
            if (current.filename() == "assets") {
                return current;
            }
            current = current.parent_path();
        }
        return {};
    }

    std::filesystem::path ResolveRelativePath(const std::filesystem::path& baseDir, const char* rawPath) {
        if (!rawPath || !*rawPath) {
            return {};
        }
        std::filesystem::path relative(rawPath);
        std::error_code ec;

        if (relative.is_absolute()) {
            return relative.lexically_normal();
        }

        if (baseDir.empty()) {
            auto absPath = std::filesystem::absolute(relative, ec);
            if (ec) {
                return relative.lexically_normal();
            }
            return absPath.lexically_normal();
        }

        auto combined = baseDir / relative;
        auto absPath = std::filesystem::absolute(combined, ec);
        if (ec) {
            return combined.lexically_normal();
        }
        return absPath.lexically_normal();
    }

    std::string MakeAssetsRelative(const std::filesystem::path& absolutePath,
                                   const TMXContext& context) {
        auto normalized = absolutePath.lexically_normal();
        if (!context.assetsRoot.empty()) {
            std::error_code ec;
            auto rel = std::filesystem::relative(normalized, context.assetsRoot, ec);
            if (!ec && rel.string().find("..") == std::string::npos) {
                auto relStr = rel.generic_string();
                if (!relStr.empty()) {
                    return relStr;
                }
            }
        }

        // Fallback: attempt relative to the map/tileset directory
        std::error_code ec{};
        auto relToMap = std::filesystem::relative(normalized, context.mapDirectory, ec);
        if (!ec && !relToMap.empty()) {
            auto relStr = relToMap.generic_string();
            if (relStr.find("..") == std::string::npos) {
                return relStr;
            }
        }

        // Final fallback: absolute string (ResourceManager will sanitize)
        return normalized.generic_string();
    }

    void RefreshLegacyAnimations(ECS::TilesetInfo& tileset) {
        tileset.animations.clear();
        for (const auto& def : tileset.tiles) {
            if (def.localID < 0 || !def.IsAnimated()) {
                continue;
            }
            ECS::TileAnimation anim;
            anim.localTileID = def.localID;
            for (const auto& frame : def.animation) {
                ECS::AnimationFrame legacyFrame;
                legacyFrame.localTileID = frame.tileID;
                legacyFrame.durationMs = frame.durationMs;
                anim.frames.push_back(legacyFrame);
            }
            tileset.animations.push_back(anim);
        }
    }

    void PopulateTilesetMetadataFromJson(const json& tilesetJson, ECS::TilesetInfo& tileset) {
        tileset.margin = tilesetJson.value("margin", tileset.margin);
        tileset.spacing = tilesetJson.value("spacing", tileset.spacing);
        
        // Parse tile offset
        if (tilesetJson.contains("tileoffset")) {
            const auto& offsetJson = tilesetJson["tileoffset"];
            tileset.tileOffset.x = offsetJson.value("x", 0.0f);
            tileset.tileOffset.y = offsetJson.value("y", 0.0f);
        }

        if (tilesetJson.contains("tiles") && tilesetJson["tiles"].is_array()) {
            for (const auto& tileData : tilesetJson["tiles"]) {
                int localId = tileData.value("id", -1);
                if (localId < 0) {
                    continue;
                }

                if (localId >= static_cast<int>(tileset.tiles.size())) {
                    tileset.tiles.resize(localId + 1);
                }

                auto& def = tileset.tiles[localId];
                def.localID = localId;

                if (tileData.contains("properties") && tileData["properties"].is_array()) {
                    ParseCustomProperties(tileData["properties"], def.properties);
                }

                def.collisionShapes.clear();
                if (tileData.contains("objectgroup") && tileData["objectgroup"].is_object()) {
                    ParseTileCollisionShapes(tileData["objectgroup"], def.collisionShapes);
                }

                def.animation.clear();
                if (tileData.contains("animation") && tileData["animation"].is_array()) {
                    for (const auto& frameJson : tileData["animation"]) {
                        ECS::AnimationFrame frame;
                        frame.localTileID = frameJson.value("tileid", -1);
                        frame.durationMs = frameJson.value("duration", 0);
                        if (frame.localTileID >= 0) {
                            def.animation.push_back(frame);
                        }
                    }

                    int totalDuration = 0;
                    for (const auto& frame : def.animation) {
                        totalDuration += std::max(0, frame.durationMs);
                    }
                    if (totalDuration <= 0) {
                        def.animation.clear();
                    }
                }
            }
        }

        RefreshLegacyAnimations(tileset);
    }

    void DeriveTilesetMetrics(ECS::TilesetInfo& tileset, int imageWidthHint, int imageHeightHint) {
        const bool textureReady = tileset.texture && tileset.texture->IsLoaded();
        const int textureWidth = textureReady ? static_cast<int>(tileset.texture->GetWidth()) : imageWidthHint;
        const int textureHeight = textureReady ? static_cast<int>(tileset.texture->GetHeight()) : imageHeightHint;

        auto computeColumns = [&](int pixelWidth) -> int {
            if (pixelWidth <= 0 || tileset.tileWidth <= 0) {
                return 0;
            }
            const int denominator = tileset.tileWidth + tileset.spacing;
            if (denominator <= 0) {
                return 0;
            }
            int available = pixelWidth - (2 * tileset.margin) + tileset.spacing;
            if (available < tileset.tileWidth) {
                return 1;
            }
            return std::max(1, available / denominator);
        };

        auto computeRows = [&](int pixelHeight) -> int {
            if (pixelHeight <= 0 || tileset.tileHeight <= 0) {
                return 0;
            }
            const int denominator = tileset.tileHeight + tileset.spacing;
            if (denominator <= 0) {
                return 0;
            }
            int available = pixelHeight - (2 * tileset.margin) + tileset.spacing;
            if (available < tileset.tileHeight) {
                return 1;
            }
            return std::max(1, available / denominator);
        };

        if (tileset.columns <= 0) {
            const int derivedColumns = computeColumns(textureWidth);
            if (derivedColumns > 0) {
                tileset.columns = derivedColumns;
            }
        }

        if (tileset.tileCount <= 0) {
            const int rows = computeRows(textureHeight);
            if (rows > 0 && tileset.columns > 0) {
                tileset.tileCount = tileset.columns * rows;
            }
        }
    }

    bool ExtractGzipDeflatePayload(const std::vector<uint8_t>& input,
                                   const uint8_t*& payload,
                                   std::size_t& payloadSize,
                                   std::string_view layerName) {
        if (input.size() < 18) {
            SAGE_ERROR("TilemapLoader::LoadTMX - Layer '{}' gzip payload too small ({} bytes)", layerName, input.size());
            return false;
        }

        const uint8_t* data = input.data();
        if (data[0] != 0x1F || data[1] != 0x8B || data[2] != 0x08) {
            SAGE_ERROR("TilemapLoader::LoadTMX - Layer '{}' has invalid gzip header", layerName);
            return false;
        }

        uint8_t flags = data[3];
        std::size_t offset = 10; // base header size

        auto RequireBytes = [&](std::size_t count) -> bool {
            if (offset + count > input.size()) {
                SAGE_ERROR("TilemapLoader::LoadTMX - Layer '{}' gzip header truncated", layerName);
                return false;
            }
            return true;
        };

        if (flags & 0x04) { // FEXTRA
            if (!RequireBytes(2)) return false;
            const uint16_t extraLen = static_cast<uint16_t>(data[offset]) | (static_cast<uint16_t>(data[offset + 1]) << 8);
            offset += 2;
            if (!RequireBytes(extraLen)) return false;
            offset += extraLen;
        }

        auto SkipZeroTerminated = [&](const char* fieldName) -> bool {
            while (offset < input.size() && data[offset] != 0) {
                ++offset;
            }
            if (offset >= input.size()) {
                SAGE_ERROR("TilemapLoader::LoadTMX - Layer '{}' gzip {} field unterminated", layerName, fieldName);
                return false;
            }
            ++offset; // skip null terminator
            return true;
        };

        if (flags & 0x08) { // FNAME
            if (!SkipZeroTerminated("filename")) return false;
        }
        if (flags & 0x10) { // FCOMMENT
            if (!SkipZeroTerminated("comment")) return false;
        }
        if (flags & 0x02) { // FHCRC
            if (!RequireBytes(2)) return false;
            offset += 2;
        }

        if (offset >= input.size()) {
            SAGE_ERROR("TilemapLoader::LoadTMX - Layer '{}' gzip stream missing deflate payload", layerName);
            return false;
        }
        if (input.size() < offset + 8) {
            SAGE_ERROR("TilemapLoader::LoadTMX - Layer '{}' gzip stream missing trailer", layerName);
            return false;
        }

        payload = data + offset;
        payloadSize = input.size() - offset - 8; // exclude CRC + ISIZE
        if (payloadSize == 0) {
            SAGE_WARN("TilemapLoader::LoadTMX - Layer '{}' gzip stream decompresses to zero bytes", layerName);
        }
        return true;
    }

    bool DecompressLayerData(const std::vector<uint8_t>& encoded,
                             const std::string& compression,
                             std::string_view layerName,
                             std::size_t expectedBytes,
                             std::vector<uint8_t>& outBytes) {
        if (compression.empty()) {
            outBytes = encoded;
            return true;
        }

        SAGE::Compression::ZlibDiagnostics diagnostics;

        if (compression == "zlib") {
            outBytes = SAGE::Compression::DecompressZlib(encoded.data(), encoded.size(), expectedBytes, &diagnostics);
            if (outBytes.empty()) {
                SAGE_ERROR("TilemapLoader::LoadTMX - Failed to zlib decompress layer '{}' data", layerName);
                return false;
            }
        } else if (compression == "gzip") {
            const uint8_t* payload = nullptr;
            std::size_t payloadSize = 0;
            if (!ExtractGzipDeflatePayload(encoded, payload, payloadSize, layerName)) {
                return false;
            }
            outBytes = SAGE::Compression::DecompressDeflate(payload, payloadSize, false, expectedBytes, &diagnostics);
            if (outBytes.empty()) {
                SAGE_ERROR("TilemapLoader::LoadTMX - Failed to gzip decompress layer '{}' data", layerName);
                return false;
            }
        } else {
            SAGE_WARN("TilemapLoader::LoadTMX - Compression '{}' not supported for layer '{}'", compression, layerName);
            return false;
        }

        if (diagnostics.syntheticBackrefs) {
            SAGE_WARN("TilemapLoader::LoadTMX - Layer '{}' deflate stream contained {} invalid back-reference(s); applied synthesized data", layerName, diagnostics.syntheticBackrefCount);
        }

        if (expectedBytes > 0 && outBytes.size() != expectedBytes) {
            SAGE_WARN("TilemapLoader::LoadTMX - Layer '{}' decompressed byte count {} differs from expected {}", layerName, outBytes.size(), expectedBytes);
        }

        return true;
    }

    void ParsePointString(const char* rawPoints,
                          std::string_view layerName,
                          std::string_view objectName,
                          std::vector<Float2>& outPoints) {
        if (!rawPoints) {
            return;
        }

        std::stringstream ss(rawPoints);
        std::string token;
        int invalidCount = 0;
        constexpr int kMaxWarnings = 5;
        while (std::getline(ss, token, ' ')) {
            if (token.empty()) {
                continue;
            }
            const auto comma = token.find(',');
            if (comma == std::string::npos) {
                if (invalidCount < kMaxWarnings) {
                    SAGE_WARN("TilemapLoader::LoadTMX - Object '{}' in layer '{}' has malformed point '{}' (missing comma)", objectName, layerName, token);
                }
                ++invalidCount;
                continue;
            }

            std::string xStr = token.substr(0, comma);
            std::string yStr = token.substr(comma + 1);
            float px = 0.0f;
            float py = 0.0f;
            if (!TryParseFloat(xStr, px) || !TryParseFloat(yStr, py)) {
                if (invalidCount < kMaxWarnings) {
                    SAGE_WARN("TilemapLoader::LoadTMX - Object '{}' in layer '{}' has non-numeric point '{}'", objectName, layerName, token);
                }
                ++invalidCount;
                continue;
            }
            outPoints.push_back({px, py});
        }

        if (invalidCount > kMaxWarnings) {
            SAGE_WARN("TilemapLoader::LoadTMX - Object '{}' in layer '{}' suppressed {} additional malformed points", objectName, layerName, invalidCount - kMaxWarnings);
        }
    }
}

#ifdef SAGE_HAS_TINYXML2
namespace {

    using tinyxml2::XMLElement;

    bool PopulateTilesetFromNode(XMLElement* tilesetElement,
                                 const TMXContext& context,
                                 int firstGID,
                                 int defaultTileWidth,
                                 int defaultTileHeight,
                                 ECS::TilesetInfo& outTileset) {
        if (!tilesetElement) {
            return false;
        }

        outTileset = {};
        outTileset.firstGID = firstGID;
        outTileset.name = tilesetElement->Attribute("name") ? tilesetElement->Attribute("name") : "";
        outTileset.tileWidth = tilesetElement->IntAttribute("tilewidth", defaultTileWidth);
        outTileset.tileHeight = tilesetElement->IntAttribute("tileheight", defaultTileHeight);
        outTileset.columns = tilesetElement->IntAttribute("columns", 0);
        outTileset.tileCount = tilesetElement->IntAttribute("tilecount", 0);
    outTileset.margin = tilesetElement->IntAttribute("margin", 0);
    outTileset.spacing = tilesetElement->IntAttribute("spacing", 0);
    
        // Parse tile offset
        if (auto* offsetNode = tilesetElement->FirstChildElement("tileoffset")) {
            outTileset.tileOffset.x = offsetNode->FloatAttribute("x", 0.0f);
            outTileset.tileOffset.y = offsetNode->FloatAttribute("y", 0.0f);
        }

        const auto* imageNode = tilesetElement->FirstChildElement("image");
        if (!imageNode || !imageNode->Attribute("source")) {
            SAGE_ERROR("TilemapLoader::LoadTMX - Tileset '{}' missing <image> source", outTileset.name);
            return false;
        }

        auto imagePath = ResolveRelativePath(context.mapDirectory, imageNode->Attribute("source"));
        if (imagePath.empty()) {
            SAGE_ERROR("TilemapLoader::LoadTMX - Tileset '{}' has invalid image path", outTileset.name);
            return false;
        }

        TMXContext imageContext{context};
        imageContext.mapDirectory = context.mapDirectory; // ensure relative fallback prefers TMX dir
        outTileset.texturePath = MakeAssetsRelative(imagePath, imageContext);
        outTileset.texture = ResourceManager::Get().Load<Texture>(outTileset.texturePath);

        const int imageWidthAttr = imageNode->IntAttribute("width", 0);
        const int imageHeightAttr = imageNode->IntAttribute("height", 0);
        const bool textureReady = outTileset.texture && outTileset.texture->IsLoaded();
        const int textureWidth = textureReady ? static_cast<int>(outTileset.texture->GetWidth()) : 0;
        const int textureHeight = textureReady ? static_cast<int>(outTileset.texture->GetHeight()) : 0;

        if (!textureReady) {
            SAGE_WARN("TilemapLoader::LoadTMX - Failed to load texture '{}' for tileset '{}'", outTileset.texturePath, outTileset.name);
        }

        auto computeColumns = [&](int pixelWidth) -> int {
            if (pixelWidth <= 0 || outTileset.tileWidth <= 0) {
                return 0;
            }
            const int denominator = outTileset.tileWidth + outTileset.spacing;
            if (denominator <= 0) {
                return 0;
            }
            int available = pixelWidth - (2 * outTileset.margin) + outTileset.spacing;
            if (available < outTileset.tileWidth) {
                return 1;
            }
            return std::max(1, available / denominator);
        };

        auto computeRows = [&](int pixelHeight) -> int {
            if (pixelHeight <= 0 || outTileset.tileHeight <= 0) {
                return 0;
            }
            const int denominator = outTileset.tileHeight + outTileset.spacing;
            if (denominator <= 0) {
                return 0;
            }
            int available = pixelHeight - (2 * outTileset.margin) + outTileset.spacing;
            if (available < outTileset.tileHeight) {
                return 1;
            }
            return std::max(1, available / denominator);
        };

        if (outTileset.columns <= 0) {
            const int sourceWidth = textureWidth > 0 ? textureWidth : imageWidthAttr;
            const int derivedColumns = computeColumns(sourceWidth);
            if (derivedColumns > 0) {
                outTileset.columns = derivedColumns;
            }
        }

        if (outTileset.tileCount <= 0 && outTileset.tileHeight > 0) {
            const int sourceHeight = textureHeight > 0 ? textureHeight : imageHeightAttr;
            const int rows = computeRows(sourceHeight);
            if (rows > 0 && outTileset.columns > 0) {
                outTileset.tileCount = outTileset.columns * rows;
            }
        }

        return true;
    }

    void PopulateTilesetTileMetadata(XMLElement* tilesetElement, ECS::TilesetInfo& tileset) {
        if (!tilesetElement) {
            return;
        }

        auto parseProperty = [](tinyxml2::XMLElement* propNode) -> ECS::CustomProperty {
            const char* typeAttr = propNode->Attribute("type");
            const char* valueAttr = propNode->Attribute("value");
            const char* text = propNode->GetText();
            auto fetchString = [&]() -> std::string {
                if (valueAttr && *valueAttr) return valueAttr;
                if (text && *text) return text;
                return std::string();
            };

            std::string type = typeAttr ? typeAttr : "";
            std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

            if (type == "int") {
                return ECS::CustomProperty(propNode->IntAttribute("value", valueAttr ? std::atoi(valueAttr) : 0));
            } else if (type == "float") {
                return ECS::CustomProperty(propNode->FloatAttribute("value", valueAttr ? static_cast<float>(std::atof(valueAttr)) : 0.0f));
            } else if (type == "bool") {
                bool boolVal = false;
                if (valueAttr) {
                    std::string valueLower = fetchString();
                    std::transform(valueLower.begin(), valueLower.end(), valueLower.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                    boolVal = (valueLower == "true" || valueLower == "1");
                }
                boolVal = propNode->BoolAttribute("value", boolVal);
                return ECS::CustomProperty(boolVal);
            } else if (type == "color") {
                std::string str = fetchString();
                return ECS::CustomProperty(ParseHexColor(str));
            } else if (type == "string" || type == "file" || type.empty()) {
                return ECS::CustomProperty(fetchString());
            }

            // Fallback to string for unsupported types
            return ECS::CustomProperty(fetchString());
        };

        for (tinyxml2::XMLElement* tileElem = tilesetElement->FirstChildElement("tile"); tileElem; tileElem = tileElem->NextSiblingElement("tile")) {
            int localID = tileElem->IntAttribute("id", -1);
            if (localID < 0) {
                continue;
            }

            if (localID >= static_cast<int>(tileset.tiles.size())) {
                tileset.tiles.resize(localID + 1);
            }

            auto& def = tileset.tiles[localID];
            def.localID = localID;

            if (auto* propsNode = tileElem->FirstChildElement("properties")) {
                for (auto* propNode = propsNode->FirstChildElement("property"); propNode; propNode = propNode->NextSiblingElement("property")) {
                    const char* nameAttr = propNode->Attribute("name");
                    if (!nameAttr || !*nameAttr) {
                        continue;
                    }
                    def.properties[nameAttr] = parseProperty(propNode);
                }
            }

            def.animation.clear();
            if (auto* animNode = tileElem->FirstChildElement("animation")) {
                for (auto* frameNode = animNode->FirstChildElement("frame"); frameNode; frameNode = frameNode->NextSiblingElement("frame")) {
                    int frameID = frameNode->IntAttribute("tileid", -1);
                    int duration = frameNode->IntAttribute("duration", 0);
                    if (frameID < 0) {
                        continue;
                    }
                    ECS::AnimationFrame frame{};
                    frame.localTileID = frameID;
                    frame.durationMs = duration;
                    def.animation.push_back(frame);
                }

                // Validate total duration
                int totalDuration = 0;
                for (const auto& frame : def.animation) {
                    totalDuration += std::max(0, frame.durationMs);
                }
                if (totalDuration <= 0) {
                    def.animation.clear();
                }
            }

            def.collisionShapes.clear();
            if (auto* objGroup = tileElem->FirstChildElement("objectgroup")) {
                for (auto* obj = objGroup->FirstChildElement("object"); obj; obj = obj->NextSiblingElement("object")) {
                    ECS::TileCollisionShape shape;
                    shape.offset.x = obj->FloatAttribute("x", 0.0f);
                    shape.offset.y = obj->FloatAttribute("y", 0.0f);
                    shape.size.x = obj->FloatAttribute("width", 0.0f);
                    shape.size.y = obj->FloatAttribute("height", 0.0f);

                    if (obj->FirstChildElement("ellipse")) {
                        shape.type = ECS::CollisionShapeType::Ellipse;
                    } else if (auto* poly = obj->FirstChildElement("polygon")) {
                        shape.type = ECS::CollisionShapeType::Polygon;
                        const char* points = poly->Attribute("points");
                        if (points) {
                            ParsePointString(points, tileset.name, obj->Attribute("name") ? obj->Attribute("name") : "", shape.points);
                        }
                    } else if (auto* polyline = obj->FirstChildElement("polyline")) {
                        shape.type = ECS::CollisionShapeType::Polygon;
                        const char* points = polyline->Attribute("points");
                        if (points) {
                            ParsePointString(points, tileset.name, obj->Attribute("name") ? obj->Attribute("name") : "", shape.points);
                        }
                    } else {
                        shape.type = ECS::CollisionShapeType::Rectangle;
                    }

                    def.collisionShapes.push_back(shape);
                }
            }
        }
        RefreshLegacyAnimations(tileset);
    }

} // namespace
#endif // SAGE_HAS_TINYXML2

    bool TilemapLoader::Load(const std::string& filepath, ECS::TilemapComponent& outMap) {
        // Determine format by extension
        std::filesystem::path path(filepath);
        std::string ext = path.extension().string();
        
        // Convert to lowercase for case-insensitive comparison
        std::transform(ext.begin(), ext.end(), ext.begin(), 
                      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        
        if (ext == ".tmx") {
            return LoadTMX(filepath, outMap);
        }
        else if (ext == ".json" || ext == ".tmj") {
            return LoadJSON(filepath, outMap);
        }
        else if (ext == ".csv") {
            return LoadCSV(filepath, outMap);
        }
        else {
            SAGE_ERROR("TilemapLoader::Load - Unsupported file extension '{}' for file '{}'", ext, filepath);
            return false;
        }
    }

    bool TilemapLoader::LoadCSV(const std::string& filepath, ECS::TilemapComponent& outMap) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            SAGE_ERROR("TilemapLoader::LoadCSV - Failed to open file: {}", filepath);
            return false;
        }

        outMap.layers.clear();
        outMap.tilesets.clear();
        outMap.objectLayers.clear();

        std::vector<std::vector<int>> grid;
        std::string line;
        int lineNumber = 0;
        int invalidCells = 0;
        constexpr int kMaxCsvWarnings = 5;
        while (std::getline(file, line)) {
            ++lineNumber;
            if (line.empty()) continue;
            std::vector<int> row;
            std::stringstream ss(line);
            std::string cell;
            int columnIndex = 0;
            while (std::getline(ss, cell, ',')) {
                ++columnIndex;
                std::string trimmed = cell;
                trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), [](unsigned char ch) { return !std::isspace(ch); }));
                trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), trimmed.end());

                if (trimmed.empty()) {
                    row.push_back(0);
                    continue;
                }

                try {
                    row.push_back(std::stoi(trimmed));
                } catch (...) {
                    row.push_back(0);
                    ++invalidCells;
                    if (invalidCells <= kMaxCsvWarnings) {
                        SAGE_WARN("TilemapLoader::LoadCSV - Invalid value '{}' at line {}, column {} (treated as empty tile)", trimmed, lineNumber, columnIndex);
                    }
                }
            }
            if (!row.empty()) grid.push_back(row);
        }

        if (invalidCells > kMaxCsvWarnings) {
            SAGE_WARN("TilemapLoader::LoadCSV - Suppressed {} additional invalid CSV values", invalidCells - kMaxCsvWarnings);
        }

        if (grid.empty()) {
            SAGE_ERROR("TilemapLoader::LoadCSV - Empty grid");
            return false;
        }

        int height = (int)grid.size();
        int width = (int)grid[0].size();

        outMap.mapWidth = width;
        outMap.mapHeight = height;
        outMap.tileWidth = 16; // default
        outMap.tileHeight = 16;

        ECS::TilemapLayer layer;
        layer.name = "default";
        layer.width = width;
        layer.height = height;
        layer.visible = true;
        layer.opacity = 1.0f;
        layer.collision = false;

        for (const auto& row : grid) {
            for (int val : row) {
                layer.tiles.push_back(val);
            }
        }

        outMap.layers.push_back(layer);
        return true;
    }

    bool TilemapLoader::LoadJSON(const std::string& filepath, ECS::TilemapComponent& outMap) {
        SAGE_INFO("TilemapLoader::LoadJSON - Attempting to load: {}", filepath);
        
        // Get absolute path
        std::filesystem::path absPath = std::filesystem::absolute(filepath);
        SAGE_INFO("TilemapLoader::LoadJSON - Absolute path: {}", absPath.string());
        SAGE_INFO("TilemapLoader::LoadJSON - File exists: {}", std::filesystem::exists(absPath));
        if (std::filesystem::exists(absPath)) {
            SAGE_INFO("TilemapLoader::LoadJSON - File size: {} bytes", std::filesystem::file_size(absPath));
        }
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            SAGE_ERROR("TilemapLoader::LoadJSON - Failed to open file: {}", filepath);
            return false;
        }

        // Debug: read first line
        std::string firstLine;
        std::getline(file, firstLine);
        SAGE_INFO("TilemapLoader::LoadJSON - First line of file: {}", firstLine);
        
        // Rewind to beginning
        file.clear();
        file.seekg(0, std::ios::beg);

        outMap.layers.clear();
        outMap.tilesets.clear();
        outMap.objectLayers.clear();

        json j;
        try {
            file >> j;
            SAGE_INFO("TilemapLoader::LoadJSON - Stream state: good={}, eof={}, fail={}, bad={}", 
                      file.good(), file.eof(), file.fail(), file.bad());
        } catch (const std::exception& e) {
            SAGE_ERROR("TilemapLoader::LoadJSON - JSON parse error: {}", e.what());
            return false;
        }

        SAGE_INFO("TilemapLoader::LoadJSON - JSON parsed successfully");
        
        // Debug: check JSON structure
        std::string jsonStr = j.dump();
        SAGE_INFO("TilemapLoader::LoadJSON - JSON size: {} bytes", jsonStr.size());
        SAGE_INFO("TilemapLoader::LoadJSON - JSON is object: {}, size: {}", j.is_object(), j.size());
        
        // Debug: check for width field
        if (j.contains("width")) {
            SAGE_INFO("TilemapLoader::LoadJSON - 'width' field found: {}", j["width"].get<int>());
        } else {
            SAGE_WARN("TilemapLoader::LoadJSON - 'width' field NOT found in JSON");
        }
        
        // Parse map properties
        SAGE_INFO("TilemapLoader::LoadJSON - Parsing map properties...");
        outMap.mapWidth = j.value("width", 0);
        outMap.mapHeight = j.value("height", 0);
        outMap.tileWidth = j.value("tilewidth", 16);
        outMap.tileHeight = j.value("tileheight", 16);
        SAGE_INFO("TilemapLoader::LoadJSON - Map size: {}x{}, tile size: {}x{}", 
                  outMap.mapWidth, outMap.mapHeight, outMap.tileWidth, outMap.tileHeight);
        
        // Parse orientation
        std::string orientation = j.value("orientation", "orthogonal");
        if (orientation == "orthogonal") {
            outMap.orientation = ECS::TilemapOrientation::Orthogonal;
        } else if (orientation == "isometric") {
            outMap.orientation = ECS::TilemapOrientation::Isometric;
        } else if (orientation == "staggered") {
            outMap.orientation = ECS::TilemapOrientation::Staggered;
        } else if (orientation == "hexagonal") {
            outMap.orientation = ECS::TilemapOrientation::Hexagonal;
        } else {
            outMap.orientation = ECS::TilemapOrientation::Orthogonal;
            SAGE_WARN("TilemapLoader::LoadJSON - Unknown orientation '{}', defaulting to orthogonal", orientation);
        }
        
        // Parse stagger axis (for staggered/hexagonal)
        if (j.contains("staggeraxis")) {
            std::string staggerAxis = j.value("staggeraxis", "");
            if (staggerAxis == "x") {
                outMap.staggerAxis = ECS::TilemapStaggerAxis::X;
            } else if (staggerAxis == "y") {
                outMap.staggerAxis = ECS::TilemapStaggerAxis::Y;
            }
        }
        
        // Parse stagger index (for staggered/hexagonal)
        if (j.contains("staggerindex")) {
            std::string staggerIndex = j.value("staggerindex", "");
            if (staggerIndex == "even") {
                outMap.staggerIndex = ECS::TilemapStaggerIndex::Even;
            } else if (staggerIndex == "odd") {
                outMap.staggerIndex = ECS::TilemapStaggerIndex::Odd;
            }
        }
        
        // Parse hex side length (for hexagonal)
        if (j.contains("hexsidelength")) {
            outMap.hexSideLength = j.value("hexsidelength", 0);
        }
        
        // Parse render order
        std::string renderOrder = j.value("renderorder", "right-down");
        if (renderOrder == "right-down") {
            outMap.renderOrder = ECS::TilemapRenderOrder::RightDown;
        } else if (renderOrder == "right-up") {
            outMap.renderOrder = ECS::TilemapRenderOrder::RightUp;
        } else if (renderOrder == "left-down") {
            outMap.renderOrder = ECS::TilemapRenderOrder::LeftDown;
        } else if (renderOrder == "left-up") {
            outMap.renderOrder = ECS::TilemapRenderOrder::LeftUp;
        }
        
        // Check for infinite maps
        bool infinite = j.value("infinite", false);
        outMap.infinite = infinite;
        if (infinite) {
            SAGE_INFO("TilemapLoader::LoadJSON - Loading infinite map with chunks: {}", filepath);
        }

        SAGE_INFO("TilemapLoader::LoadJSON - Checking for tilesets array...");
        // Parse tilesets
        if (j.contains("tilesets") && j["tilesets"].is_array()) {
            SAGE_INFO("TilemapLoader::LoadJSON - Parsing {} tilesets", j["tilesets"].size());
            TMXContext baseContext{};
            baseContext.mapDirectory = std::filesystem::path(filepath).parent_path().lexically_normal();
            baseContext.assetsRoot = FindAssetsRoot(baseContext.mapDirectory);

            auto resolveTexturePath = [&](const TMXContext& ctx, const std::string& image) -> std::string {
                if (image.empty()) {
                    return {};
                }
                auto absPath = ResolveRelativePath(ctx.mapDirectory, image.c_str());
                if (absPath.empty()) {
                    return {};
                }
                return MakeAssetsRelative(absPath, ctx);
            };

            for (const auto& ts : j["tilesets"]) {
                SAGE_INFO("TilemapLoader::LoadJSON - Processing tileset...");
                ECS::TilesetInfo tileset;
                tileset.firstGID = ts.value("firstgid", 1);
                tileset.tileWidth = ts.value("tilewidth", outMap.tileWidth);
                tileset.tileHeight = ts.value("tileheight", outMap.tileHeight);
                tileset.columns = ts.value("columns", 0);
                tileset.tileCount = ts.value("tilecount", 0);
                tileset.margin = ts.value("margin", 0);
                tileset.spacing = ts.value("spacing", 0);
                
                // Parse tile offset
                if (ts.contains("tileoffset")) {
                    const auto& offsetJson = ts["tileoffset"];
                    tileset.tileOffset.x = offsetJson.value("x", 0.0f);
                    tileset.tileOffset.y = offsetJson.value("y", 0.0f);
                }

                int imageWidthHint = ts.value("imagewidth", 0);
                int imageHeightHint = ts.value("imageheight", 0);

                TMXContext tilesetContext = baseContext;
                bool loadedMetadata = false;

                if (ts.contains("source") && ts["source"].is_string()) {
                    std::string tilesetSource = ts["source"].get<std::string>();
                    auto sourcePath = ResolveRelativePath(baseContext.mapDirectory, tilesetSource.c_str());
                    if (sourcePath.empty()) {
                        SAGE_WARN("TilemapLoader::LoadJSON - Tileset '{}' had invalid external source '{}'", tileset.firstGID, tilesetSource);
                        continue;
                    }

                    tilesetContext.mapDirectory = sourcePath.parent_path().lexically_normal();
                    tilesetContext.assetsRoot = FindAssetsRoot(tilesetContext.mapDirectory);

                    std::string extension = sourcePath.extension().string();
                    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

#ifdef SAGE_HAS_TINYXML2
                    if (extension == ".tsx") {
                        tinyxml2::XMLDocument tsxDoc;
                        if (tsxDoc.LoadFile(sourcePath.string().c_str()) != tinyxml2::XML_SUCCESS) {
                            SAGE_WARN("TilemapLoader::LoadJSON - Failed to load TSX '{}': {}", sourcePath.string(), tsxDoc.ErrorStr());
                            continue;
                        }

                        auto* tsxRoot = tsxDoc.FirstChildElement("tileset");
                        if (!tsxRoot) {
                            SAGE_WARN("TilemapLoader::LoadJSON - TSX '{}' missing <tileset> root", sourcePath.string());
                            continue;
                        }

                        if (!PopulateTilesetFromNode(tsxRoot, tilesetContext, tileset.firstGID, tileset.tileWidth, tileset.tileHeight, tileset)) {
                            continue;
                        }
                        PopulateTilesetTileMetadata(tsxRoot, tileset);
                        loadedMetadata = true;
                    }
#endif

                    if (!loadedMetadata) {
                        if (!extension.empty() && extension != ".json" && extension != ".tmj" && extension != ".tileset") {
                            SAGE_WARN("TilemapLoader::LoadJSON - Unsupported external tileset format '{}' for '{}'", extension, sourcePath.string());
                            continue;
                        }
                        // Assume JSON-formatted external tileset
                        std::ifstream tilesetFile(sourcePath);
                        if (!tilesetFile.is_open()) {
                            SAGE_WARN("TilemapLoader::LoadJSON - Failed to open external tileset: {}", sourcePath.string());
                            continue;
                        }

                        json tilesetJson;
                        try {
                            tilesetFile >> tilesetJson;
                        } catch (const std::exception& e) {
                            SAGE_WARN("TilemapLoader::LoadJSON - Failed to parse tileset JSON '{}': {}", sourcePath.string(), e.what());
                            continue;
                        }

                        tileset.name = tilesetJson.value("name", tileset.name);
                        tileset.tileWidth = tilesetJson.value("tilewidth", tileset.tileWidth);
                        tileset.tileHeight = tilesetJson.value("tileheight", tileset.tileHeight);
                        tileset.columns = tilesetJson.value("columns", tileset.columns);
                        tileset.tileCount = tilesetJson.value("tilecount", tileset.tileCount);
                        tileset.margin = tilesetJson.value("margin", tileset.margin);
                        tileset.spacing = tilesetJson.value("spacing", tileset.spacing);
                        imageWidthHint = tilesetJson.value("imagewidth", imageWidthHint);
                        imageHeightHint = tilesetJson.value("imageheight", imageHeightHint);

                        const std::string imagePath = tilesetJson.value("image", std::string());
                        tileset.texturePath = resolveTexturePath(tilesetContext, imagePath);

                        PopulateTilesetMetadataFromJson(tilesetJson, tileset);
                        loadedMetadata = true;
                    }
                } else {
                    tileset.name = ts.value("name", "");
                    const std::string imagePath = ts.value("image", std::string());
                    tileset.texturePath = resolveTexturePath(tilesetContext, imagePath);

                    PopulateTilesetMetadataFromJson(ts, tileset);
                    loadedMetadata = true;
                }

                if (!loadedMetadata) {
                    SAGE_WARN("TilemapLoader::LoadJSON - Skipping tileset with firstGID {} due to missing metadata", tileset.firstGID);
                    continue;
                }

                if (!tileset.texturePath.empty()) {
                    if (!tileset.texture) {
                        SAGE_INFO("TilemapLoader::LoadJSON - Loading tileset texture: {}", tileset.texturePath);
                        tileset.texture = ResourceManager::Get().Load<Texture>(tileset.texturePath);
                        if (!tileset.texture) {
                            SAGE_ERROR("TilemapLoader::LoadJSON - Failed to load tileset texture: {}", tileset.texturePath);
                            return false;
                        }
                    }
                } else {
                    SAGE_WARN("TilemapLoader::LoadJSON - Tileset '{}' has no image path", tileset.name);
                }

                DeriveTilesetMetrics(tileset, imageWidthHint, imageHeightHint);
                outMap.tilesets.push_back(tileset);
            }
        }

        auto findTilesetForGid = [&](int gid) -> const ECS::TilesetInfo* {
            for (int i = static_cast<int>(outMap.tilesets.size()) - 1; i >= 0; --i) {
                if (gid >= outMap.tilesets[i].firstGID) {
                    return &outMap.tilesets[i];
                }
            }
            return nullptr;
        };

        std::function<void(const json&, const LayerContext&)> parseLayerJson;
        parseLayerJson = [&](const json& layerJson, const LayerContext& parentContext) {
            std::string type = layerJson.value("type", "");
            std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (type == "group") {
                LayerContext groupContext = parentContext;
                const bool visible = layerJson.value("visible", true);
                const float opacity = layerJson.value("opacity", 1.0f);
                groupContext.visible = parentContext.visible && visible;
                groupContext.opacity = parentContext.opacity * opacity;
                groupContext.offset.x += layerJson.value("offsetx", 0.0f);
                groupContext.offset.y += layerJson.value("offsety", 0.0f);
                groupContext.parallax.x *= layerJson.value("parallaxx", 1.0f);
                groupContext.parallax.y *= layerJson.value("parallaxy", 1.0f);

                if (layerJson.contains("tintcolor") && layerJson["tintcolor"].is_string()) {
                    const std::string tintStr = layerJson["tintcolor"].get<std::string>();
                    Color tintColor;
                    if (ParseTiledColorString(tintStr.c_str(), tintColor)) {
                        groupContext.tint = MultiplyColor(parentContext.tint, tintColor);
                    }
                }

                if (layerJson.contains("layers") && layerJson["layers"].is_array()) {
                    for (const auto& child : layerJson["layers"]) {
                        parseLayerJson(child, groupContext);
                    }
                }
                return;
            }

            if (type == "tilelayer") {
                ECS::TilemapLayer layer;
                layer.name = layerJson.value("name", "");
                layer.width = layerJson.value("width", outMap.mapWidth);
                layer.height = layerJson.value("height", outMap.mapHeight);
                layer.visible = layerJson.value("visible", true);
                layer.opacity = layerJson.value("opacity", 1.0f);
                layer.offset.x = layerJson.value("offsetx", 0.0f);
                layer.offset.y = layerJson.value("offsety", 0.0f);
                layer.parallaxFactor.x = layerJson.value("parallaxx", 1.0f);
                layer.parallaxFactor.y = layerJson.value("parallaxy", 1.0f);

                if (layerJson.contains("tintcolor") && layerJson["tintcolor"].is_string()) {
                    const std::string tintStr = layerJson["tintcolor"].get<std::string>();
                    Color tintColor;
                    if (ParseTiledColorString(tintStr.c_str(), tintColor)) {
                        layer.tint = tintColor;
                    }
                }

                if (layerJson.contains("properties") && layerJson["properties"].is_array()) {
                    ParseCustomProperties(layerJson["properties"], layer.properties);
                    
                    // Check for special collision property
                    if (layer.properties.count("collision")) {
                        layer.collision = layer.properties["collision"].AsBool();
                    }
                }

                // Parse tile data - support multiple encodings
                if (layerJson.contains("chunks") && layerJson["chunks"].is_array()) {
                    // Infinite map with chunks
                    for (const auto& chunkJson : layerJson["chunks"]) {
                        ECS::TilemapChunk chunk;
                        chunk.x = chunkJson.value("x", 0);
                        chunk.y = chunkJson.value("y", 0);
                        chunk.width = chunkJson.value("width", 16);
                        chunk.height = chunkJson.value("height", 16);
                        
                        // Parse chunk data
                        if (chunkJson.contains("data")) {
                            if (chunkJson["data"].is_array()) {
                                // Plain array
                                for (const auto& tile : chunkJson["data"]) {
                                    chunk.tiles.push_back(tile.get<int>());
                                }
                            } else if (chunkJson["data"].is_string()) {
                                // Base64 encoded
                                std::string base64Data = chunkJson["data"].get<std::string>();
                                std::vector<uint8_t> decodedBytes;
                                if (DecodeBase64(base64Data, decodedBytes, layer.name)) {
                                    // Convert bytes to tile IDs (little-endian 32-bit)
                                    for (size_t i = 0; i + 3 < decodedBytes.size(); i += 4) {
                                        uint32_t tileId = decodedBytes[i] | 
                                                         (decodedBytes[i+1] << 8) | 
                                                         (decodedBytes[i+2] << 16) | 
                                                         (decodedBytes[i+3] << 24);
                                        chunk.tiles.push_back(static_cast<int>(tileId));
                                    }
                                }
                            }
                        }
                        
                        // Store chunk with key (chunkY << 32) | chunkX
                        int64_t key = (static_cast<int64_t>(chunk.y) << 32) | static_cast<uint32_t>(chunk.x);
                        layer.chunks[key] = chunk;
                    }
                } else if (layerJson.contains("data")) {
                    std::string encoding = layerJson.value("encoding", "");
                    std::string compression = layerJson.value("compression", "");
                    
                    if (encoding.empty() && layerJson["data"].is_array()) {
                        // Plain array format
                        for (const auto& tile : layerJson["data"]) {
                            layer.tiles.push_back(tile.get<int>());
                        }
                    }
                    else if (encoding == "csv" && layerJson["data"].is_string()) {
                        // CSV string format
                        std::string csvData = layerJson["data"].get<std::string>();
                        std::istringstream stream(csvData);
                        std::string token;
                        while (std::getline(stream, token, ',')) {
                            // Trim whitespace
                            token.erase(0, token.find_first_not_of(" \t\n\r"));
                            token.erase(token.find_last_not_of(" \t\n\r") + 1);
                            if (!token.empty()) {
                                try {
                                    layer.tiles.push_back(std::stoi(token));
                                } catch (const std::exception& e) {
                                    layer.tiles.push_back(0);
                                    SAGE_WARNING("TilemapLoader::LoadJSON - Invalid CSV value '{}' in layer '{}': {}", token, layer.name, e.what());
                                }
                            }
                        }
                    }
                    else if (encoding == "base64" && layerJson["data"].is_string()) {
                        // Base64 encoding (with optional compression)
                        std::string base64Data = layerJson["data"].get<std::string>();
                        std::vector<uint8_t> decodedBytes;
                        
                        if (!DecodeBase64(base64Data, decodedBytes, layer.name)) {
                            SAGE_ERROR("TilemapLoader::LoadJSON - Failed to decode base64 data for layer '{}'", layer.name);
                            return;
                        }
                        
                        // Decompress if needed
                        if (!compression.empty()) {
                            const size_t expectedBytes = static_cast<size_t>(layer.width) * layer.height * 4;
                            std::vector<uint8_t> decompressed;
                            if (!DecompressLayerData(decodedBytes, compression, layer.name, expectedBytes, decompressed)) {
                                SAGE_ERROR("TilemapLoader::LoadJSON - Failed to decompress layer '{}' data", layer.name);
                                return;
                            }
                            decodedBytes = std::move(decompressed);
                        }
                        
                        // Convert bytes to tile IDs (little-endian 32-bit integers)
                        if (decodedBytes.size() % 4 != 0) {
                            SAGE_WARN("TilemapLoader::LoadJSON - Layer '{}' data size {} not divisible by 4", layer.name, decodedBytes.size());
                        }
                        
                        for (size_t i = 0; i + 3 < decodedBytes.size(); i += 4) {
                            uint32_t gid = decodedBytes[i]
                                         | (decodedBytes[i + 1] << 8)
                                         | (decodedBytes[i + 2] << 16)
                                         | (decodedBytes[i + 3] << 24);
                            layer.tiles.push_back(static_cast<int>(gid));
                        }
                    }
                    else if (!encoding.empty()) {
                        SAGE_WARN("TilemapLoader::LoadJSON - Unsupported encoding '{}' for layer '{}'", encoding, layer.name);
                    }
                }

                ApplyContextToTileLayer(parentContext, layer);

                outMap.layers.push_back(layer);
                return;
            }

            if (type == "objectgroup") {
                ECS::TilemapObjectLayer objectLayer;
                objectLayer.name = layerJson.value("name", "");
                objectLayer.visible = layerJson.value("visible", true);
                objectLayer.opacity = layerJson.value("opacity", 1.0f);
                objectLayer.offset.x = layerJson.value("offsetx", 0.0f);
                objectLayer.offset.y = layerJson.value("offsety", 0.0f);
                objectLayer.parallaxFactor.x = layerJson.value("parallaxx", 1.0f);
                objectLayer.parallaxFactor.y = layerJson.value("parallaxy", 1.0f);

                if (layerJson.contains("tintcolor") && layerJson["tintcolor"].is_string()) {
                    const std::string tintStr = layerJson["tintcolor"].get<std::string>();
                    Color tintColor;
                    if (ParseTiledColorString(tintStr.c_str(), tintColor)) {
                        objectLayer.tint = tintColor;
                    }
                }

                if (layerJson.contains("properties") && layerJson["properties"].is_array()) {
                    ParseCustomProperties(layerJson["properties"], objectLayer.properties);
                    
                    // Check for special collision property
                    if (objectLayer.properties.count("collision")) {
                        objectLayer.collision = objectLayer.properties["collision"].AsBool();
                    }
                }

                if (layerJson.contains("objects") && layerJson["objects"].is_array()) {
                    for (const auto& obj : layerJson["objects"]) {
                        const uint32_t rawGid = obj.value("gid", 0u);
                        const uint32_t normalizedGid = rawGid & ~kFlipMask;
                        if (normalizedGid == 0u) {
                            continue;
                        }

                        ECS::TilemapSprite sprite;
                        sprite.name = obj.value("name", "");
                        sprite.visible = obj.value("visible", true);
                        sprite.rotation = obj.value("rotation", 0.0f);
                        sprite.gid = rawGid;
                        sprite.position.x = obj.value("x", 0.0f);
                        sprite.position.y = obj.value("y", 0.0f);

                        float width = obj.value("width", 0.0f);
                        float height = obj.value("height", 0.0f);
                        if (width <= 0.0f || height <= 0.0f) {
                            if (const auto* ts = findTilesetForGid(static_cast<int>(normalizedGid))) {
                                width = static_cast<float>(ts->tileWidth);
                                height = static_cast<float>(ts->tileHeight);
                            } else {
                                width = static_cast<float>(outMap.tileWidth);
                                height = static_cast<float>(outMap.tileHeight);
                            }
                        }
                        sprite.size = { width, height };

                        float spriteOpacity = obj.value("opacity", 1.0f);
                        spriteOpacity = std::clamp(spriteOpacity, 0.0f, 1.0f);
                        sprite.tint.a *= spriteOpacity;

                        if (obj.contains("color") && obj["color"].is_string()) {
                            const std::string colorStr = obj["color"].get<std::string>();
                            Color objectColor;
                            if (ParseTiledColorString(colorStr.c_str(), objectColor)) {
                                sprite.tint = MultiplyColor(objectColor, sprite.tint);
                            }
                        }

                        // Parse custom properties
                        if (obj.contains("properties") && obj["properties"].is_array()) {
                            ParseCustomProperties(obj["properties"], sprite.properties);
                        }

                        // Parse text object properties
                        if (obj.contains("text") && obj["text"].is_object()) {
                            const auto& textObj = obj["text"];
                            sprite.shape = ECS::TilemapObjectShape::Text;
                            sprite.text = textObj.value("text", "");
                            sprite.fontFamily = textObj.value("fontfamily", "sans-serif");
                            sprite.pixelSize = textObj.value("pixelsize", 16);
                            sprite.wrap = textObj.value("wrap", false);
                            sprite.bold = textObj.value("bold", false);
                            sprite.italic = textObj.value("italic", false);
                            sprite.underline = textObj.value("underline", false);
                            sprite.strikeout = textObj.value("strikeout", false);
                            sprite.halign = textObj.value("halign", "left");
                            sprite.valign = textObj.value("valign", "top");
                            
                            // Parse text color
                            if (textObj.contains("color") && textObj["color"].is_string()) {
                                std::string colorStr = textObj["color"].get<std::string>();
                                sprite.textColor = ParseHexColor(colorStr);
                            }
                        }

                        objectLayer.sprites.push_back(sprite);
                    }
                }

                ApplyContextToObjectLayer(parentContext, objectLayer);

                if (!objectLayer.sprites.empty() || objectLayer.collision) {
                    outMap.objectLayers.push_back(objectLayer);
                }
                return;
            }

            if (type == "imagelayer") {
                ECS::TilemapImageLayer imageLayer;
                imageLayer.name = layerJson.value("name", "");
                imageLayer.visible = layerJson.value("visible", true);
                imageLayer.opacity = layerJson.value("opacity", 1.0f);
                imageLayer.offset.x = layerJson.value("offsetx", 0.0f);
                imageLayer.offset.y = layerJson.value("offsety", 0.0f);
                imageLayer.parallaxFactor.x = layerJson.value("parallaxx", 1.0f);
                imageLayer.parallaxFactor.y = layerJson.value("parallaxy", 1.0f);
                imageLayer.repeatX = layerJson.value("repeatx", false);
                imageLayer.repeatY = layerJson.value("repeaty", false);

                // Parse tint color
                if (layerJson.contains("tintcolor") && layerJson["tintcolor"].is_string()) {
                    std::string tintStr = layerJson["tintcolor"].get<std::string>();
                    imageLayer.tint = ParseHexColor(tintStr);
                }

                // Parse image path
                if (layerJson.contains("image") && layerJson["image"].is_string()) {
                    imageLayer.imagePath = layerJson["image"].get<std::string>();
                    
                    // Resolve relative path
                    std::filesystem::path mapDir = std::filesystem::path(filepath).parent_path();
                    std::filesystem::path imagePath = mapDir / imageLayer.imagePath;
                    
                    // Load texture
                    imageLayer.texture = ResourceManager::Get().Load<Texture>(imagePath.string());
                    if (!imageLayer.texture) {
                        SAGE_WARN("TilemapLoader::LoadJSON - Failed to load image layer texture: {}", imagePath.string());
                    }
                }

                // Apply parent context
                ApplyContextToImageLayer(parentContext, imageLayer);

                outMap.imageLayers.push_back(imageLayer);
                return;
            }
        };

        if (j.contains("layers") && j["layers"].is_array()) {
            LayerContext rootContext;
            for (const auto& lyr : j["layers"]) {
                parseLayerJson(lyr, rootContext);
            }
        }

        // Parse map-level custom properties
        if (j.contains("properties") && j["properties"].is_array()) {
            ParseCustomProperties(j["properties"], outMap.properties);
        }

        return outMap.IsValid();
    }

    bool TilemapLoader::LoadTMX(const std::string& filepath, ECS::TilemapComponent& outMap) {
#ifdef SAGE_HAS_TINYXML2
    outMap.layers.clear();
    outMap.tilesets.clear();
    outMap.objectLayers.clear();

        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(filepath.c_str()) != tinyxml2::XML_SUCCESS) {
            SAGE_ERROR("TilemapLoader::LoadTMX - Failed to load file: {}", filepath);
            return false;
        }

        auto* mapNode = doc.FirstChildElement("map");
        if (!mapNode) {
            SAGE_ERROR("TilemapLoader::LoadTMX - No <map> element found");
            return false;
        }

        outMap.mapWidth = mapNode->IntAttribute("width", 0);
        outMap.mapHeight = mapNode->IntAttribute("height", 0);
        outMap.tileWidth = mapNode->IntAttribute("tilewidth", 16);
        outMap.tileHeight = mapNode->IntAttribute("tileheight", 16);

        auto toLower = [](const char* value) {
            std::string lower = value ? value : "";
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            return lower;
        };

        // Parse orientation
        outMap.orientation = ECS::TilemapOrientation::Orthogonal;
        if (const char* orientationAttr = mapNode->Attribute("orientation")) {
            const std::string orientation = toLower(orientationAttr);
            if (orientation == "orthogonal") {
                outMap.orientation = ECS::TilemapOrientation::Orthogonal;
            } else if (orientation == "isometric") {
                outMap.orientation = ECS::TilemapOrientation::Isometric;
            } else if (orientation == "staggered") {
                outMap.orientation = ECS::TilemapOrientation::Staggered;
            } else if (orientation == "hexagonal") {
                outMap.orientation = ECS::TilemapOrientation::Hexagonal;
            } else {
                SAGE_WARN("TilemapLoader::LoadTMX - Unknown orientation '{}', defaulting to orthogonal", orientationAttr);
            }
        }
        
        // Parse stagger axis (for staggered/hexagonal)
        if (const char* staggerAxisAttr = mapNode->Attribute("staggeraxis")) {
            const std::string staggerAxis = toLower(staggerAxisAttr);
            if (staggerAxis == "x") {
                outMap.staggerAxis = ECS::TilemapStaggerAxis::X;
            } else if (staggerAxis == "y") {
                outMap.staggerAxis = ECS::TilemapStaggerAxis::Y;
            }
        }
        
        // Parse stagger index (for staggered/hexagonal)
        if (const char* staggerIndexAttr = mapNode->Attribute("staggerindex")) {
            const std::string staggerIndex = toLower(staggerIndexAttr);
            if (staggerIndex == "even") {
                outMap.staggerIndex = ECS::TilemapStaggerIndex::Even;
            } else if (staggerIndex == "odd") {
                outMap.staggerIndex = ECS::TilemapStaggerIndex::Odd;
            }
        }
        
        // Parse hex side length (for hexagonal)
        if (mapNode->Attribute("hexsidelength")) {
            outMap.hexSideLength = mapNode->IntAttribute("hexsidelength", 0);
        }

        std::filesystem::path tmxAbsolute = std::filesystem::absolute(filepath);
        TMXContext baseContext{};
        baseContext.mapDirectory = tmxAbsolute.parent_path().lexically_normal();
        baseContext.assetsRoot = FindAssetsRoot(baseContext.mapDirectory);

        // Parse tilesets
        for (auto* tsNode = mapNode->FirstChildElement("tileset"); tsNode; tsNode = tsNode->NextSiblingElement("tileset")) {
            ECS::TilesetInfo tileset;
            int firstGID = tsNode->IntAttribute("firstgid", 1);

            const char* externalSource = tsNode->Attribute("source");
            tinyxml2::XMLElement* metadataRoot = tsNode;

            if (externalSource && *externalSource) {
                auto tsxPath = ResolveRelativePath(baseContext.mapDirectory, externalSource);
                if (tsxPath.empty()) {
                    SAGE_ERROR("TilemapLoader::LoadTMX - Tileset with firstgid {} has invalid TSX path '{}'", firstGID, externalSource);
                    continue;
                }

                tinyxml2::XMLDocument tsxDoc;
                if (tsxDoc.LoadFile(tsxPath.string().c_str()) != tinyxml2::XML_SUCCESS) {
                    SAGE_ERROR("TilemapLoader::LoadTMX - Failed to load TSX '{}': {}", tsxPath.string(), tsxDoc.ErrorStr());
                    continue;
                }

                auto* tsxRoot = tsxDoc.FirstChildElement("tileset");
                if (!tsxRoot) {
                    SAGE_ERROR("TilemapLoader::LoadTMX - TSX '{}' missing <tileset> root", tsxPath.string());
                    continue;
                }

                TMXContext tsxContext = baseContext;
                tsxContext.mapDirectory = tsxPath.parent_path().lexically_normal();

                if (!PopulateTilesetFromNode(tsxRoot, tsxContext, firstGID, outMap.tileWidth, outMap.tileHeight, tileset)) {
                    continue;
                }
                metadataRoot = tsxRoot;
            } else {
                if (!PopulateTilesetFromNode(tsNode, baseContext, firstGID, outMap.tileWidth, outMap.tileHeight, tileset)) {
                    continue;
                }
            }

            PopulateTilesetTileMetadata(metadataRoot, tileset);
            outMap.tilesets.push_back(tileset);
        }

        auto findTilesetForGid = [&](int gid) -> const ECS::TilesetInfo* {
            for (int i = static_cast<int>(outMap.tilesets.size()) - 1; i >= 0; --i) {
                if (gid >= outMap.tilesets[i].firstGID) {
                    return &outMap.tilesets[i];
                }
            }
            return nullptr;
        };

        LayerContext rootContext;
        if (const char* mapTintAttr = mapNode->Attribute("tintcolor")) {
            Color mapTint;
            if (ParseTiledColorString(mapTintAttr, mapTint)) {
                rootContext.tint = mapTint;
            }
        }

        std::function<void(tinyxml2::XMLElement*, const LayerContext&)> parseLayerNode;
        parseLayerNode = [&](tinyxml2::XMLElement* node, const LayerContext& parentContext) {
            if (!node || !node->Name()) {
                return;
            }

            const char* nodeName = node->Name();

            if (std::strcmp(nodeName, "group") == 0) {
                LayerContext groupContext = parentContext;
                const bool visible = node->IntAttribute("visible", 1) == 1;
                const float opacity = node->FloatAttribute("opacity", 1.0f);
                groupContext.visible = parentContext.visible && visible;
                groupContext.opacity = parentContext.opacity * opacity;
                groupContext.offset.x += node->FloatAttribute("offsetx", 0.0f);
                groupContext.offset.y += node->FloatAttribute("offsety", 0.0f);
                groupContext.parallax.x *= node->FloatAttribute("parallaxx", 1.0f);
                groupContext.parallax.y *= node->FloatAttribute("parallaxy", 1.0f);

                if (const char* tintAttr = node->Attribute("tintcolor")) {
                    Color tint;
                    if (ParseTiledColorString(tintAttr, tint)) {
                        groupContext.tint = MultiplyColor(parentContext.tint, tint);
                    }
                }

                for (auto* child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
                    parseLayerNode(child, groupContext);
                }
                return;
            }

            if (std::strcmp(nodeName, "layer") == 0) {
                ECS::TilemapLayer layer;
                layer.name = node->Attribute("name") ? node->Attribute("name") : "";
                layer.width = node->IntAttribute("width", outMap.mapWidth);
                layer.height = node->IntAttribute("height", outMap.mapHeight);
                layer.visible = node->IntAttribute("visible", 1) == 1;
                layer.opacity = node->FloatAttribute("opacity", 1.0f);
                layer.parallaxFactor.x = node->FloatAttribute("parallaxx", 1.0f);
                layer.parallaxFactor.y = node->FloatAttribute("parallaxy", 1.0f);
                layer.offset.x = node->FloatAttribute("offsetx", 0.0f);
                layer.offset.y = node->FloatAttribute("offsety", 0.0f);

                if (const char* tintAttr = node->Attribute("tintcolor")) {
                    Color tintColor;
                    if (ParseTiledColorString(tintAttr, tintColor)) {
                        layer.tint = tintColor;
                    }
                }

                if (auto* propsNode = node->FirstChildElement("properties")) {
                    for (auto* propNode = propsNode->FirstChildElement("property"); propNode; propNode = propNode->NextSiblingElement("property")) {
                        std::string propName = propNode->Attribute("name") ? propNode->Attribute("name") : "";
                        if (propName == "collision") {
                            layer.collision = propNode->BoolAttribute("value", false);
                        }
                    }
                }

                if (auto* dataNode = node->FirstChildElement("data")) {
                    std::string encoding = dataNode->Attribute("encoding") ? dataNode->Attribute("encoding") : "";
                    std::string compression = dataNode->Attribute("compression") ? dataNode->Attribute("compression") : "";

                    if (encoding == "csv") {
                        const char* text = dataNode->GetText();
                        if (text) {
                            std::stringstream ss(text);
                            std::string cell;
                            int cellIndex = 0;
                            int invalidCells = 0;
                            constexpr int kMaxCsvWarnings = 5;
                            while (std::getline(ss, cell, ',')) {
                                size_t start = cell.find_first_not_of(" \t\r\n");
                                size_t end = cell.find_last_not_of(" \t\r\n");
                                std::string trimmed = (start == std::string::npos) ? "" : cell.substr(start, end - start + 1);
                                ++cellIndex;
                                if (trimmed.empty()) {
                                    layer.tiles.push_back(0);
                                    continue;
                                }
                                try {
                                    layer.tiles.push_back(std::stoi(trimmed));
                                } catch (...) {
                                    layer.tiles.push_back(0);
                                    ++invalidCells;
                                    if (invalidCells <= kMaxCsvWarnings) {
                                        SAGE_WARN("TilemapLoader::LoadTMX - Layer '{}' has invalid CSV value '{}' (cell {}). Treated as empty tile.", layer.name, trimmed, cellIndex);
                                    }
                                }
                            }
                            if (invalidCells > kMaxCsvWarnings) {
                                SAGE_WARN("TilemapLoader::LoadTMX - Layer '{}' CSV parsing suppressed {} additional invalid values", layer.name, invalidCells - kMaxCsvWarnings);
                            }
                        }
                    } else if (encoding == "base64") {
                        const char* text = dataNode->GetText();
                        if (text) {
                            std::string b64 = text;
                            b64.erase(std::remove_if(b64.begin(), b64.end(), [](unsigned char c){ return std::isspace(c); }), b64.end());
                            std::vector<uint8_t> bytes;
                            if (DecodeBase64(b64, bytes, layer.name)) {
                                std::vector<uint8_t> tileBytes;
                                const int64_t tileCountEstimate = static_cast<int64_t>(layer.width) * static_cast<int64_t>(layer.height);
                                const std::size_t expectedTiles = tileCountEstimate > 0 ? static_cast<std::size_t>(tileCountEstimate) : 0;
                                const std::size_t expectedBytes = expectedTiles * sizeof(uint32_t);

                                if (DecompressLayerData(bytes, compression, layer.name, expectedBytes, tileBytes)) {
                                    if ((tileBytes.size() % sizeof(uint32_t)) != 0) {
                                        SAGE_WARN("TilemapLoader::LoadTMX - Layer '{}' decompressed byte count {} is not divisible by 4", layer.name, tileBytes.size());
                                    }
                                    const std::size_t count = tileBytes.size() / sizeof(uint32_t);
                                    layer.tiles.reserve(layer.tiles.size() + count);
                                    for (std::size_t i = 0; i < count; ++i) {
                                        const uint32_t gidLE = static_cast<uint32_t>(tileBytes[i * 4]) |
                                            (static_cast<uint32_t>(tileBytes[i * 4 + 1]) << 8) |
                                            (static_cast<uint32_t>(tileBytes[i * 4 + 2]) << 16) |
                                            (static_cast<uint32_t>(tileBytes[i * 4 + 3]) << 24);
                                        layer.tiles.push_back(static_cast<int>(gidLE));
                                    }
                                } else {
                                    SAGE_WARN("TilemapLoader::LoadTMX - Failed to decompress base64 layer '{}'", layer.name);
                                }
                            } else {
                                SAGE_WARN("TilemapLoader::LoadTMX - Failed to decode base64 layer '{}'", layer.name);
                            }
                        }
                    } else {
                        for (auto* tileNode = dataNode->FirstChildElement("tile"); tileNode; tileNode = tileNode->NextSiblingElement("tile")) {
                            layer.tiles.push_back(tileNode->IntAttribute("gid", 0));
                        }
                    }
                }

                const size_t expectedCount = static_cast<size_t>(std::max(0, layer.width * layer.height));
                if (layer.tiles.size() < expectedCount) {
                    layer.tiles.resize(expectedCount, 0);
                } else if (layer.tiles.size() > expectedCount && expectedCount > 0) {
                    layer.tiles.resize(expectedCount);
                }

                ApplyContextToTileLayer(parentContext, layer);

                outMap.layers.push_back(layer);
                return;
            }

            if (std::strcmp(nodeName, "objectgroup") == 0) {
                ECS::TilemapObjectLayer objectLayer;
                objectLayer.name = node->Attribute("name") ? node->Attribute("name") : "";
                objectLayer.visible = node->IntAttribute("visible", 1) == 1;
                objectLayer.opacity = node->FloatAttribute("opacity", 1.0f);
                objectLayer.offset.x = node->FloatAttribute("offsetx", 0.0f);
                objectLayer.offset.y = node->FloatAttribute("offsety", 0.0f);
                objectLayer.parallaxFactor.x = node->FloatAttribute("parallaxx", 1.0f);
                objectLayer.parallaxFactor.y = node->FloatAttribute("parallaxy", 1.0f);

                if (const char* tintAttr = node->Attribute("tintcolor")) {
                    Color tintColor;
                    if (ParseTiledColorString(tintAttr, tintColor)) {
                        objectLayer.tint = tintColor;
                    }
                }

                if (auto* propsNode = node->FirstChildElement("properties")) {
                    for (auto* propNode = propsNode->FirstChildElement("property"); propNode; propNode = propNode->NextSiblingElement("property")) {
                        std::string propName = propNode->Attribute("name") ? propNode->Attribute("name") : "";
                        if (propName == "collision") {
                            objectLayer.collision = propNode->BoolAttribute("value", false);
                        }
                    }
                }

                for (auto* objectNode = node->FirstChildElement("object"); objectNode; objectNode = objectNode->NextSiblingElement("object")) {
                    const uint32_t rawGid = static_cast<uint32_t>(objectNode->UnsignedAttribute("gid", 0));
                    const uint32_t normalizedGid = rawGid & ~kFlipMask;

                    ECS::TilemapSprite sprite;
                    sprite.name = objectNode->Attribute("name") ? objectNode->Attribute("name") : "";
                    sprite.visible = objectNode->IntAttribute("visible", 1) == 1;
                    sprite.rotation = objectNode->FloatAttribute("rotation", 0.0f);
                    sprite.gid = normalizedGid == 0u ? 0u : rawGid;
                    sprite.position.x = objectNode->FloatAttribute("x", 0.0f);
                    sprite.position.y = objectNode->FloatAttribute("y", 0.0f);

                    float width = objectNode->FloatAttribute("width", 0.0f);
                    float height = objectNode->FloatAttribute("height", 0.0f);

                    if (normalizedGid == 0u) {
                        sprite.size = { width, height };
                        if (objectNode->FirstChildElement("ellipse")) {
                            sprite.shape = ECS::TilemapObjectShape::Ellipse;
                        } else if (auto* poly = objectNode->FirstChildElement("polygon")) {
                            sprite.shape = ECS::TilemapObjectShape::Polygon;
                            ParsePointString(poly->Attribute("points"), objectLayer.name, sprite.name, sprite.points);
                        } else if (auto* polyline = objectNode->FirstChildElement("polyline")) {
                            sprite.shape = ECS::TilemapObjectShape::Polyline;
                            ParsePointString(polyline->Attribute("points"), objectLayer.name, sprite.name, sprite.points);
                        } else if (objectNode->FirstChildElement("text")) {
                            sprite.shape = ECS::TilemapObjectShape::Text;
                            if (auto* textElem = objectNode->FirstChildElement("text")) {
                                if (const char* text = textElem->GetText()) {
                                    sprite.text = text;
                                }
                            }
                        } else if (width <= 0.0f && height <= 0.0f) {
                            sprite.shape = ECS::TilemapObjectShape::Point;
                        } else {
                            sprite.shape = ECS::TilemapObjectShape::Rectangle;
                        }
                    } else {
                        if (width <= 0.0f || height <= 0.0f) {
                            if (const auto* ts = findTilesetForGid(static_cast<int>(normalizedGid))) {
                                width = static_cast<float>(ts->tileWidth);
                                height = static_cast<float>(ts->tileHeight);
                            } else {
                                width = static_cast<float>(outMap.tileWidth);
                                height = static_cast<float>(outMap.tileHeight);
                            }
                        }

                        sprite.size = { width, height };
                        sprite.shape = ECS::TilemapObjectShape::Tile;
                    }

                    float spriteOpacity = objectNode->FloatAttribute("opacity", 1.0f);
                    spriteOpacity = std::clamp(spriteOpacity, 0.0f, 1.0f);
                    sprite.tint.a *= spriteOpacity;

                    if (const char* colorAttr = objectNode->Attribute("color")) {
                        Color objectColor;
                        if (ParseTiledColorString(colorAttr, objectColor)) {
                            sprite.tint = MultiplyColor(objectColor, sprite.tint);
                        }
                    }

                    objectLayer.sprites.push_back(sprite);
                }

                ApplyContextToObjectLayer(parentContext, objectLayer);

                if (!objectLayer.sprites.empty() || objectLayer.collision) {
                    outMap.objectLayers.push_back(objectLayer);
                }
                return;
            }

            if (std::strcmp(nodeName, "imagelayer") == 0) {
                ECS::TilemapImageLayer imageLayer;
                imageLayer.name = node->Attribute("name") ? node->Attribute("name") : "";
                imageLayer.visible = node->IntAttribute("visible", 1) == 1;
                imageLayer.opacity = node->FloatAttribute("opacity", 1.0f);
                imageLayer.offset.x = node->FloatAttribute("offsetx", 0.0f);
                imageLayer.offset.y = node->FloatAttribute("offsety", 0.0f);
                imageLayer.parallaxFactor.x = node->FloatAttribute("parallaxx", 1.0f);
                imageLayer.parallaxFactor.y = node->FloatAttribute("parallaxy", 1.0f);
                imageLayer.repeatX = node->BoolAttribute("repeatx", false);
                imageLayer.repeatY = node->BoolAttribute("repeaty", false);

                if (const char* tintAttr = node->Attribute("tintcolor")) {
                    Color tintColor;
                    if (ParseTiledColorString(tintAttr, tintColor)) {
                        imageLayer.tint = tintColor;
                    }
                }

                // Parse <image> element
                if (auto* imageNode = node->FirstChildElement("image")) {
                    if (const char* source = imageNode->Attribute("source")) {
                        imageLayer.imagePath = source;
                        
                        // Load texture
                        std::filesystem::path mapDir = std::filesystem::path(filepath).parent_path();
                        std::filesystem::path imagePath = mapDir / imageLayer.imagePath;
                        if (!imagePath.is_absolute()) {
                            imagePath = std::filesystem::absolute(imagePath);
                        }
                        
                        imageLayer.texture = ResourceManager::Get().Load<Texture>(imagePath.string());
                        if (!imageLayer.texture) {
                            SAGE_WARN("TilemapLoader::LoadTMX - Failed to load image layer texture: {}", imagePath.string());
                        }
                    }
                }

                ApplyContextToImageLayer(parentContext, imageLayer);

                outMap.imageLayers.push_back(imageLayer);
                return;
            }
        };

        for (auto* node = mapNode->FirstChildElement(); node; node = node->NextSiblingElement()) {
            const char* nodeName = node->Name();
            if (!nodeName) {
                continue;
            }

            if (std::strcmp(nodeName, "layer") == 0 || std::strcmp(nodeName, "objectgroup") == 0 || std::strcmp(nodeName, "group") == 0 || std::strcmp(nodeName, "imagelayer") == 0) {
                parseLayerNode(node, rootContext);
            }
        }

        return outMap.IsValid();
#else
        SAGE_WARN("TilemapLoader::LoadTMX - tinyxml2 not available, TMX format disabled");
        return false;
#endif
    }

} // namespace SAGE
