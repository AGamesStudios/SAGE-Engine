cmake_minimum_required(VERSION 3.20)
project(PongTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# Add executable
add_executable(PongTest
    main.cpp
)

# Link with SAGE Engine
target_link_libraries(PongTest PRIVATE SAGE_Engine)
target_include_directories(PongTest PRIVATE ${CMAKE_SOURCE_DIR}/Engine)

# Set output directory
set_target_properties(PongTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Examples/PongTest"
)

# Windows: Create standalone executable (no console, all dependencies static)
if(WIN32)
    # Hide console window in Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(PongTest PROPERTIES
            WIN32_EXECUTABLE TRUE  # No console window
        )
    endif()
    
    # Static linking to avoid DLL dependencies
    if(SAGE_STATIC_BUILD)
        target_compile_options(PongTest PRIVATE /MT$<$<CONFIG:Debug>:d>)
    endif()
endif()
