# Добавление GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

# No MSVC-specific runtime setting - controlled by root CMakeLists.txt via CMAKE_MSVC_RUNTIME_LIBRARY

# Suppress legacy CRT warnings in GLFW (strncpy, strtok, etc.)
if(MSVC)
    target_compile_definitions(glfw PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# GLAD
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/glad/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# No MSVC-specific runtime setting - controlled by root CMakeLists.txt via CMAKE_MSVC_RUNTIME_LIBRARY

# Header-only dependencies (stb_image, stb_truetype, miniaudio)
option(SAGE_OFFLINE_THIRDPARTY "Не скачивать заголовки из сети, ожидать их локально" OFF)
set(THIRDPARTY_GENERATED_DIR ${CMAKE_BINARY_DIR}/thirdparty)
file(MAKE_DIRECTORY ${THIRDPARTY_GENERATED_DIR})

set(STB_DIR ${THIRDPARTY_GENERATED_DIR}/stb)
set(MINIAUDIO_DIR ${THIRDPARTY_GENERATED_DIR}/miniaudio)
file(MAKE_DIRECTORY ${STB_DIR})
file(MAKE_DIRECTORY ${MINIAUDIO_DIR})

function(download_header url dest)
    if(SAGE_OFFLINE_THIRDPARTY)
        if(NOT EXISTS ${dest})
            message(WARNING "[offline] Требуется локальный файл: ${dest}; пропущено скачивание ${url}")
        endif()
        return()
    endif()
    set(_need_download FALSE)
    if(NOT EXISTS ${dest})
        set(_need_download TRUE)
    else()
        file(SIZE ${dest} _size_check)
        if(_size_check EQUAL 0)
            message(STATUS "Перезакачиваем пустой файл ${dest} из ${url}")
            set(_need_download TRUE)
        endif()
    endif()
    if(_need_download)
        file(DOWNLOAD
            ${url}
            ${dest}
            STATUS download_status
            SHOW_PROGRESS)
        list(GET download_status 0 status_code)
        if(NOT status_code EQUAL 0)
            message(WARNING "Failed to download ${url}: ${download_status} — переключитесь на SAGE_OFFLINE_THIRDPARTY=ON если без сети")
        endif()
    endif()
endfunction()

set(STB_IMAGE_HEADER ${STB_DIR}/stb_image.h)
set(STB_TRUETYPE_HEADER ${STB_DIR}/stb_truetype.h)
download_header("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" ${STB_IMAGE_HEADER})
download_header("https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h" ${STB_TRUETYPE_HEADER})

set(STB_INCLUDE_DIR ${STB_DIR})
if(EXISTS ${STB_IMAGE_HEADER})
    file(SIZE ${STB_IMAGE_HEADER} STB_IMAGE_SIZE)
else()
    set(STB_IMAGE_SIZE 0)
endif()
if(EXISTS ${STB_TRUETYPE_HEADER})
    file(SIZE ${STB_TRUETYPE_HEADER} STB_TRUETYPE_SIZE)
else()
    set(STB_TRUETYPE_SIZE 0)
endif()
if(STB_IMAGE_SIZE EQUAL 0 OR STB_TRUETYPE_SIZE EQUAL 0)
    message(WARNING "stb headers unavailable in build tree; falling back to source copies under ThirdParty/")
    set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# nlohmann/json (header-only)
set(JSON_INCLUDE_ROOT ${THIRDPARTY_GENERATED_DIR})
set(JSON_DIR ${JSON_INCLUDE_ROOT}/nlohmann)
file(MAKE_DIRECTORY ${JSON_DIR})
set(JSON_HEADER ${JSON_DIR}/json.hpp)
download_header("https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp" ${JSON_HEADER})

set(JSON_FALLBACK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)
set(NLOHMANN_INCLUDE_DIR ${JSON_INCLUDE_ROOT})
if(EXISTS ${JSON_HEADER})
    file(SIZE ${JSON_HEADER} JSON_HEADER_SIZE)
else()
    set(JSON_HEADER_SIZE 0)
endif()
if(JSON_HEADER_SIZE EQUAL 0)
    message(WARNING "nlohmann/json header unavailable in build tree; falling back to ThirdParty/nlohmann/json.hpp")
    set(NLOHMANN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE 
    $<BUILD_INTERFACE:${NLOHMANN_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SAGE/ThirdParty>
)

# tinyxml2 for TMX parsing (allow offline stub)
set(SAGE_TINYXML2_AVAILABLE FALSE)
if(NOT SAGE_OFFLINE_THIRDPARTY)
    include(FetchContent)
    FetchContent_Declare(
        tinyxml2
        GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
        GIT_TAG 9.0.0
    )
    set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(tinyxml2)
    install(TARGETS tinyxml2 EXPORT SAGETargets)
    set(SAGE_TINYXML2_AVAILABLE TRUE)
else()
    set(_tinyxml2_local_dir ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)
    if(EXISTS ${_tinyxml2_local_dir}/tinyxml2.h)
        message(WARNING "[offline] Using local ThirdParty/tinyxml2/ headers; ensure they remain up to date.")
        add_library(tinyxml2 INTERFACE)
        target_include_directories(tinyxml2 INTERFACE ${_tinyxml2_local_dir})
        set(SAGE_TINYXML2_AVAILABLE TRUE)
    else()
        message(WARNING "[offline] Skipping tinyxml2 fetch; creating interface stub target. TMX parsing will be disabled.")
        add_library(tinyxml2 INTERFACE)
    endif()
    install(TARGETS tinyxml2 EXPORT SAGETargets)
endif()

set(SAGE_TINYXML2_AVAILABLE ${SAGE_TINYXML2_AVAILABLE} PARENT_SCOPE)

# Add to install exports
install(TARGETS nlohmann_json 
    EXPORT SAGETargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SAGE/ThirdParty
)
set(STB_EASY_FONT_HEADER ${STB_DIR}/stb_easy_font.h)
download_header("https://raw.githubusercontent.com/nothings/stb/master/stb_easy_font.h" ${STB_EASY_FONT_HEADER})
set(MINIAUDIO_HEADER ${MINIAUDIO_DIR}/miniaudio.h)
download_header("https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h" ${MINIAUDIO_HEADER})

set(MINIAUDIO_INCLUDE_DIR ${MINIAUDIO_DIR})
if(EXISTS ${MINIAUDIO_HEADER})
    file(SIZE ${MINIAUDIO_HEADER} MINIAUDIO_SIZE)
else()
    set(MINIAUDIO_SIZE 0)
endif()
if(MINIAUDIO_SIZE EQUAL 0)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/miniaudio.h)
        message(WARNING "miniaudio header unavailable in build tree; falling back to ThirdParty/miniaudio/")
        set(MINIAUDIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio)
    else()
        message(WARNING "miniaudio header missing. Audio functionality may be limited without network fetch.")
    endif()
endif()

add_library(sage_thirdparty_headers INTERFACE)
target_include_directories(sage_thirdparty_headers INTERFACE
    $<BUILD_INTERFACE:${STB_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${MINIAUDIO_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SAGE/ThirdParty>
)

# Box2D (optional physics backend)
if(SAGE_PHYSICS_BACKEND STREQUAL "BOX2D")
    include(FetchContent)
    FetchContent_Declare(
        box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d.git
        GIT_TAG v2.4.1
    )
    set(BOX2D_BUILD_TESTS OFF CACHE BOOL "Disable Box2D tests" FORCE)
    set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "Disable Box2D testbed (avoids glad/glfw duplication)" FORCE)
    FetchContent_MakeAvailable(box2d)
    # Ensure consistent runtime on MSVC
    if(MSVC AND TARGET box2d)
        set_property(TARGET box2d PROPERTY MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY})
    endif()
endif()

# ImGui (if available)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h)
    message(STATUS "ImGui found - enabling debug tools")
    add_subdirectory(imgui)
else()
    message(WARNING "ImGui not found. Download from https://github.com/ocornut/imgui and copy to ThirdParty/imgui/")
endif()
