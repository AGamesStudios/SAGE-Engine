#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace SAGE {

/**
 * @brief Fast string ID using FNV-1a hashing
 * 
 * Converts strings to 32-bit IDs for fast comparisons.
 * Perfect for tags, names, events where you compare strings frequently.
 * 
 * Usage:
 *   StringID id1("Player");
 *   StringID id2("Enemy");
 *   if (id1 == id2) { ... } // Fast integer comparison
 */
class StringID {
public:
    constexpr StringID() : m_Hash(0) {}
    
    constexpr StringID(uint32_t hash) : m_Hash(hash) {}
    
    explicit StringID(std::string_view str) : m_Hash(Hash(str)) {}
    
    explicit StringID(const std::string& str) : m_Hash(Hash(str)) {}
    
    explicit StringID(const char* str) : m_Hash(Hash(str)) {}
    
    constexpr uint32_t GetHash() const { return m_Hash; }
    
    constexpr bool IsValid() const { return m_Hash != 0; }
    
    constexpr operator uint32_t() const { return m_Hash; }
    
    constexpr bool operator==(const StringID& other) const {
        return m_Hash == other.m_Hash;
    }
    
    constexpr bool operator!=(const StringID& other) const {
        return m_Hash != other.m_Hash;
    }
    
    constexpr bool operator<(const StringID& other) const {
        return m_Hash < other.m_Hash;
    }
    
    /**
     * @brief FNV-1a hash algorithm (compile-time capable)
     */
    static constexpr uint32_t Hash(std::string_view str) {
        uint32_t hash = 0x811c9dc5u; // FNV offset basis
        for (char c : str) {
            hash ^= static_cast<uint8_t>(c);
            hash *= 0x01000193u; // FNV prime
        }
        return hash;
    }
    
private:
    uint32_t m_Hash;
};

/**
 * @brief Compile-time string literal to StringID
 * 
 * Usage:
 *   constexpr StringID playerId = "Player"_sid;
 *   if (objId == playerId) { ... }
 */
constexpr StringID operator""_sid(const char* str, size_t len) {
    return StringID(StringID::Hash(std::string_view(str, len)));
}

} // namespace SAGE

// Hash specialization for std::unordered_map
namespace std {
    template<>
    struct hash<SAGE::StringID> {
        size_t operator()(const SAGE::StringID& id) const noexcept {
            return static_cast<size_t>(id.GetHash());
        }
    };
}
