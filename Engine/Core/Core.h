#pragma once

#include "../Memory/Ref.h"

// Макросы для движка
#define SAGE_ASSERT(x, msg) if(!(x)) { SAGE_ERROR("Assertion Failed: {0}", msg); __debugbreak(); }
