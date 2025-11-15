cmake_minimum_required(VERSION 3.20)

# Force dynamic runtime library for all targets (MSVC)
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

project(SAGE_Engine VERSION 0.1.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# MSVC settings
if(MSVC)
    # Set global policy to use MultiThreadedDLL runtime for all targets
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    # Suppress macro redefinition warning (APIENTRY) from Windows/minwindef vs glad
    add_compile_options(/wd4005)
endif()

# Options
option(SAGE_BUILD_TESTS "Build SAGE tests" ON)
option(SAGE_BUILD_EXAMPLES "Build SAGE examples" ON)
option(SAGE_ENABLE_PROFILING "Enable profiling support" OFF)
option(SAGE_STRICT_WARNINGS "Treat warnings as errors" OFF)
set(SAGE_PHYSICS_BACKEND "PRIMITIVE" CACHE STRING "Physics backend: PRIMITIVE or BOX2D")
set_property(CACHE SAGE_PHYSICS_BACKEND PROPERTY STRINGS PRIMITIVE BOX2D)
if(NOT SAGE_PHYSICS_BACKEND IN_LIST CMAKE_CACHE_STRINGS)
    # Basic validation
    if(NOT SAGE_PHYSICS_BACKEND MATCHES "^(PRIMITIVE|BOX2D)$")
        message(FATAL_ERROR "Invalid SAGE_PHYSICS_BACKEND='${SAGE_PHYSICS_BACKEND}'. Use PRIMITIVE or BOX2D")
    endif()
endif()

if(SAGE_PHYSICS_BACKEND STREQUAL "BOX2D")
    add_compile_definitions(SAGE_USE_BOX2D)
    # Mirror compile definition in a variable for generator expressions
    set(SAGE_USE_BOX2D ON)
    message(STATUS "Using Box2D physics backend")
else()
    set(SAGE_USE_BOX2D OFF)
    message(STATUS "Using primitive physics backend")
endif()

# Third-party dependencies
add_subdirectory(ThirdParty)

if(SAGE_TINYXML2_AVAILABLE)
    add_compile_definitions(SAGE_HAS_TINYXML2)
    message(STATUS "TinyXML2 enabled for TMX tilemap loading")
else()
    message(WARNING "TinyXML2 not available; TMX tilemap loading disabled")
endif()

# Engine library
add_subdirectory(Engine)

# Tests
if(SAGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

# Editor
add_subdirectory(Editor)

# Examples
if(SAGE_BUILD_EXAMPLES)
    message(STATUS "Building examples: TextureLoadingExample")
    add_executable(TextureLoadingExample Examples/TextureLoadingExample.cpp)
    target_link_libraries(TextureLoadingExample PRIVATE SAGE_Engine)
    set_target_properties(TextureLoadingExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin FOLDER "Examples" MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    add_executable(TextureRenderExample Examples/TextureRenderExample.cpp)
    target_link_libraries(TextureRenderExample PRIVATE SAGE_Engine)
    set_target_properties(TextureRenderExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin FOLDER "Examples" MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    add_executable(BatchTextureExample Examples/BatchTextureExample.cpp)
    target_link_libraries(BatchTextureExample PRIVATE SAGE_Engine)
    set_target_properties(BatchTextureExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin FOLDER "Examples" MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    add_executable(CharacterSheetDemo Examples/CharacterSheetDemo.cpp)
    target_link_libraries(CharacterSheetDemo PRIVATE SAGE_Engine)
    set_target_properties(CharacterSheetDemo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin FOLDER "Examples" MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Tools (SpriteSheetAnalyzer)
add_executable(SpriteSheetAnalyzer tools/SpriteSheetAnalyzer.cpp)
target_link_libraries(SpriteSheetAnalyzer PRIVATE SAGE_Engine)
if(MSVC)
    set_target_properties(SpriteSheetAnalyzer PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
set_target_properties(SpriteSheetAnalyzer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin FOLDER "Tools")

# Apply strict warnings globally (after targets exist) by iterating over targets if enabled
if(SAGE_STRICT_WARNINGS)
    get_property(_sage_targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_t ${_sage_targets})
        if(TARGET ${_t})
            if(MSVC)
                target_compile_options(${_t} PRIVATE /WX)
            else()
                target_compile_options(${_t} PRIVATE -Werror)
            endif()
        endif()
    endforeach()
    message(STATUS "SAGE_STRICT_WARNINGS enabled: treating all warnings as errors")
endif()

# Installation
include(GNUInstallDirs)

install(EXPORT SAGETargets
    FILE SAGETargets.cmake
    NAMESPACE SAGE::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SAGE
)
