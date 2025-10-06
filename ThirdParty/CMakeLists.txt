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
set(THIRDPARTY_GENERATED_DIR ${CMAKE_BINARY_DIR}/thirdparty)
file(MAKE_DIRECTORY ${THIRDPARTY_GENERATED_DIR})

set(STB_DIR ${THIRDPARTY_GENERATED_DIR}/stb)
set(MINIAUDIO_DIR ${THIRDPARTY_GENERATED_DIR}/miniaudio)
file(MAKE_DIRECTORY ${STB_DIR})
file(MAKE_DIRECTORY ${MINIAUDIO_DIR})

function(download_header url dest)
	if(NOT EXISTS ${dest})
		file(DOWNLOAD
			${url}
			${dest}
			STATUS download_status
			SHOW_PROGRESS)
		list(GET download_status 0 status_code)
		if(NOT status_code EQUAL 0)
			message(FATAL_ERROR "Failed to download ${url}: ${download_status}")
		endif()
	endif()
endfunction()

download_header("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" ${STB_DIR}/stb_image.h)
download_header("https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h" ${STB_DIR}/stb_truetype.h)
download_header("https://raw.githubusercontent.com/nothings/stb/master/stb_easy_font.h" ${STB_DIR}/stb_easy_font.h)
download_header("https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h" ${MINIAUDIO_DIR}/miniaudio.h)

add_library(sage_thirdparty_headers INTERFACE)
target_include_directories(sage_thirdparty_headers INTERFACE
    $<BUILD_INTERFACE:${STB_DIR}>
    $<BUILD_INTERFACE:${MINIAUDIO_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SAGE/ThirdParty>
)
