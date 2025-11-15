#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace SAGE::Compression {

struct ZlibDiagnostics {
    bool syntheticBackrefs = false;
    uint32_t syntheticBackrefCount = 0;
};

std::vector<uint8_t> DecompressDeflate(const uint8_t* data,
                                       std::size_t size,
                                       bool parseZlibHeader,
                                       std::size_t expectedOutputSize = 0,
                                       ZlibDiagnostics* diagnostics = nullptr);

inline std::vector<uint8_t> DecompressZlib(const uint8_t* data,
                                           std::size_t size,
                                           std::size_t expectedOutputSize = 0,
                                           ZlibDiagnostics* diagnostics = nullptr) {
    return DecompressDeflate(data, size, true, expectedOutputSize, diagnostics);
}

} // namespace SAGE::Compression
