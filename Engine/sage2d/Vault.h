#pragma once

#include "ResourceId.h"
#include "Role.h"
#include "Skin.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sage2d {

    class Vault {
    public:
        struct ImageResource {
            std::filesystem::path source;
            bool exists{ false };
        };

        struct SoundResource {
            std::filesystem::path source;
            bool exists{ false };
        };

        struct AnimationResource {
            std::filesystem::path source;
            bool exists{ false };
        };

        Vault();

        [[nodiscard]] ResId image(const std::filesystem::path& path);
        [[nodiscard]] ResId sound(const std::filesystem::path& path);
        [[nodiscard]] ResId animation(const std::filesystem::path& path);

        [[nodiscard]] ResId roleFromFile(const std::filesystem::path& path);
        [[nodiscard]] ResId registerRole(const std::string& name, Role role);
        [[nodiscard]] ResId skinFromFile(const std::filesystem::path& path);
        [[nodiscard]] ResId registerSkin(const std::string& name, Skin skin);

        void retain(ResId id);
        void release(ResId id);

        [[nodiscard]] std::uint32_t refCount(ResId id) const;

        [[nodiscard]] const ImageResource* getImage(ResId id) const;
        [[nodiscard]] const SoundResource* getSound(ResId id) const;
        [[nodiscard]] const AnimationResource* getAnimation(ResId id) const;
        [[nodiscard]] const Role* getRole(ResId id) const;
        [[nodiscard]] const Role* getRole(const std::string& name) const;
        [[nodiscard]] const Skin* getSkin(ResId id) const;
        [[nodiscard]] const Skin* getSkin(const std::string& name) const;

        void clear();

    private:
        template<typename PayloadT>
        struct ResourceEntry {
            std::filesystem::path source;
            PayloadT payload{};
            std::uint32_t refCount{ 0 };
            bool active{ false };
            bool loaded{ false };
        };

        template<typename PayloadT>
        struct ResourceCache {
            std::vector<ResourceEntry<PayloadT>> entries;
            std::unordered_map<std::string, std::uint32_t> keyLookup;
        };

        using ImageCache = ResourceCache<ImageResource>;
        using SoundCache = ResourceCache<SoundResource>;
        using AnimationCache = ResourceCache<AnimationResource>;
        using RoleCache = ResourceCache<Role>;
        using SkinCache = ResourceCache<Skin>;

        struct Document;

        ImageCache m_Images;
        SoundCache m_Sounds;
        AnimationCache m_Animations;
        RoleCache m_Roles;
        SkinCache m_Skins;

        std::unordered_map<std::string, ResId> m_RolesByName;
        std::unordered_map<std::string, ResId> m_SkinsByName;

        std::string canonicalKey(const std::filesystem::path& path) const;

        template<typename PayloadT>
        ResId acquireFromPath(ResourceKind kind, ResourceCache<PayloadT>& cache, const std::filesystem::path& path);

        template<typename PayloadT>
        ResourceEntry<PayloadT>* getEntry(ResourceKind kind, ResourceCache<PayloadT>& cache, ResId id);

        template<typename PayloadT>
        const ResourceEntry<PayloadT>* getEntry(ResourceKind kind, const ResourceCache<PayloadT>& cache, ResId id) const;

    void ensureLoaded(ResourceEntry<ImageResource>& entry) const;
    void ensureLoaded(ResourceEntry<SoundResource>& entry) const;
    void ensureLoaded(ResourceEntry<AnimationResource>& entry) const;
    void ensureLoaded(ResourceEntry<Role>& entry, ResId id);
    void ensureLoaded(ResourceEntry<Skin>& entry, ResId id);

    static Document parseDocument(const std::filesystem::path& path);
    Role buildRoleFromDocument(const Document& doc, const std::filesystem::path& source);
    Skin buildSkinFromDocument(const Document& doc, const std::filesystem::path& source);

    template<typename PayloadT>
    bool releaseCacheEntry(ResourceCache<PayloadT>& cache, ResId id);

        template<typename PayloadT>
        [[nodiscard]] std::uint32_t refCountFor(const ResourceCache<PayloadT>& cache, ResId id) const;
    };

} // namespace sage2d
