# ImGui library
cmake_minimum_required(VERSION 3.15)

project(imgui)

# ImGui core files
set(IMGUI_SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    # imgui_demo.cpp  # Not needed for editor
)

# ImGui headers
set(IMGUI_HEADERS
    imgui.h
    imgui_internal.h
    imconfig.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
)

# Backends
set(IMGUI_BACKEND_SOURCES
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

set(IMGUI_BACKEND_HEADERS
    backends/imgui_impl_glfw.h
    backends/imgui_impl_opengl3.h
)

# Create static library
add_library(imgui STATIC
    ${IMGUI_SOURCES}
    ${IMGUI_HEADERS}
    ${IMGUI_BACKEND_SOURCES}
    ${IMGUI_BACKEND_HEADERS}
)

# Include directories
target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/backends>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/imgui>
)

# Link GLFW
target_link_libraries(imgui PUBLIC glfw)

# Platform-specific OpenGL
if(WIN32)
    target_link_libraries(imgui PUBLIC opengl32)
elseif(APPLE)
    target_link_libraries(imgui PUBLIC "-framework OpenGL")
else()
    target_link_libraries(imgui PUBLIC GL)
endif()

# Set properties
set_target_properties(imgui PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    FOLDER "ThirdParty"
)

# Disable warnings for third-party code
if(MSVC)
    target_compile_options(imgui PRIVATE /W0)
    # Use MultiThreadedDLL runtime to match SAGE_Engine
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_property(TARGET imgui PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
    else()
        set_property(TARGET imgui PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()
else()
    target_compile_options(imgui PRIVATE -w)
    target_link_libraries(imgui PRIVATE m)  # Link math library on Unix
endif()
