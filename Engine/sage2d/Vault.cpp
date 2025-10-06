#include "Vault.h"

#include "Capabilities.h"
#include "Types.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace sage2d {

namespace {

    std::string trim(std::string_view view) {
        auto begin = view.begin();
        auto end = view.end();
        while (begin != end && std::isspace(static_cast<unsigned char>(*begin))) {
            ++begin;
        }
        while (end != begin) {
            auto prev = end;
            --prev;
            if (!std::isspace(static_cast<unsigned char>(*prev))) {
                break;
            }
            end = prev;
        }
        return std::string(begin, end);
    }

    std::string stripQuotes(std::string value) {
        if (value.empty()) {
            return value;
        }

        if (value.front() == '"' && value.back() == '"' && value.size() >= 2) {
            return value.substr(1, value.size() - 2);
        }
        if (value.front() == '\'' && value.back() == '\'' && value.size() >= 2) {
            return value.substr(1, value.size() - 2);
        }
        return value;
    }

    std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    std::optional<float> parseFloat(const std::string& value) {
        float result = 0.0f;
        auto trimmed = trim(value);
        auto str = stripQuotes(trimmed);
        auto ptr = str.data();
        auto endPtr = str.data() + str.size();
        std::string_view view(ptr, static_cast<std::size_t>(endPtr - ptr));
        try {
            result = std::stof(std::string(view));
            return result;
        }
        catch (...) {
            return std::nullopt;
        }
    }

    std::optional<int> parseInt(const std::string& value) {
        int result = 0;
        auto trimmed = trim(value);
        auto str = stripQuotes(trimmed);
        try {
            result = std::stoi(str);
            return result;
        }
        catch (...) {
            if (!str.empty()) {
                return static_cast<int>(str[0]);
            }
            return std::nullopt;
        }
    }

    std::optional<bool> parseBool(const std::string& value) {
        auto lowered = toLower(stripQuotes(trim(value)));
        if (lowered == "true" || lowered == "yes" || lowered == "on") {
            return true;
        }
        if (lowered == "false" || lowered == "no" || lowered == "off") {
            return false;
        }
        return std::nullopt;
    }

    std::optional<Vec2> parseVec2(const std::string& value) {
        auto trimmed = trim(value);
        if (trimmed.empty()) {
            return std::nullopt;
        }

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            auto inner = trimmed.substr(1, trimmed.size() - 2);
            std::stringstream stream(inner);
            std::string component;
            float values[2] = { 0.0f, 0.0f };
            int index = 0;
            while (std::getline(stream, component, ',') && index < 2) {
                if (auto parsed = parseFloat(component)) {
                    values[index++] = *parsed;
                }
            }
            if (index == 2) {
                return Vec2(values[0], values[1]);
            }
        }
        return std::nullopt;
    }

    std::string normalizeKey(std::string key) {
        auto trimmed = trim(key);
        std::string result;
        result.reserve(trimmed.size());
        for (char ch : trimmed) {
            unsigned char uch = static_cast<unsigned char>(ch);
            if (std::isalnum(uch)) {
                result.push_back(static_cast<char>(std::tolower(uch)));
            }
            else if (ch == '_' || ch == '-' || ch == ' ' || ch == '.') {
                if (!result.empty() && result.back() != '_') {
                    result.push_back('_');
                }
            }
        }
        return result;
    }

} // namespace

struct Vault::Document {
    std::unordered_map<std::string, std::string> root;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections;
};

Vault::Vault() = default;

ResId Vault::image(const std::filesystem::path& path) {
    return acquireFromPath(ResourceKind::Image, m_Images, path);
}

ResId Vault::sound(const std::filesystem::path& path) {
    return acquireFromPath(ResourceKind::Sound, m_Sounds, path);
}

ResId Vault::animation(const std::filesystem::path& path) {
    return acquireFromPath(ResourceKind::Animation, m_Animations, path);
}

ResId Vault::roleFromFile(const std::filesystem::path& path) {
    ResId id = acquireFromPath(ResourceKind::Role, m_Roles, path);
    if (auto* entry = getEntry(ResourceKind::Role, m_Roles, id); entry) {
        entry->source = path;
    }
    return id;
}

