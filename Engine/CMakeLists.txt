# ===== PUBLIC API ADAPTERS =====
set(SAGE_ADAPTER_SOURCES
    src/Adapters/EngineImpl.cpp
    src/Adapters/SceneImpl.cpp
    src/Adapters/EditorAPIImpl.cpp
)

set(SAGE_CORE_SOURCES
    Core/Application.cpp
    Core/Compression/ZlibDecompressor.cpp
    Core/EventBus.cpp
    # Core/GameObject.cpp  # DISABLED: SceneID errors
    # Core/GameUtility.cpp  # DISABLED: depends on GameObject
    Core/Logger.cpp
    Core/PlatformUTF8.cpp
    Core/Profiler.cpp
    Core/ResourceManager.cpp
    Core/SceneManager.cpp
    # Core/SceneStack.cpp  # DISABLED: depends on GameObject
    Core/ServiceLocator.cpp
    Core/Window.cpp
    Core/UTF8Utils.cpp
    Core/EngineConfig.cpp
    Core/Plugin/PluginManager.cpp
    # Core/SaveSystem.cpp  # DISABLED: missing ThirdParty/json/json.hpp
    # Core/AchievementSystem.cpp  # DISABLED: JSON errors
    # Core/ProgressionSystem.cpp  # DISABLED: JSON errors
    # Core/TweenSystem.cpp  # DISABLED: missing glm
    # Core/ProceduralGeneration.cpp  # DISABLED: missing glm
    # Core/LocalizationSystem.cpp  # DISABLED: JSON errors
)
set(SAGE_ECS_SOURCES
    # ECS components - lightweight and optimized
    ECS/Components/Visual/AnimationComponent.cpp
    ECS/Components/Physics/ColliderComponent.cpp
    ECS/Components/Visual/TilemapComponent.cpp
)

set(SAGE_PHYSICS_SOURCES
    # Physics/PhysicsWorld.cpp - REMOVED: Old custom physics (2600+ lines)
    # Physics/PhysicsWorldBox2D.cpp - REMOVED: Replaced by Box2DBackend
    # Box2D integration files added conditionally below after box2d target check
)


set(SAGE_GRAPHICS_SOURCES
    Graphics/Core/Animation/AnimationClip.cpp
    Graphics/Core/Animation/SpriteAnimator.h
    # Graphics/Core/Rendering/TilemapRenderer.cpp  # DISABLED: missing fields
    Graphics/API/Renderer.cpp
    Graphics/API/RenderSystem.cpp
    Graphics/API/RenderSystemRegistry.cpp
    Graphics/API/RenderContextFactory.cpp
    Graphics/Backend/Implementations/OpenGL/OpenGLAdapters.cpp
    Graphics/Backend/Implementations/OpenGL/OpenGLRenderBackend.cpp
    Graphics/Backend/Implementations/OpenGL/OpenGLSceneRenderer.cpp
    Graphics/Core/Resources/Font.cpp
    Graphics/Core/Resources/Material.cpp
    Graphics/Core/Resources/Shader.cpp
    # Graphics/Core/Resources/Sprite.cpp  # DISABLED: SAGE_WARN not found
    Graphics/Core/Resources/SpriteBatchSoA.cpp
    Graphics/Core/Resources/Spritesheet.cpp
    Graphics/Core/Resources/Texture.cpp
    Graphics/Core/RenderContext.cpp
    Graphics/Core/ViewportManager.cpp
    Graphics/Backend/Implementations/OpenGL/Utils/GLDebug.cpp
    Graphics/Backend/Implementations/OpenGL/Utils/GLStateTracker.cpp
    Graphics/Core/Utils/PNGLoader.cpp
    Graphics/Core/Utils/stb_image_impl.cpp
    Graphics/GraphicsResourceManager.cpp
    Graphics/Rendering/Batching/BatchRenderer.cpp
    Graphics/Rendering/Batching/CommandBuffer.cpp
    Graphics/Rendering/Effects/Particles/ParticleSystem.cpp
    Graphics/Rendering/StateManagement/BlendStateController.cpp
    Graphics/Rendering/StateManagement/DepthStateController.cpp
    Graphics/Rendering/StateManagement/RenderStateManager.cpp
    Graphics/Rendering/StateManagement/StateStackManager.cpp
    Graphics/Rendering/RenderTarget.cpp
    Graphics/FrustumCuller2D.cpp
    # Graphics/NineSlice.cpp  # DISABLED: missing Graphics/Texture.h
    # Graphics/DayNightCycle.cpp  # DISABLED: missing glm
    Graphics/ShaderManager.cpp
    Graphics/PostProcessing/PostProcessManager.cpp
)

