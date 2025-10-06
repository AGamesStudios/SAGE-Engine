#pragma once

#include <cstdint>

namespace sage2d {

    using ResId = std::uint32_t;

    constexpr ResId kInvalidResId = 0;

    enum class ResourceKind : std::uint8_t {
        None = 0,
        Image,
        Animation,
        Sound,
        Role,
        Skin
    };

    constexpr ResId MakeResId(ResourceKind kind, std::uint32_t index) {
        return static_cast<ResId>((static_cast<std::uint32_t>(kind) << 24) | (index & 0x00FFFFFFu));
    }

    constexpr ResourceKind GetKind(ResId id) {
        return id == kInvalidResId ? ResourceKind::None : static_cast<ResourceKind>((id >> 24) & 0xFFu);
    }

    constexpr std::uint32_t GetIndex(ResId id) {
        return id & 0x00FFFFFFu;
    }

    constexpr bool IsValid(ResId id) {
        return id != kInvalidResId && GetKind(id) != ResourceKind::None;
    }

} // namespace sage2d