ResId Vault::registerRole(const std::string& name, Role role) {
    auto key = normalizeKey(name);
    auto it = m_RolesByName.find(key);
    if (it != m_RolesByName.end()) {
        if (auto* entry = getEntry(ResourceKind::Role, m_Roles, it->second); entry) {
            entry->payload = std::move(role);
            entry->payload.name = name;
            entry->loaded = true;
            entry->active = true;
            ++entry->refCount;
            return it->second;
        }
    }

    ResourceEntry<Role> entry;
    entry.source.clear();
    entry.payload = std::move(role);
    entry.payload.name = name;
    entry.refCount = 1;
    entry.active = true;
    entry.loaded = true;

    m_Roles.entries.emplace_back(std::move(entry));
    const std::uint32_t index = static_cast<std::uint32_t>(m_Roles.entries.size());
    ResId id = MakeResId(ResourceKind::Role, index);
    m_RolesByName[key] = id;
    return id;
}

ResId Vault::skinFromFile(const std::filesystem::path& path) {
    ResId id = acquireFromPath(ResourceKind::Skin, m_Skins, path);
    if (auto* entry = getEntry(ResourceKind::Skin, m_Skins, id); entry) {
        entry->source = path;
    }
    return id;
}

ResId Vault::registerSkin(const std::string& name, Skin skin) {
    auto key = normalizeKey(name);
    auto it = m_SkinsByName.find(key);
    if (it != m_SkinsByName.end()) {
        if (auto* entry = getEntry(ResourceKind::Skin, m_Skins, it->second); entry) {
            entry->payload = std::move(skin);
            entry->payload.name = name;
            entry->loaded = true;
            entry->active = true;
            ++entry->refCount;
            return it->second;
        }
    }

    ResourceEntry<Skin> entry;
    entry.source.clear();
    entry.payload = std::move(skin);
    entry.payload.name = name;
    entry.refCount = 1;
    entry.active = true;
    entry.loaded = true;

    m_Skins.entries.emplace_back(std::move(entry));
    const std::uint32_t index = static_cast<std::uint32_t>(m_Skins.entries.size());
    ResId id = MakeResId(ResourceKind::Skin, index);
    m_SkinsByName[key] = id;
    return id;
}

void Vault::retain(ResId id) {
    switch (GetKind(id)) {
    case ResourceKind::Image:
        if (auto* entry = getEntry(ResourceKind::Image, m_Images, id); entry) {
            ++entry->refCount;
            entry->active = true;
        }
        break;
    case ResourceKind::Sound:
        if (auto* entry = getEntry(ResourceKind::Sound, m_Sounds, id); entry) {
            ++entry->refCount;
            entry->active = true;
        }
        break;
    case ResourceKind::Animation:
        if (auto* entry = getEntry(ResourceKind::Animation, m_Animations, id); entry) {
            ++entry->refCount;
            entry->active = true;
        }
        break;
    case ResourceKind::Role:
        if (auto* entry = getEntry(ResourceKind::Role, m_Roles, id); entry) {
            ++entry->refCount;
            entry->active = true;
        }
        break;
    case ResourceKind::Skin:
        if (auto* entry = getEntry(ResourceKind::Skin, m_Skins, id); entry) {
            ++entry->refCount;
            entry->active = true;
        }
        break;
    default:
        break;
    }
}

void Vault::release(ResId id) {
    switch (GetKind(id)) {
    case ResourceKind::Image:
        releaseCacheEntry(m_Images, id);
        break;
    case ResourceKind::Sound:
        releaseCacheEntry(m_Sounds, id);
        break;
    case ResourceKind::Animation:
        releaseCacheEntry(m_Animations, id);
        break;
    case ResourceKind::Role: {
        std::string key;
        if (auto* entry = getEntry(ResourceKind::Role, m_Roles, id); entry) {
            if (!entry->payload.name.empty()) {
                key = normalizeKey(entry->payload.name);
            }
        }
        const bool removed = releaseCacheEntry(m_Roles, id);
        if (removed && !key.empty()) {
            auto it = m_RolesByName.find(key);
            if (it != m_RolesByName.end() && it->second == id) {
                m_RolesByName.erase(it);
            }
        }
        break;
    }
    case ResourceKind::Skin: {
        std::string key;
        if (auto* entry = getEntry(ResourceKind::Skin, m_Skins, id); entry) {
            if (!entry->payload.name.empty()) {
                key = normalizeKey(entry->payload.name);
            }
        }
        const bool removed = releaseCacheEntry(m_Skins, id);
        if (removed && !key.empty()) {
            auto it = m_SkinsByName.find(key);
            if (it != m_SkinsByName.end() && it->second == id) {
                m_SkinsByName.erase(it);
            }
        }
        break;
    }
    default:
        break;
    }
}

