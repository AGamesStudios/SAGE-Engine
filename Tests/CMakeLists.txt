if(MSVC)
    # Используем статический CRT как и основной движок
    set(_SAGE_MSVC_DLL_RUNTIME "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    foreach(_sage_target SAGE_Engine glad glfw logcon logcon_integration)
        if(TARGET ${_sage_target})
            set_property(TARGET ${_sage_target} PROPERTY MSVC_RUNTIME_LIBRARY ${_SAGE_MSVC_DLL_RUNTIME})
        endif()
    endforeach()
endif()

set(TEST_SOURCES
    TestFramework.cpp
    TestRunner.cpp
    
    # Original existing tests
    Matrix4Tests.cpp
    MathRandomTests.cpp
    PNGLoaderTests.cpp
    Core/UTF8Tests.cpp
    Core/EventBusTests.cpp
    Core/ResourceManagerTests.cpp
    Core/HeadlessSmokeTests.cpp
    LogCon/LogConLexerSmokeTests.cpp
    LogCon/LogConParserTests.cpp
    
    # NEW COMPREHENSIVE TESTS (2024)
    ECS/ECSCoreTests.cpp
    ECS/ECSSystemTests.cpp
    Graphics/Camera2DTests.cpp
    Physics/PhysicsIntegrationTests.cpp
    Core/SerializationTests.cpp
    Integration/IntegrationTests.cpp
    Audio/AudioSystemTests.cpp
    Input/InputManagerTests.cpp
    
    # FULL ENGINE INTEGRATION TEST
    EngineIntegrationTest.cpp
)

add_executable(SAGETests ${TEST_SOURCES})

target_link_libraries(SAGETests PRIVATE SAGE_Engine)

target_include_directories(SAGETests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(SAGETests PRIVATE SAGE_ENGINE_TESTING)

if(MSVC)
    target_compile_options(SAGETests PRIVATE /W4 /FS)
    set_property(TARGET SAGETests PROPERTY MSVC_RUNTIME_LIBRARY ${_SAGE_MSVC_DLL_RUNTIME})
    # Удалён /NODEFAULTLIB:LIBCMT для предотвращения LNK4098
    # Force linker to include all object files (needed for static test registration)
    target_link_options(SAGETests PRIVATE /OPT:NOREF)
    if(SAGE_STRICT_WARNINGS)
        target_compile_options(SAGETests PRIVATE /WX)
    endif()
else()
    target_compile_options(SAGETests PRIVATE -Wall -Wextra -pedantic)
    if(SAGE_STRICT_WARNINGS)
        target_compile_options(SAGETests PRIVATE -Werror)
    endif()
endif()

target_compile_features(SAGETests PRIVATE cxx_std_20)

add_test(NAME SAGE_Engine_Tests COMMAND SAGETests)
