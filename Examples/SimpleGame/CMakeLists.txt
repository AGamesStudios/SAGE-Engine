cmake_minimum_required(VERSION 3.20)
project(SAGESimpleGame)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SAGE Engine
# Option 1: Using installed SAGE (uncomment if using install)
# set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../../install")
# find_package(SAGE REQUIRED)

# Option 2: Using SAGE as subdirectory (default for development)
if(NOT TARGET SAGE_Engine)
    add_subdirectory(${CMAKE_SOURCE_DIR}/../../Engine ${CMAKE_BINARY_DIR}/SAGE_Engine)
endif()

# Create executable
add_executable(SAGESimpleGame main.cpp)

# Link SAGE Engine
if(TARGET SAGE::SAGE_Engine)
    # Using installed SAGE
    target_link_libraries(SAGESimpleGame PRIVATE SAGE::SAGE_Engine)
else()
    # Using SAGE as subdirectory
    target_link_libraries(SAGESimpleGame PRIVATE SAGE_Engine)
endif()

# Set runtime library for MSVC
if(MSVC)
    set_property(TARGET SAGESimpleGame PROPERTY 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Copy Assets directory to build folder (if exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Assets")
    add_custom_command(TARGET SAGESimpleGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:SAGESimpleGame>/Assets"
        COMMENT "Copying Assets to build directory"
    )
endif()

# Set as startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SAGESimpleGame)
