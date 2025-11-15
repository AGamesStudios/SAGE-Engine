#include "STMLoader.h"
#include "Core/ResourceManager.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Utils/PNGLoader.h"
#include "Core/Logger.h"
#include <filesystem>
#include <cstring>

namespace SAGE {

#pragma pack(push, 1)
struct STMHeader {
    char magic[4];
    uint32_t mapWidth;
    uint32_t mapHeight;
    uint32_t tileWidth;
    uint32_t tileHeight;
    uint32_t tilesetCount;
    uint32_t layerCount;
    uint32_t flags;
};
#pragma pack(pop)

// Flag bits (must match exporter & docs)
static constexpr uint32_t STM_FLAG_EMBED_TEXTURES       = 1u << 0;
static constexpr uint32_t STM_FLAG_COMPRESS_TILE_LAYERS = 1u << 1;
static constexpr uint32_t STM_FLAG_INCLUDE_OBJECT_LAYERS= 1u << 2;
static constexpr uint32_t STM_FLAG_INCLUDE_IMAGE_LAYERS = 1u << 3;

bool STMLoader::Load(const std::string& filepath, ECS::TilemapComponent& outMap) {
    SAGE_INFO("STMLoader::Load - Loading: {}", filepath);
    if (!std::filesystem::exists(filepath)) {
        SAGE_ERROR("STMLoader::Load - File not found: {}", filepath);
        return false;
    }
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        SAGE_ERROR("STMLoader::Load - Failed to open file: {}", filepath);
        return false;
    }

    STMHeader header{};
    file.read(reinterpret_cast<char*>(&header), sizeof(STMHeader));
    if (!file) { SAGE_ERROR("STMLoader::Load - Failed to read header block"); return false; }
    if (std::memcmp(header.magic, "STM1", 4) != 0) { SAGE_ERROR("STMLoader::Load - Bad magic"); return false; }
    if (header.mapWidth==0 || header.mapHeight==0 || header.tileWidth==0 || header.tileHeight==0) { SAGE_ERROR("STMLoader::Load - Invalid dimensions"); return false; }

    // Fill map component
    outMap.mapWidth  = header.mapWidth;
    outMap.mapHeight = header.mapHeight;
    outMap.tileWidth = header.tileWidth;
    outMap.tileHeight= header.tileHeight;

    SAGE_INFO("STMLoader::Load - Header ok {}x{} tile={}x{} flags=0x{:08X}", header.mapWidth, header.mapHeight, header.tileWidth, header.tileHeight, header.flags);

    if (!ReadTilesets(file, outMap, header.tilesetCount, header.flags)) return false;
    if (!ReadLayers(file, outMap, header.layerCount, header.flags)) return false;

    SAGE_INFO("STMLoader::Load - Complete tilesets={} layers(tile)={} objects={} images={}", outMap.tilesets.size(), outMap.layers.size(), outMap.objectLayers.size(), outMap.imageLayers.size());
    return true;
}

