cmake_minimum_required(VERSION 3.23)

# ============================================
# SAGE Engine Library
# ============================================

# Header Files (Public API)
set(SAGE_ENGINE_HEADERS
    # Core
    include/SAGE/SAGE.h
    include/SAGE/Logger.h
    include/SAGE/Log.h
    include/SAGE/Time.h
    include/SAGE/WindowConfig.h
    include/SAGE/Window.h
    include/SAGE/ApplicationConfig.h
    include/SAGE/Application.h
    include/SAGE/DevMode.h
    include/SAGE/Core/ECS.h
    include/SAGE/Core/ResourceManager.h
    include/SAGE/Core/Scene.h
    include/SAGE/Core/SceneManager.h
    include/SAGE/Core/Game.h
    include/SAGE/Core/Timer.h
    include/SAGE/Core/CommandLine.h
    include/SAGE/Core/EventBus.h
    include/SAGE/Core/AssetManager.h
    include/SAGE/Core/SaveSystem.h
    include/SAGE/Core/Profiler.h
    include/SAGE/Core/ECSComponents.h
    include/SAGE/Core/ECSSystems.h
    include/SAGE/Core/ECSGame.h
    include/SAGE/Core/TiledLevel.h
    include/SAGE/Core/GameObject.h
    include/SAGE/Core/SceneSerializer.h
    include/SAGE/Core/Prefab.h
    
    # Scripting
    include/SAGE/Scripting/ScriptableEntity.h
    include/SAGE/Scripting/ScriptEngine.h
    
    # Legacy ECS removed
    include/SAGE/Core/ECS.h
    include/SAGE/Plugin/IPlugin.h
    include/SAGE/Plugin/PluginManager.h
    
    # Math
    include/SAGE/Math/Vector2.h
    include/SAGE/Math/Color.h
    include/SAGE/Math/Matrix3.h
    include/SAGE/Math/Rect.h
    include/SAGE/Math/QuadTree.h
    
    # Input
    include/SAGE/Input/Input.h
    
    # Graphics
    include/SAGE/Graphics/Renderer.h
    include/SAGE/Graphics/RenderBackend.h
    include/SAGE/Graphics/Shader.h
    include/SAGE/Graphics/ShaderLibrary.h
    include/SAGE/Graphics/Texture.h
    include/SAGE/Graphics/Camera2D.h
    include/SAGE/Graphics/Sprite.h
    include/SAGE/Graphics/SpriteRenderer.h
    include/SAGE/Graphics/Animation.h
    include/SAGE/Graphics/Animator.h
    include/SAGE/Graphics/ParticleSystem.h
    include/SAGE/Graphics/ParticleEmitter.h
    include/SAGE/Graphics/Font.h
    include/SAGE/Graphics/Tilemap.h
    include/SAGE/Graphics/TMXLoader.h
    include/SAGE/Graphics/UVCoordinates.h
    include/SAGE/Graphics/Gizmo.h
    
    # Physics
    include/SAGE/Physics/PhysicsCommon.h
    include/SAGE/Physics/PhysicsWorld.h
    
    # Audio
    include/SAGE/Audio/Audio.h

    # UI
    include/SAGE/UI/UIContext.h
    include/SAGE/UI/Widget.h
    include/SAGE/UI/UIComponents.h
)

# Source Files (Implementation)
set(SAGE_ENGINE_SOURCES
    # Core
    src/Logger.cpp
    src/Time.cpp
    src/Window.cpp
    src/Application.cpp
    src/ResourceManager.cpp
    src/Core/AssetManager.cpp
    src/Core/Scene.cpp
    src/Core/SceneManager.cpp
    src/Core/CommandLine.cpp
    src/Core/Game.cpp
    src/Core/Timer.cpp
    src/Core/EventBus.cpp
    src/Core/SaveSystem.cpp
    src/Core/Profiler.cpp
    src/Core/ECSSystems.cpp
    src/Core/ECSGame.cpp
    src/Core/TiledLevel.cpp
    src/Core/SceneSerializer.cpp
    src/Core/Prefab.cpp
    src/PluginManager.cpp
    
    # Input
    src/Input.cpp
    
    # Graphics
    src/Renderer.cpp
    src/Graphics/OpenGL/OpenGLRenderBackend.cpp
    src/Shader.cpp
    src/ShaderLibrary.cpp
    src/Texture.cpp
    src/Camera2D.cpp
    src/Sprite.cpp
    src/SpriteRenderer.cpp
    src/Animation.cpp
    src/Graphics/Animator.cpp
    src/ParticleSystem.cpp
    src/Graphics/ParticleEmitter.cpp
    src/Graphics/Font.cpp
    src/Graphics/Tilemap.cpp
    src/Graphics/TMXLoader.cpp
    src/Graphics/UVCoordinates.cpp
    src/Gizmo.cpp
    
    # Physics
    src/Physics/PhysicsWorld.cpp
    
    # Audio
    src/Audio.cpp

    # UI
    src/UI/UIContext.cpp
    src/UI/Widget.cpp
    src/UI/UIComponents.cpp
)

# Create Static Library
add_library(SAGE_Engine STATIC
    ${SAGE_ENGINE_HEADERS}
    ${SAGE_ENGINE_SOURCES}
)

# Visual Studio Solution Filters
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SAGE_ENGINE_HEADERS}
    ${SAGE_ENGINE_SOURCES}
)

# Compiler Settings
set_target_properties(SAGE_Engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Include Directories
target_include_directories(SAGE_Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/ThirdParty
)

# Link Dependencies
target_link_libraries(SAGE_Engine
    PUBLIC
        sage_thirdparty
)

if(WIN32)
    target_link_libraries(SAGE_Engine PUBLIC shell32)
endif()

# Compiler Warnings
if(MSVC)
    target_compile_options(SAGE_Engine PRIVATE
        /W4
        /permissive-
        /Zc:preprocessor
    )
else()
    target_compile_options(SAGE_Engine PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
