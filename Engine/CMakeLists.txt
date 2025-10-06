set(SAGE_AUDIO_SOURCES
    Audio/AudioSystem.cpp
    Audio/Sound.cpp
)

set(SAGE_CORE_SOURCES
    Core/Application.cpp
    Core/EventBus.cpp
    Core/GameObject.cpp
    Core/Logger.cpp
    Core/Profiler.cpp
    Core/SceneManager.cpp
    Core/SceneStack.cpp
    Core/Window.cpp
)

set(SAGE_GRAPHICS_SOURCES
    Graphics/Font.cpp
    # Graphics/GLDebug.cpp # TODO: GLAD include issues - disabled for alpha
    Graphics/Material.cpp
    Graphics/ParticleSystem.cpp
    Graphics/Renderer.cpp
    Graphics/Shader.cpp
    Graphics/ShaderManager.cpp
    Graphics/Sprite.cpp
    Graphics/Texture.cpp
    Graphics/stb_image_impl.cpp
)

set(SAGE_INPUT_SOURCES
    Input/Input.cpp
    Input/InputBindings.cpp
)

set(SAGE_MATH_SOURCES
    Math/Vector2.cpp
)

set(SAGE_PHYSICS_SOURCES
    Physics/PhysicsSystem.cpp
)

set(SAGE_RESOURCES_SOURCES
    Resources/AssetManager.cpp
    Resources/EmbeddedFonts.cpp
    Resources/FontManager.cpp
    Resources/ResourceRegistry.cpp
    Resources/SoundManager.cpp
    Resources/TextureManager.cpp
)

set(SAGE_UI_SOURCES
    UI/UISystem.cpp
)

set(SAGE2D_SOURCES
    sage2d/EventBus.cpp
    sage2d/Stage.cpp
    sage2d/StageManager.cpp
    sage2d/Vault.cpp
)

file(GLOB_RECURSE SAGE_ENGINE_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Audio/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Input/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Memory/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sage2d/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UI/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SAGE.h
)

set(SAGE_ENGINE_SOURCES
    ${SAGE_AUDIO_SOURCES}
    ${SAGE_CORE_SOURCES}
    ${SAGE_GRAPHICS_SOURCES}
    ${SAGE_INPUT_SOURCES}
    ${SAGE_MATH_SOURCES}
    ${SAGE_PHYSICS_SOURCES}
    ${SAGE_RESOURCES_SOURCES}
    ${SAGE_UI_SOURCES}
    ${SAGE2D_SOURCES}
)

add_library(SAGE_Engine STATIC ${SAGE_ENGINE_SOURCES} ${SAGE_ENGINE_HEADERS})

# No MSVC-specific runtime setting here - controlled by root CMakeLists.txt via CMAKE_MSVC_RUNTIME_LIBRARY

# Use generator expressions for build vs install include directories
target_include_directories(SAGE_Engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(SAGE_Engine
    PUBLIC
        glad
        glfw
        sage_thirdparty_headers
)

target_compile_definitions(SAGE_Engine
    PUBLIC GLFW_INCLUDE_NONE
)

target_compile_features(SAGE_Engine PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(SAGE_Engine PRIVATE /W4 /permissive-)
else()
    target_compile_options(SAGE_Engine PRIVATE -Wall -Wextra -pedantic)
endif()

# ========================================
# INSTALLATION CONFIGURATION
# ========================================

include(GNUInstallDirs)

# Install library (also install third-party dependencies)
install(TARGETS SAGE_Engine glad glfw sage_thirdparty_headers
    EXPORT SAGETargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers - preserve directory structure
install(DIRECTORY 
    ${CMAKE_CURRENT_SOURCE_DIR}/Audio
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/Input
    ${CMAKE_CURRENT_SOURCE_DIR}/Math
    ${CMAKE_CURRENT_SOURCE_DIR}/Memory
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources
    ${CMAKE_CURRENT_SOURCE_DIR}/sage2d
    ${CMAKE_CURRENT_SOURCE_DIR}/UI
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SAGE
    FILES_MATCHING PATTERN "*.h"
)

# Install main header
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/SAGE.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install third-party headers
install(DIRECTORY ${CMAKE_BINARY_DIR}/thirdparty/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SAGE/ThirdParty
    FILES_MATCHING PATTERN "*.h"
)

# Install GLAD headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/ThirdParty/glad/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Generate and install export file
install(EXPORT SAGETargets
    FILE SAGETargets.cmake
    NAMESPACE SAGE::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SAGE
)

# Create package config files
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SAGEConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SAGEConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SAGE
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/SAGEConfigVersion.cmake
    VERSION 0.1.0
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/SAGEConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/SAGEConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SAGE
)
