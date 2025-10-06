#pragma once

#include <vector>

namespace SAGE::EmbeddedFonts {

    // Возвращает декодированные данные встроенного шрифта ProggyClean (TTF).
    std::vector<unsigned char> GetProggyCleanTTF();

} // namespace SAGE::EmbeddedFonts
