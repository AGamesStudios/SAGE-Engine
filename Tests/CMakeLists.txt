set(TEST_SOURCES
    TestFramework.cpp
    VaultTests.cpp
    StageTests.cpp
    RendererSmokeTests.cpp
    SystemIntegrationTests.cpp
    # Temporarily disabled until test framework macro issues are resolved
    # InputTests.cpp
    # TextureTests.cpp
    # MaterialTests.cpp
    # SpriteTests.cpp
    main.cpp
)

add_executable(SAGE_Tests ${TEST_SOURCES})

if(MSVC)
    set_property(TARGET SAGE_Tests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

target_link_libraries(SAGE_Tests PRIVATE SAGE_Engine)

target_include_directories(SAGE_Tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

if(MSVC)
    target_compile_options(SAGE_Tests PRIVATE /W4)
    target_link_options(SAGE_Tests PRIVATE /NODEFAULTLIB:LIBCMT)
else()
    target_compile_options(SAGE_Tests PRIVATE -Wall -Wextra -pedantic)
endif()

target_compile_features(SAGE_Tests PRIVATE cxx_std_17)

add_test(NAME SAGE_Engine_Tests COMMAND SAGE_Tests)