bool STMLoader::ReadTilesets(std::ifstream& file, ECS::TilemapComponent& outMap, uint32_t count, uint32_t flags) {
    bool embedded = (flags & STM_FLAG_EMBED_TEXTURES)!=0;
    for (uint32_t i=0;i<count;++i){
        ECS::TilesetInfo tileset;
        file.read(reinterpret_cast<char*>(&tileset.firstGID), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&tileset.tileCount), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&tileset.columns), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&tileset.tileWidth), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&tileset.tileHeight), sizeof(uint32_t));
        uint16_t nameLen; file.read(reinterpret_cast<char*>(&nameLen), sizeof(uint16_t)); tileset.name = ReadString(file, nameLen);
        uint16_t pathLen; file.read(reinterpret_cast<char*>(&pathLen), sizeof(uint16_t)); tileset.texturePath = ReadString(file, pathLen);
        std::vector<uint8_t> embeddedPNG;
        if (embedded){
            uint32_t pngSize=0; file.read(reinterpret_cast<char*>(&pngSize), sizeof(uint32_t));
            if (pngSize>0){ embeddedPNG.resize(pngSize); file.read(reinterpret_cast<char*>(embeddedPNG.data()), pngSize); }
        }
        uint32_t animCount=0; file.read(reinterpret_cast<char*>(&animCount), sizeof(uint32_t));
        for (uint32_t a=0;a<animCount;++a){
            ECS::TileAnimation anim; file.read(reinterpret_cast<char*>(&anim.localTileID), sizeof(uint32_t)); uint32_t frameCount=0; file.read(reinterpret_cast<char*>(&frameCount), sizeof(uint32_t));
            for (uint32_t f=0; f<frameCount; ++f){ ECS::AnimationFrame frame; file.read(reinterpret_cast<char*>(&frame.localTileID), sizeof(uint32_t)); file.read(reinterpret_cast<char*>(&frame.durationMs), sizeof(uint32_t)); anim.frames.push_back(frame);} tileset.animations.push_back(anim);
        }
        if (!file){ SAGE_ERROR("STMLoader::ReadTilesets - File error tileset {}", i); return false; }
        if (embedded && !embeddedPNG.empty()){
#ifdef _WIN32
            auto decoded = Image::DecodeWithWIC(embeddedPNG.data(), embeddedPNG.size());
#else
            auto decoded = Image::PNGImageDecoder::LoadFromMemory(embeddedPNG.data(), embeddedPNG.size());
#endif
            if (decoded.IsValid()){
                tileset.texture = CreateRef<Texture>(decoded.width, decoded.height, Texture::Format::RGBA8, decoded.pixels.data(), false);
                tileset.texturePath = "<embedded:" + tileset.name + ">";
            } else {
                SAGE_WARN("STMLoader::ReadTilesets - Failed decode embedded tileset '{}'", tileset.name);
            }
        } else if (!tileset.texturePath.empty()) {
            tileset.texture = ResourceManager::Get().Load<Texture>(tileset.texturePath);
        }
        outMap.tilesets.push_back(tileset);
        SAGE_INFO("STMLoader::ReadTilesets - '{}' firstGID={} tiles={} embedded={}", tileset.name, tileset.firstGID, tileset.tileCount, embedded);
    }
    return true;
}

bool STMLoader::ReadLayers(std::ifstream& file, ECS::TilemapComponent& outMap, uint32_t count, uint32_t flags) {
    for (uint32_t i=0;i<count;++i){
        uint8_t type, visible; uint8_t reserved[2]; float opacity, offsetX, offsetY, parallaxX, parallaxY; uint32_t tintColor; uint16_t nameLen;
        file.read(reinterpret_cast<char*>(&type), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(&visible), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(reserved), 2);
        file.read(reinterpret_cast<char*>(&opacity), sizeof(float));
        file.read(reinterpret_cast<char*>(&offsetX), sizeof(float));
        file.read(reinterpret_cast<char*>(&offsetY), sizeof(float));
        file.read(reinterpret_cast<char*>(&parallaxX), sizeof(float));
        file.read(reinterpret_cast<char*>(&parallaxY), sizeof(float));
        file.read(reinterpret_cast<char*>(&tintColor), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(uint16_t));
        std::string name = ReadString(file, nameLen);
        if (!file){ SAGE_ERROR("STMLoader::ReadLayers - header fail layer {}", i); return false; }
        uint8_t r=(tintColor>>24)&0xFF, g=(tintColor>>16)&0xFF, b=(tintColor>>8)&0xFF, a=tintColor&0xFF; Color tint{r/255.0f,g/255.0f,b/255.0f,a/255.0f}; Float2 offset{offsetX,offsetY}; Float2 parallax{parallaxX,parallaxY}; bool vis=(visible!=0);
        if (type==0){ if (!ReadTileLayer(file, outMap, name, vis, opacity, offset, parallax, tint, flags)) return false; }
        else if (type==1){ if (!ReadObjectLayer(file, outMap, name, vis, opacity, offset, parallax, tint, flags)) return false; }
        else if (type==2){ if (!ReadImageLayer(file, outMap, name, vis, opacity, offset, parallax, tint, flags)) return false; }
        else { SAGE_WARN("STMLoader::ReadLayers - Unknown type {}", (int)type); return false; }
    }
    return true;
}