std::uint32_t Vault::refCount(ResId id) const {
    switch (GetKind(id)) {
    case ResourceKind::Image:
        return refCountFor(m_Images, id);
    case ResourceKind::Sound:
        return refCountFor(m_Sounds, id);
    case ResourceKind::Animation:
        return refCountFor(m_Animations, id);
    case ResourceKind::Role:
        return refCountFor(m_Roles, id);
    case ResourceKind::Skin:
        return refCountFor(m_Skins, id);
    default:
        return 0;
    }
}

const Vault::ImageResource* Vault::getImage(ResId id) const {
    if (auto* entry = getEntry(ResourceKind::Image, m_Images, id); entry) {
        const_cast<Vault*>(this)->ensureLoaded(*const_cast<ResourceEntry<ImageResource>*>(entry));
        return &entry->payload;
    }
    return nullptr;
}

const Vault::SoundResource* Vault::getSound(ResId id) const {
    if (auto* entry = getEntry(ResourceKind::Sound, m_Sounds, id); entry) {
        const_cast<Vault*>(this)->ensureLoaded(*const_cast<ResourceEntry<SoundResource>*>(entry));
        return &entry->payload;
    }
    return nullptr;
}

const Vault::AnimationResource* Vault::getAnimation(ResId id) const {
    if (auto* entry = getEntry(ResourceKind::Animation, m_Animations, id); entry) {
        const_cast<Vault*>(this)->ensureLoaded(*const_cast<ResourceEntry<AnimationResource>*>(entry));
        return &entry->payload;
    }
    return nullptr;
}

const Role* Vault::getRole(ResId id) const {
    if (auto* entry = getEntry(ResourceKind::Role, m_Roles, id); entry) {
        const_cast<Vault*>(this)->ensureLoaded(*const_cast<ResourceEntry<Role>*>(entry), id);
        return &entry->payload;
    }
    return nullptr;
}

const Role* Vault::getRole(const std::string& name) const {
    auto key = normalizeKey(name);
    auto it = m_RolesByName.find(key);
    if (it == m_RolesByName.end()) {
        return nullptr;
    }
    return getRole(it->second);
}

const Skin* Vault::getSkin(ResId id) const {
    if (auto* entry = getEntry(ResourceKind::Skin, m_Skins, id); entry) {
        const_cast<Vault*>(this)->ensureLoaded(*const_cast<ResourceEntry<Skin>*>(entry), id);
        return &entry->payload;
    }
    return nullptr;
}

const Skin* Vault::getSkin(const std::string& name) const {
    auto key = normalizeKey(name);
    auto it = m_SkinsByName.find(key);
    if (it == m_SkinsByName.end()) {
        return nullptr;
    }
    return getSkin(it->second);
}

void Vault::clear() {
    m_Images = ImageCache{};
    m_Sounds = SoundCache{};
    m_Animations = AnimationCache{};
    m_Roles = RoleCache{};
    m_Skins = SkinCache{};
    m_RolesByName.clear();
    m_SkinsByName.clear();
}

std::string Vault::canonicalKey(const std::filesystem::path& path) const {
    namespace fs = std::filesystem;
    try {
        return fs::weakly_canonical(path).generic_string();
    }
    catch (const std::exception&) {
        try {
            return fs::absolute(path).generic_string();
        }
        catch (const std::exception&) {
            return path.generic_string();
        }
    }
}

