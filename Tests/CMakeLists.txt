cmake_minimum_required(VERSION 3.23)

# ============================================
# SAGE Engine Tests
# ============================================

project(SAGE_Tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Test Framework: Catch2 (header-only, single-file)
add_library(Catch2 INTERFACE)
target_include_directories(Catch2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Test executable
set(TEST_SOURCES
    TestMain.cpp
    MathTests.cpp
    ResourceManagerTests.cpp
    SceneSerializerTests.cpp
    Camera2DTests.cpp
    PluginManagerTests.cpp
    ProfilerTests.cpp
    QuadTreeTests.cpp
    ParticleEmitterTests.cpp
    ShaderTests.cpp
    IntegrationTests.cpp
    RendererTests.cpp
    PerformanceBenchmarks.cpp
    ECSTests.cpp
    ECSSystemsTests.cpp
    TextTests.cpp
    PhysicsTests.cpp
)

add_executable(SAGE_Tests ${TEST_SOURCES})

# MSVC: Enable /FS to avoid PDB file access conflicts during parallel compilation
if(MSVC)
    target_compile_options(SAGE_Tests PRIVATE /FS)
endif()

target_include_directories(SAGE_Tests PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/ThirdParty
    ${CMAKE_SOURCE_DIR}/ThirdParty/glfw/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/glad/include
)

target_link_libraries(SAGE_Tests PRIVATE
    SAGE_Engine
    Catch2
)

# Enable testing
enable_testing()
add_test(NAME SAGE_UnitTests COMMAND SAGE_Tests)

# Copy test to bin directory
set_target_properties(SAGE_Tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
)