bool STMLoader::ReadTileLayer(std::ifstream& file, ECS::TilemapComponent& outMap, const std::string& name, bool visible, float opacity, const Float2& offset, const Float2& parallax, const Color& tint, uint32_t flags){
    ECS::TilemapLayer layer; layer.name=name; layer.visible=visible; layer.opacity=opacity; layer.offset=offset; layer.parallaxFactor=parallax; layer.tint=tint;
    file.read(reinterpret_cast<char*>(&layer.width), sizeof(uint32_t)); file.read(reinterpret_cast<char*>(&layer.height), sizeof(uint32_t)); if (!file){ SAGE_ERROR("STMLoader::ReadTileLayer - dims fail '{}'", name); return false; }
    bool compressionEnabled = (flags & STM_FLAG_COMPRESS_TILE_LAYERS)!=0; uint8_t compType=0; if (compressionEnabled){ file.read(reinterpret_cast<char*>(&compType), sizeof(uint8_t)); }
    size_t total = static_cast<size_t>(layer.width)*static_cast<size_t>(layer.height); layer.tiles.resize(total);
    if (compressionEnabled && compType==1){ if (!DecompressTileDataRLE(file, layer.tiles, layer.width, layer.height)) return false; }
    else { file.read(reinterpret_cast<char*>(layer.tiles.data()), total*sizeof(int)); if (!file){ SAGE_ERROR("STMLoader::ReadTileLayer - raw data fail '{}'", name); return false; } }
    outMap.layers.push_back(layer);
    return true;
}

bool STMLoader::DecompressTileDataRLE(std::ifstream& file, std::vector<int>& outTiles, uint32_t width, uint32_t height){
    uint32_t runCount=0; file.read(reinterpret_cast<char*>(&runCount), sizeof(uint32_t)); if (!file){ SAGE_ERROR("STMLoader::DecompressTileDataRLE - runCount fail"); return false; }
    size_t total = static_cast<size_t>(width)*height; size_t written=0;
    for (uint32_t i=0;i<runCount;++i){ uint32_t runLength=0, gid=0; file.read(reinterpret_cast<char*>(&runLength), sizeof(uint32_t)); file.read(reinterpret_cast<char*>(&gid), sizeof(uint32_t)); if (!file){ SAGE_ERROR("STMLoader::DecompressTileDataRLE - run {} fail", i); return false; } for (uint32_t r=0;r<runLength && written<total;++r){ outTiles[written++] = (int)gid; } }
    if (written!=total){ SAGE_WARN("STMLoader::DecompressTileDataRLE - written {} expected {}", written, total); }
    return true;
}

