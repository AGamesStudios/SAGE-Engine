project(ExamplePlugin)

add_library(ExamplePlugin SHARED
    ExamplePlugin.cpp
)

target_link_libraries(ExamplePlugin PRIVATE SAGE_Engine)

# Ensure output directory exists
add_custom_command(TARGET ExamplePlugin PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/Plugins
)

# Copy to bin/Plugins folder
add_custom_command(TARGET ExamplePlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:ExamplePlugin>
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/Plugins/$<TARGET_FILE_NAME:ExamplePlugin>
)