template<typename PayloadT>
ResId Vault::acquireFromPath(ResourceKind kind, ResourceCache<PayloadT>& cache, const std::filesystem::path& path) {
    auto key = canonicalKey(path);
    auto it = cache.keyLookup.find(key);
    if (it != cache.keyLookup.end()) {
        auto& entry = cache.entries[it->second];
        ++entry.refCount;
        entry.active = true;
        return MakeResId(kind, it->second + 1);
    }

    ResourceEntry<PayloadT> entry;
    entry.source = path;
    entry.refCount = 1;
    entry.active = true;
    entry.loaded = false;

    cache.entries.emplace_back(std::move(entry));
    const std::uint32_t index = static_cast<std::uint32_t>(cache.entries.size());
    cache.keyLookup.emplace(std::move(key), index - 1);
    return MakeResId(kind, index);
}

template<typename PayloadT>
Vault::ResourceEntry<PayloadT>* Vault::getEntry(ResourceKind kind, ResourceCache<PayloadT>& cache, ResId id) {
    if (!IsValid(id) || GetKind(id) != kind) {
        return nullptr;
    }
    const std::uint32_t index = GetIndex(id);
    if (index == 0 || index > cache.entries.size()) {
        return nullptr;
    }
    auto& entry = cache.entries[index - 1];
    if (!entry.active) {
        return nullptr;
    }
    return &entry;
}

template<typename PayloadT>
const Vault::ResourceEntry<PayloadT>* Vault::getEntry(ResourceKind kind, const ResourceCache<PayloadT>& cache, ResId id) const {
    if (!IsValid(id) || GetKind(id) != kind) {
        return nullptr;
    }
    const std::uint32_t index = GetIndex(id);
    if (index == 0 || index > cache.entries.size()) {
        return nullptr;
    }
    const auto& entry = cache.entries[index - 1];
    if (!entry.active) {
        return nullptr;
    }
    return &entry;
}

void Vault::ensureLoaded(ResourceEntry<ImageResource>& entry) const {
    if (entry.loaded) {
        return;
    }
    entry.payload.source = entry.source;
    entry.payload.exists = std::filesystem::exists(entry.source);
    entry.loaded = true;
}

void Vault::ensureLoaded(ResourceEntry<SoundResource>& entry) const {
    if (entry.loaded) {
        return;
    }
    entry.payload.source = entry.source;
    entry.payload.exists = std::filesystem::exists(entry.source);
    entry.loaded = true;
}

void Vault::ensureLoaded(ResourceEntry<AnimationResource>& entry) const {
    if (entry.loaded) {
        return;
    }
    entry.payload.source = entry.source;
    entry.payload.exists = std::filesystem::exists(entry.source);
    entry.loaded = true;
}

void Vault::ensureLoaded(ResourceEntry<Role>& entry, ResId id) {
    if (entry.loaded) {
        if (!entry.payload.name.empty()) {
            m_RolesByName[normalizeKey(entry.payload.name)] = id;
        }
        return;
    }
    if (entry.source.empty()) {
        entry.loaded = true;
        if (!entry.payload.name.empty()) {
            m_RolesByName[normalizeKey(entry.payload.name)] = id;
        }
        return;
    }

    auto doc = parseDocument(entry.source);
    entry.payload = buildRoleFromDocument(doc, entry.source);
    entry.loaded = true;
    if (!entry.payload.name.empty()) {
        m_RolesByName[normalizeKey(entry.payload.name)] = id;
    }
}

void Vault::ensureLoaded(ResourceEntry<Skin>& entry, ResId id) {
    if (entry.loaded) {
        if (!entry.payload.name.empty()) {
            m_SkinsByName[normalizeKey(entry.payload.name)] = id;
        }
        return;
    }
    if (entry.source.empty()) {
        entry.loaded = true;
        if (!entry.payload.name.empty()) {
            m_SkinsByName[normalizeKey(entry.payload.name)] = id;
        }
        return;
    }

    auto doc = parseDocument(entry.source);
    entry.payload = buildSkinFromDocument(doc, entry.source);
    entry.loaded = true;
    if (!entry.payload.name.empty()) {
        m_SkinsByName[normalizeKey(entry.payload.name)] = id;
    }
}