bool STMLoader::ReadObjectLayer(std::ifstream& file, ECS::TilemapComponent& outMap, const std::string& name, bool visible, float opacity, const Float2& offset, const Float2& parallax, const Color& tint, uint32_t flags){
    if ((flags & STM_FLAG_INCLUDE_OBJECT_LAYERS)==0){ SAGE_WARN("STMLoader::ReadObjectLayer - flag not set but data present '{}'", name); }
    uint32_t objectCount=0; file.read(reinterpret_cast<char*>(&objectCount), sizeof(uint32_t)); if (!file){ SAGE_ERROR("STMLoader::ReadObjectLayer - objectCount fail '{}'", name); return false; }
    ECS::TilemapObjectLayer layer; layer.name=name; layer.visible=visible; layer.opacity=opacity; layer.offset=offset; layer.parallaxFactor=parallax; layer.tint=tint;
    for (uint32_t i=0;i<objectCount;++i){ uint8_t shape; file.read(reinterpret_cast<char*>(&shape), sizeof(uint8_t)); float x,y,w,h,rot; uint32_t gid; uint8_t objVis; file.read(reinterpret_cast<char*>(&x), sizeof(float)); file.read(reinterpret_cast<char*>(&y), sizeof(float)); file.read(reinterpret_cast<char*>(&w), sizeof(float)); file.read(reinterpret_cast<char*>(&h), sizeof(float)); file.read(reinterpret_cast<char*>(&rot), sizeof(float)); file.read(reinterpret_cast<char*>(&gid), sizeof(uint32_t)); file.read(reinterpret_cast<char*>(&objVis), sizeof(uint8_t)); uint16_t nameLen; file.read(reinterpret_cast<char*>(&nameLen), sizeof(uint16_t)); std::string objName = ReadString(file, nameLen); uint16_t textLen; file.read(reinterpret_cast<char*>(&textLen), sizeof(uint16_t)); std::string text = ReadString(file, textLen); if (!file){ SAGE_ERROR("STMLoader::ReadObjectLayer - object {} fail '{}'", i, name); return false; } ECS::TilemapSprite sprite; sprite.name=objName; sprite.position={x,y}; sprite.size={w,h}; sprite.rotation=rot; sprite.gid=gid; sprite.visible=(objVis!=0); sprite.text=text; sprite.shape=static_cast<ECS::TilemapObjectShape>(shape); layer.sprites.push_back(sprite);} outMap.objectLayers.push_back(layer); return true; }

bool STMLoader::ReadImageLayer(std::ifstream& file, ECS::TilemapComponent& outMap, const std::string& name, bool visible, float opacity, const Float2& offset, const Float2& parallax, const Color& tint, uint32_t flags){
    if ((flags & STM_FLAG_INCLUDE_IMAGE_LAYERS)==0){ SAGE_WARN("STMLoader::ReadImageLayer - flag not set but data present '{}'", name); }
    uint16_t pathLen; file.read(reinterpret_cast<char*>(&pathLen), sizeof(uint16_t)); std::string path = ReadString(file, pathLen); uint32_t pngSize=0; file.read(reinterpret_cast<char*>(&pngSize), sizeof(uint32_t)); std::vector<uint8_t> bytes; if (pngSize>0){ bytes.resize(pngSize); file.read(reinterpret_cast<char*>(bytes.data()), pngSize); }
    if (!file){ SAGE_ERROR("STMLoader::ReadImageLayer - payload fail '{}'", name); return false; }
    ECS::TilemapImageLayer layer; layer.name=name; layer.imagePath=path; layer.visible=visible; layer.opacity=opacity; layer.offset=offset; layer.parallaxFactor=parallax; layer.tint=tint;
    if (pngSize>0){
#ifdef _WIN32
        auto decoded = Image::DecodeWithWIC(bytes.data(), bytes.size());
#else
        auto decoded = Image::PNGImageDecoder::LoadFromMemory(bytes.data(), bytes.size());
#endif
        if (decoded.IsValid()){ layer.texture = CreateRef<Texture>(decoded.width, decoded.height, Texture::Format::RGBA8, decoded.pixels.data(), false); layer.imagePath = "<embedded:"+name+">"; }
        else { SAGE_WARN("STMLoader::ReadImageLayer - decode fail '{}'", name); }
    }
    outMap.imageLayers.push_back(layer); return true; }

std::string STMLoader::ReadString(std::ifstream& file, uint16_t length){ if (length==0) return ""; std::vector<char> buf(length); file.read(buf.data(), length); return std::string(buf.begin(), buf.end()); }

bool STMLoader::Validate(const std::string& filepath){ std::ifstream file(filepath, std::ios::binary); if (!file.is_open()) return false; char magic[4]; file.read(magic,4); return std::memcmp(magic,"STM1",4)==0; }

} // namespace SAGE
