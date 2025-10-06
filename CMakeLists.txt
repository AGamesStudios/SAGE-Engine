cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
project(SAGE_Engine VERSION 1.0.0 LANGUAGES CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SAGE_ENABLE_SANITIZERS "Enable Address Sanitizer for supported compilers" OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

if(SAGE_ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

# Опции проекта
option(SAGE_BUILD_TESTS "Build unit tests" ON)
option(SAGE_BUILD_EXAMPLES "Build example projects" ON)

# Добавление сторонних библиотек
add_subdirectory(ThirdParty)

# Добавление движка
add_subdirectory(Engine)

if(SAGE_BUILD_TESTS)
    target_compile_definitions(SAGE_Engine PUBLIC SAGE_ENGINE_TESTING)
endif()

if(SAGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

# Добавление примеров
if(SAGE_BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

# Добавление тестового приложения