Vault::Document Vault::parseDocument(const std::filesystem::path& path) {
    Document doc;
    std::ifstream stream(path);
    if (!stream) {
        throw std::runtime_error("Failed to open document: " + path.generic_string());
    }

    std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    bool insideArray = false;
    std::string buffer;
    buffer.reserve(content.size());

    for (char ch : content) {
        if (ch == '[') {
            insideArray = true;
            buffer.push_back(ch);
            continue;
        }
        if (ch == ']') {
            insideArray = false;
            buffer.push_back(ch);
            continue;
        }
        if (ch == '{' || ch == '}') {
            buffer.push_back('\n');
            continue;
        }
        if (ch == ',' && !insideArray) {
            buffer.push_back('\n');
            continue;
        }
        buffer.push_back(ch);
    }

    std::stringstream ss(buffer);
    std::string line;
    std::string currentSection;
    while (std::getline(ss, line)) {
        auto trimmed = trim(line);
        if (trimmed.empty()) {
            continue;
        }
        if (trimmed[0] == '#' || (trimmed.size() >= 2 && trimmed[0] == '/' && trimmed[1] == '/')) {
            continue;
        }

        auto colonPos = trimmed.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        auto keyRaw = trimmed.substr(0, colonPos);
        auto valueRaw = trimmed.substr(colonPos + 1);
        auto key = stripQuotes(trim(keyRaw));
        auto value = trim(valueRaw);
        if (!value.empty() && value.back() == ',') {
            value.pop_back();
            value = trim(value);
        }
        if (!value.empty()) {
            value = stripQuotes(value);
        }

        if (value.empty()) {
            currentSection = normalizeKey(key);
            continue;
        }

        auto normalizedKey = normalizeKey(key);
        if (currentSection.empty()) {
            doc.root[normalizedKey] = value;
        }
        else {
            doc.sections[currentSection][normalizedKey] = value;
        }
    }

    return doc;
}

