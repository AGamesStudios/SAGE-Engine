#pragma once

#include "UTF8String.h"

#include <vector>

namespace SAGE::Platform {

class UTF8Support {
public:
    using Utf8String = SAGE::Core::utf8::String;

    static bool Initialize();
    static bool IsInitialized();

    static bool CreateDirectory(const Utf8String& path);
    static bool FileExists(const Utf8String& path);
    static std::vector<Utf8String> ListDirectory(const Utf8String& path);

    static Utf8String GetCurrentDirectory();
    static Utf8String NormalizePath(const Utf8String& path);

private:
    static bool InitializeInternal();

    static bool s_initialized;
};

} // namespace SAGE::Platform