set(SAGE_MATH_SOURCES
    Math/Matrix4.cpp
    Math/Vector2.cpp
    Math/Random.cpp
)

set(SAGE_INPUT_SOURCES
    Input/InputAction.cpp
    Input/InputMap.cpp
    Input/ActionContext.cpp
    Input/InputBuffer.cpp
    # Input/InputConfig.cpp           # DISABLED: requires downloaded nlohmann/json header (offline)
    Input/InputBridge.cpp
    Input/InputManager.cpp
    Input/IWindow.cpp
)

set(SAGE_DEBUG_SOURCES
    # Debug/ImGuiLayer.cpp              # DISABLED: uses ImGui docking/viewport features
    # Debug/EntityInspector.cpp         # DISABLED: missing Components.h
    Debug/Profiler.cpp
    Debug/Console.cpp
    # Debug/SceneHierarchy.cpp          # DISABLED: uses EntityManager
    # Debug/TilemapEditor.cpp           # DISABLED: compilation errors
)

set(SAGE_AUDIO_SOURCES
    Audio/AudioSystem.cpp
    Audio/MusicSystem.cpp
)

set(SAGE_RESOURCE_SOURCES
    # Resources/AnimationAtlas.cpp          # DISABLED: missing json/json.hpp
    # Resources/TilemapLoader.cpp  # DISABLED: missing fields
    # Resources/STMLoader.cpp  # DISABLED: missing fields
    # Resources/TextureManager.cpp          # Removed: legacy system replaced by ResourceManager
)

set(SAGE_UI_SOURCES
    UI/Widget.cpp
    UI/Button.cpp
    UI/Label.cpp
    UI/Panel.cpp
    UI/TextInput.cpp
    UI/UIManager.cpp
    UI/FontManager.cpp
    UI/Widgets.cpp                   # ENABLED: Slider, Checkbox, Dropdown (fixed MouseEvent.Handled)
    # UI/QuestLogWidget.cpp          # DISABLED: compilation errors
    # UI/LayoutContainer.cpp         # DISABLED: compilation errors (m_Padding)
    # UI/UIScalingManager.cpp        # DISABLED: compilation errors
)

set(SAGE_SCRIPTING_SOURCES
    Scripting/LogCon/Core/Lexer.cpp
    Scripting/LogCon/Core/Parser.cpp
    Scripting/LogCon/Core/TokenID.cpp
    Scripting/LogCon/Languages/LanguageDefinition.cpp
    Scripting/LogCon/Languages/EnglishLanguage.cpp
    Scripting/LogCon/Languages/RussianLanguage.cpp
    # Scripting/LogCon/Languages/AdditionalLanguages.cpp  # Disabled: outdated TokenID enums
    # Scripting/LogCon/Runtime/Interpreter.cpp  # DISABLED: depends on GameObject
    Scripting/LogCon/Runtime/RuntimeValue.cpp
    Scripting/LogCon/Runtime/FunctionRegistry.cpp
    Scripting/LogCon/Runtime/BuiltinFunctions.cpp
    Scripting/LogCon/ScriptCompiler.cpp
    # Scripting/LogCon/LogConRuntime.cpp  # TODO: requires working Interpreter + API fixes
    # Scripting/Lua/Bindings/LuaBindings.cpp  # DISABLED: missing sol/sol.hpp
    # Scripting/Lua/Bindings/EntityBindings.cpp  # DISABLED: missing sol/sol.hpp
    # Scripting/Lua/Bindings/InputBindings.cpp  # DISABLED: missing sol/sol.hpp
)

set(SAGE_MODDING_SOURCES
    # Modding/ModInfo.cpp  # DISABLED: JSON errors
    # Modding/ModManager.cpp           # Disabled: depends on ModInfo/JSON
    # Modding/ModScriptBindings.cpp    # Disabled: depends on ModManager
    Modding/ModScriptBindingsStub.cpp
)

# DISABLED: compilation errors (nlohmann/json, EventBus::Publish)
# set(SAGE_QUEST_SOURCES
#     Quests/QuestManager.cpp
# )

# DISABLED: compilation errors (missing includes, function signatures)
# set(SAGE_AI_SOURCES
#     AI/SteeringBehaviors.cpp
#     AI/Pathfinder.cpp
#     AI/Perception.cpp
# )