Role Vault::buildRoleFromDocument(const Document& doc, const std::filesystem::path& source) {
    const auto baseDir = source.has_parent_path() ? source.parent_path() : std::filesystem::path{};
    Role role;
    if (auto it = doc.root.find("name"); it != doc.root.end()) {
        role.name = stripQuotes(it->second);
    }
    else {
        role.name = source.stem().string();
    }

    if (auto secIt = doc.sections.find("sprite"); secIt != doc.sections.end()) {
        Sprite sprite;
        if (auto it = secIt->second.find("image"); it != secIt->second.end()) {
            auto resolved = baseDir / stripQuotes(it->second);
            sprite.image = image(resolved);
        }
        if (auto it = secIt->second.find("animation"); it != secIt->second.end()) {
            auto resolved = baseDir / stripQuotes(it->second);
            sprite.animation = animation(resolved);
        }
        if (auto it = secIt->second.find("size"); it != secIt->second.end()) {
            if (auto parsed = parseVec2(it->second)) {
                sprite.size = *parsed;
            }
        }
        if (auto it = secIt->second.find("layer"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                sprite.layer = *parsed;
            }
        }
        if (auto it = secIt->second.find("alpha"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                sprite.alpha = *parsed;
            }
        }
        if (auto it = secIt->second.find("flipx"); it != secIt->second.end()) {
            if (auto parsed = parseBool(it->second)) {
                sprite.flipX = *parsed;
            }
        }
        if (auto it = secIt->second.find("flipy"); it != secIt->second.end()) {
            if (auto parsed = parseBool(it->second)) {
                sprite.flipY = *parsed;
            }
        }
        role.sprite = sprite;
    }

    if (auto secIt = doc.sections.find("physics"); secIt != doc.sections.end()) {
        Physics physics;
        if (auto it = secIt->second.find("mass"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                physics.mass = *parsed;
            }
        }
        if (auto it = secIt->second.find("gravityscale"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                physics.gravityScale = *parsed;
            }
        }
        if (auto it = secIt->second.find("drag"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                physics.drag = *parsed;
            }
        }
        if (auto it = secIt->second.find("kinematic"); it != secIt->second.end()) {
            if (auto parsed = parseBool(it->second)) {
                physics.kinematic = *parsed;
            }
        }
        role.physics = physics;
    }

    if (auto secIt = doc.sections.find("collider"); secIt != doc.sections.end()) {
        Collider collider;
        if (auto it = secIt->second.find("x"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                collider.x = *parsed;
            }
        }
        if (auto it = secIt->second.find("y"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                collider.y = *parsed;
            }
        }
        if (auto it = secIt->second.find("w"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                collider.w = *parsed;
            }
        }
        if (auto it = secIt->second.find("h"); it != secIt->second.end()) {
            if (auto parsed = parseFloat(it->second)) {
                collider.h = *parsed;
            }
        }
        if (auto it = secIt->second.find("trigger"); it != secIt->second.end()) {
            if (auto parsed = parseBool(it->second)) {
                collider.trigger = *parsed;
            }
        }
        role.collider = collider;
    }

    if (auto secIt = doc.sections.find("controls"); secIt != doc.sections.end()) {
        Controls controls;
        if (auto it = secIt->second.find("left"); it != secIt->second.end()) {
            if (auto parsed = parseInt(it->second)) {
                controls.left = *parsed;
            }
        }
        if (auto it = secIt->second.find("right"); it != secIt->second.end()) {
            if (auto parsed = parseInt(it->second)) {
                controls.right = *parsed;
            }
        }
        if (auto it = secIt->second.find("up"); it != secIt->second.end()) {
            if (auto parsed = parseInt(it->second)) {
                controls.up = *parsed;
            }
        }
        if (auto it = secIt->second.find("down"); it != secIt->second.end()) {
            if (auto parsed = parseInt(it->second)) {
                controls.down = *parsed;
            }
        }
        if (auto it = secIt->second.find("action"); it != secIt->second.end()) {
            if (auto parsed = parseInt(it->second)) {
                controls.action = *parsed;
            }
        }
        if (auto it = secIt->second.find("jump"); it != secIt->second.end()) {
            if (auto parsed = parseInt(it->second)) {
                controls.jump = *parsed;
            }
        }
        role.controls = controls;
    }

    if (auto secIt = doc.sections.find("script"); secIt != doc.sections.end()) {
        Script script;
        if (auto it = secIt->second.find("binding"); it != secIt->second.end()) {
            script.binding = stripQuotes(it->second);
        }
        role.script = script;
    }

    return role;
}

Skin Vault::buildSkinFromDocument(const Document& doc, const std::filesystem::path& source) {
    Skin skin;
    if (auto it = doc.root.find("name"); it != doc.root.end()) {
        skin.name = stripQuotes(it->second);
    }
    else {
        skin.name = source.stem().string();
    }

    if (auto secIt = doc.sections.find("images"); secIt != doc.sections.end()) {
        for (const auto& [key, value] : secIt->second) {
            skin.imageOverrides[key] = stripQuotes(value);
        }
    }

    if (auto secIt = doc.sections.find("sounds"); secIt != doc.sections.end()) {
        for (const auto& [key, value] : secIt->second) {
            skin.soundOverrides[key] = stripQuotes(value);
        }
    }

    if (auto secIt = doc.sections.find("animations"); secIt != doc.sections.end()) {
        for (const auto& [key, value] : secIt->second) {
            skin.animationOverrides[key] = stripQuotes(value);
        }
    }

    return skin;
}

template<typename PayloadT>
bool Vault::releaseCacheEntry(ResourceCache<PayloadT>& cache, ResId id) {
    if (auto* entry = getEntry(GetKind(id), cache, id); entry) {
        if (entry->refCount > 0) {
            --entry->refCount;
        }
        if (entry->refCount == 0) {
            entry->active = false;
            entry->loaded = false;
            return true;
        }
        return false;
    }
    return false;
}

template<typename PayloadT>
std::uint32_t Vault::refCountFor(const ResourceCache<PayloadT>& cache, ResId id) const {
    if (auto* entry = getEntry(GetKind(id), cache, id); entry) {
        return entry->refCount;
    }
    return 0;
}

} // namespace sage2d
