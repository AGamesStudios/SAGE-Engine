# Minimal third-party setup for the reset engine baseline

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/glad/include>
        $<INSTALL_INTERFACE:include>
)

# Box2D Physics Engine
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_USER_SETTINGS OFF CACHE BOOL "" FORCE)

add_subdirectory(box2d)
add_subdirectory(tinyxml2)

if(MSVC)
    target_compile_definitions(glfw PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

add_library(sage_thirdparty INTERFACE)
target_link_libraries(sage_thirdparty INTERFACE glfw glad box2d tinyxml2)
