#pragma once

#include "TokenID.h"
#include <string>

namespace SAGE::Scripting::LogCon {

struct Token {
    TokenID id = TokenID::INVALID;
    std::string lexeme;
    int line = 0;
    int column = 0;

    double numberValue = 0.0;
    bool boolValue = false;
    std::string stringValue;
};

} // namespace SAGE::Scripting::LogCon
