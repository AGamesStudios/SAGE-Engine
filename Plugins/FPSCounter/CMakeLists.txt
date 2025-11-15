project(FPSCounterPlugin)

# Create plugin as shared library
add_library(FPSCounterPlugin SHARED
    FPSCounterPlugin.cpp
)

# Define export macro
target_compile_definitions(FPSCounterPlugin PRIVATE SAGE_PLUGIN_EXPORTS)

# Link with SAGE Engine
target_link_libraries(FPSCounterPlugin PRIVATE SAGE_Engine)

# Set output directory
set_target_properties(FPSCounterPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# Copy to Release plugins folder after build
add_custom_command(TARGET FPSCounterPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/bin/Release/plugins
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:FPSCounterPlugin>
        ${CMAKE_BINARY_DIR}/bin/Release/plugins/
    COMMENT "Copying FPSCounterPlugin to Release plugins folder"
)
