cmake_minimum_required(VERSION 3.23)
project(SAGEEngine VERSION 0.1.0 LANGUAGES CXX C)

# ============================================
# SAGE Engine - 2D Game Engine
# Version: 0.1.0 Alpha
# ============================================

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler Warnings
if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Subdirectories
add_subdirectory(ThirdParty)
add_subdirectory(Engine)
add_subdirectory(Sandbox)
add_subdirectory(Tests)
add_subdirectory(Examples/PluginExample)

# Examples
add_executable(SceneDemo Examples/SceneDemo.cpp)
target_link_libraries(SceneDemo SAGE_Engine)

add_executable(TilemapDemo Examples/TilemapDemo.cpp)
target_link_libraries(TilemapDemo SAGE_Engine)

add_executable(ScriptingDemo Examples/ScriptingDemo.cpp)
target_link_libraries(ScriptingDemo SAGE_Engine)

add_executable(UIDemo Examples/UIDemo.cpp)
target_link_libraries(UIDemo SAGE_Engine)

add_executable(ShapesAndParticlesDemo Examples/ShapesAndParticlesDemo.cpp)
target_link_libraries(ShapesAndParticlesDemo SAGE_Engine)

add_executable(PathDemo Examples/PathDemo.cpp)
target_link_libraries(PathDemo SAGE_Engine)

add_executable(AnimationDemo Examples/AnimationDemo.cpp)
target_link_libraries(AnimationDemo SAGE_Engine)

add_executable(TextDemo Examples/TextDemo.cpp)
target_link_libraries(TextDemo SAGE_Engine)

add_executable(PhysicsDemo Examples/PhysicsDemo.cpp)
target_link_libraries(PhysicsDemo SAGE_Engine)

add_executable(TicTacToe Examples/TicTacToe.cpp)
target_link_libraries(TicTacToe SAGE_Engine)

add_executable(TMXDemo Examples/TMXDemo.cpp)
target_link_libraries(TMXDemo SAGE_Engine)

add_executable(CursorAndUIDemo Examples/CursorAndUIDemo.cpp)
target_link_libraries(CursorAndUIDemo SAGE_Engine)

add_executable(CatAnimationDemo Examples/CatAnimationDemo.cpp)
target_link_libraries(CatAnimationDemo SAGE_Engine)

# Installation Rules
install(TARGETS SAGE_Engine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
