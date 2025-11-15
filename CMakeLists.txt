cmake_minimum_required(VERSION 3.15)
project(SAGEEngine VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# UTF-8 support for MSVC - автодетекцию включаем для всех файлов с кириллицей
if(MSVC)
    # Принудительная компиляция с UTF-8 (включает source и execution charset)
    add_compile_options(/utf-8)
    # Отключаем предупреждения о non-ASCII символах
    add_compile_options(/wd4819)
    add_compile_options(/W3)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Multi-config generators (Visual Studio, Xcode)
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

# Options
option(SAGE_BUILD_EXAMPLES "Build SAGE Engine examples" ON)
option(SAGE_BUILD_EDITOR "Build SAGE Engine editor" ON)
option(SAGE_BUILD_TESTS "Build SAGE Engine tests" ON)
option(SAGE_BUILD_PLUGINS "Build SAGE Engine plugins" ON)
option(SAGE_ENABLE_LUA "Enable Lua scripting via sol2" OFF)

if(SAGE_ENABLE_LUA)
    add_compile_definitions(SAGE_ENABLE_LUA=1)
else()
    add_compile_definitions(SAGE_ENABLE_LUA=0)
endif()

# ThirdParty libraries
add_subdirectory(ThirdParty)

# Engine library
add_subdirectory(Engine)

# Examples
if(SAGE_BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

# # LogCon Launcher - DISABLED: tools/ removed
# add_executable(LogConLauncher
#     Tools/LogConLauncher/main.cpp
#     Engine/Scripting/LogCon/LogConRuntime.cpp
# )
# target_link_libraries(LogConLauncher PRIVATE SAGE_Engine)

# # Editor - DISABLED: missing RigidBodyComponent.h
# if(SAGE_BUILD_EDITOR)
#     add_subdirectory(Editor)
# endif()

# Tests - ENABLED for comprehensive testing
if(SAGE_BUILD_TESTS)
    add_subdirectory(Tests)
endif()

# Plugins
if(SAGE_BUILD_PLUGINS)
    add_subdirectory(Plugins)
endif()

# # SAGE CLI Tool - DISABLED: tools/ removed
# add_executable(sage_cli
#     tools/sage_cli_main.cpp
# )
# 
# target_compile_features(sage_cli PRIVATE cxx_std_17)
# 
# target_include_directories(sage_cli PRIVATE
#     ${CMAKE_SOURCE_DIR}/tools
# )
# 
# # No dependencies needed - header-only implementation
# # SAGE CLI Tool - DISABLED: tools/ removed
# set_target_properties(sage_cli PROPERTIES
#     OUTPUT_NAME "sage"
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )
# 
# # Copy to root bin/ directory for easy access
# add_custom_command(TARGET sage_cli POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
#     COMMAND ${CMAKE_COMMAND} -E copy
#         $<TARGET_FILE:sage_cli>
#         ${CMAKE_SOURCE_DIR}/bin/sage$<$<PLATFORM_ID:Windows>:.exe>
#     COMMENT "Installing SAGE CLI to bin/"
# )

# Installation
install(TARGETS SAGE_Engine
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# install(TARGETS sage_cli
#     RUNTIME DESTINATION bin
# )

# Copy assets
install(DIRECTORY assets/ DESTINATION bin/assets)
