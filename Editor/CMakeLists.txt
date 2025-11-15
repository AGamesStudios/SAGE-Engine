cmake_minimum_required(VERSION 3.20)

# SAGE Editor executable
add_executable(SAGEEditor
    main.cpp
    EditorBridge.cpp
    EditorBridge.h
    EditorApplication.cpp
    EditorApplication.h
    EditorConfig.cpp
    EditorConfig.h
    EditorScene.cpp
    EditorScene.h
    SelectionContext.h
    Project.cpp
    Project.h
    EditorSceneWrapper.h
    Viewport.cpp
    Viewport.h
    ViewportSelection.cpp
    HierarchyPanel.cpp
    HierarchyPanel.h
    InspectorPanel.cpp
    InspectorPanel.h
    AssetBrowserPanel.cpp
    AssetBrowserPanel.h
    AssetBrowserPanel_FileOps.cpp
    ImageViewerWindow.cpp
    ImageViewerWindow.h
    GameWindow.cpp
    GameWindow.h
    Localization.cpp
    Localization.h
    Undo/EditorCommands.h
    Notifications/NotificationBus.h
    Notifications/NotificationBus.cpp
)

target_link_libraries(SAGEEditor
    PRIVATE
        SAGE_Engine
        imgui
        glad
    glfw
    nlohmann_json
)

target_include_directories(SAGEEditor
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Engine/include  # Public API only
        ${CMAKE_SOURCE_DIR}/Engine          # Temporary: still needed for legacy code
        ${CMAKE_SOURCE_DIR}/ThirdParty
        ${CMAKE_SOURCE_DIR}/ThirdParty/imgui
)

# Set MSVC runtime library to match SAGE_Engine
if(MSVC)
    # Use global CMAKE_MSVC_RUNTIME_LIBRARY setting (already set in root CMakeLists)
    # Avoid excluding default CRT libs; previous /NODEFAULTLIB directives caused missing symbols in Release
    if(SAGE_STRICT_WARNINGS)
        target_compile_options(SAGEEditor PRIVATE /WX)
    endif()
    # Optional: ensure correct runtime explicitly if policy not applied
    set_target_properties(SAGEEditor PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Set output directory
set_target_properties(SAGEEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    FOLDER "Editor"
)

target_compile_features(SAGEEditor PRIVATE cxx_std_20)
