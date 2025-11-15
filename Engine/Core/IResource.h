#pragma once

#include <cstddef>
#include <string>
#include <ostream>

namespace SAGE {

enum class ResourceState {
    Loaded,
    Unloaded,
    Stub,
    Failed
};

inline std::ostream& operator<<(std::ostream& os, ResourceState state) {
    switch(state) {
    case ResourceState::Loaded:   os << "Loaded"; break;
    case ResourceState::Unloaded: os << "Unloaded"; break;
    case ResourceState::Stub:     os << "Stub"; break;
    case ResourceState::Failed:   os << "Failed"; break;
    }
    return os;
}

class IResource {
public:
    virtual ~IResource() noexcept = default;

    virtual std::size_t GetGPUMemorySize() const = 0;
    virtual const std::string& GetPath() const = 0;
    virtual bool Unload() noexcept = 0; // Возвращает true при успехе
    virtual bool Reload() = 0; // Возвращает true при успехе
    virtual bool IsLoaded() const = 0;
    virtual ResourceState GetState() const = 0;
};

} // namespace SAGE