file(GLOB_RECURSE SAGE_ENGINE_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ECS/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Memory/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripting/**/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Modding/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SAGE.h
)

# Public API headers (visible to external users)
file(GLOB_RECURSE SAGE_PUBLIC_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SAGE/*.h
)

set(SAGE_ENGINE_SOURCES
    ${SAGE_ADAPTER_SOURCES}
    ${SAGE_CORE_SOURCES}
    ${SAGE_ECS_SOURCES}
    ${SAGE_PHYSICS_SOURCES}
    ${SAGE_GRAPHICS_SOURCES}
    ${SAGE_MATH_SOURCES}
    ${SAGE_INPUT_SOURCES}
    ${SAGE_DEBUG_SOURCES}
    ${SAGE_AUDIO_SOURCES}
    ${SAGE_RESOURCE_SOURCES}
    ${SAGE_UI_SOURCES}
    ${SAGE_SCRIPTING_SOURCES}
    ${SAGE_MODDING_SOURCES}
    # ${SAGE_QUEST_SOURCES}    # DISABLED: compilation errors
    # ${SAGE_AI_SOURCES}       # DISABLED: compilation errors
)

add_library(SAGE_Engine STATIC ${SAGE_ENGINE_SOURCES} ${SAGE_ENGINE_HEADERS} ${SAGE_PUBLIC_HEADERS})

# Force dynamic runtime library for MSVC
if(MSVC)
    # Remove any existing runtime library flags and add /MDd for Debug, /MD for Release
    target_compile_options(SAGE_Engine PRIVATE
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:RelWithDebInfo>:/MD>
        $<$<CONFIG:MinSizeRel>:/MD>
    )
endif()

# ===== CRITICAL: Public API visibility configuration =====
# Use generator expressions for build vs install include directories
target_include_directories(SAGE_Engine
    PUBLIC
        # External users (Editor, Examples) see ONLY include/
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/ThirdParty>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/ThirdParty/sol2>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        # Internal implementation sees everything
    SYSTEM PRIVATE
        ${CMAKE_SOURCE_DIR}/ThirdParty/miniaudio
)

target_link_libraries(SAGE_Engine
    PUBLIC
        glad
        glfw
        sage_thirdparty_headers
        nlohmann_json
        tinyxml2
)

# Link Box2D if available
# Box2D v3.x C API integration
if(TARGET box2d)
    target_sources(SAGE_Engine PRIVATE
        Physics/Box2DBackend.cpp
    )
    target_link_libraries(SAGE_Engine PUBLIC box2d)
    target_compile_definitions(SAGE_Engine PUBLIC SAGE_HAS_BOX2D)
    target_include_directories(SAGE_Engine PRIVATE ${CMAKE_SOURCE_DIR}/ThirdParty/box2d/include)
    message(STATUS "Box2D v3.x C API linked to SAGE_Engine")
endif()

# Link ImGui if available
if(TARGET imgui)
    target_link_libraries(SAGE_Engine PUBLIC imgui)
    target_compile_definitions(SAGE_Engine PRIVATE SAGE_HAS_IMGUI)
endif()

target_compile_definitions(SAGE_Engine
    PUBLIC GLFW_INCLUDE_NONE
    PRIVATE $<$<CONFIG:Debug>:SAGE_GL_DEBUG=1>
)

target_compile_features(SAGE_Engine PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(SAGE_Engine PRIVATE /W4 /permissive- /FS /wd4005)
else()
    target_compile_options(SAGE_Engine PRIVATE -Wall -Wextra -pedantic)
endif()

# ========================================
# INSTALLATION CONFIGURATION
# ========================================

include(GNUInstallDirs)

# Install library (also install third-party dependencies)
# Note: imgui is conditionally added if available
set(SAGE_INSTALL_TARGETS SAGE_Engine glad glfw sage_thirdparty_headers)
if(TARGET imgui)
    list(APPEND SAGE_INSTALL_TARGETS imgui)
endif()

install(TARGETS ${SAGE_INSTALL_TARGETS}
    EXPORT SAGETargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers - preserve directory structure
install(DIRECTORY 
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/ECS
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/Math
    ${CMAKE_CURRENT_SOURCE_DIR}/Memory
    ${CMAKE_CURRENT_SOURCE_DIR}/Input
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripting
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SAGE
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
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
