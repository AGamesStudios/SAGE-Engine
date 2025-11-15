#include "TestFramework.h"

#include "logcon/vm.hpp"

#include <cstdint>

TEST_CASE(LogCon_VM_LoadAssignsHandle) {
    logcon::VirtualMachine vm;
    logcon::BytecodeChunk chunk;
    chunk.code.push_back(static_cast<std::uint8_t>(logcon::OpCode::OP_NOOP));

    const auto handle = vm.load(chunk);
    CHECK(handle != 0);
}
