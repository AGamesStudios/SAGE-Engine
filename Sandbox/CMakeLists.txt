cmake_minimum_required(VERSION 3.23)

# ============================================
# SAGE Sandbox Demo Application
# ============================================

add_executable(SAGE_Sandbox
    src/main.cpp
)

target_link_libraries(SAGE_Sandbox
    PRIVATE
        SAGE_Engine
)

set_target_properties(SAGE_Sandbox PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "SAGE_Sandbox"
)
