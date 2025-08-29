# ######################################################################################################################
# Dullahan

# ######################################################################################################################
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ######################################################################################################################
# Main project/solution
#
project(dullahan)

# ######################################################################################################################
# Grab and build CEF DLL Wrapper (define a CEF package URL or local DIR in the CMake command line)
#
if(NOT DEFINED CEF_PACKAGE_URL)
    if(NOT DEFINED CEF_PACKAGE_DIR)
        message(FATAL_ERROR "Required variable CEF_PACKAGE_URL or CEF_PACKAGE_DIR is not defined")
    endif()
endif()

include(FetchContent)
if(DEFINED CEF_PACKAGE_URL)
    fetchcontent_declare(cef_prebuild URL ${CEF_PACKAGE_URL} DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
elseif(DEFINED CEF_PACKAGE_DIR)
    fetchcontent_declare(cef_prebuild SOURCE_DIR ${CEF_PACKAGE_DIR})
endif()
fetchcontent_makeavailable(cef_prebuild)

set(CEF_ROOT "${cef_prebuild_SOURCE_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${cef_prebuild_SOURCE_DIR}/cmake")
find_package(CEF REQUIRED)

# ######################################################################################################################
# Dullahan Library
#
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_win.cpp)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_mac.mm)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_linux.cpp)
endif()

add_library(
    dullahan
    STATIC
    src/dullahan.cpp
    src/dullahan.h
    src/dullahan_browser_client.cpp
    src/dullahan_browser_client.h
    src/dullahan_callback_manager.cpp
    src/dullahan_callback_manager.h
    src/dullahan_debug.h
    src/dullahan_impl.cpp
    src/dullahan_impl.h
    src/dullahan_version.h
    src/dullahan_version.h.in
    ${KEYBOARD_IMPL_SRC_FILE}
    src/dullahan_impl_mouse.cpp
    src/dullahan_render_handler.cpp
    src/dullahan_render_handler.h
)

# ######################################################################################################################
# Populate the version header file from the template
#
file(REMOVE "${PROJECT_SOURCE_DIR}/src/dullahan_version.h")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CEF_VERSION_STR REGEX "\#define CEF_VERSION ")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CEF_VERSION_MAJOR_STR REGEX "\#define CEF_VERSION_MAJOR ")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CEF_VERSION_MINOR_STR REGEX "\#define CEF_VERSION_MINOR ")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CEF_VERSION_BUILD_STR REGEX "\#define CEF_VERSION_BUILD ")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CEF_VERSION_PATCH_STR REGEX "\#define CEF_VERSION_PATCH ")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CHROME_VERSION_MAJOR_STR REGEX "\#define CHROME_VERSION_MAJOR ")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CHROME_VERSION_MINOR_STR REGEX "\#define CHROME_VERSION_MINOR ")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CHROME_VERSION_BUILD_STR REGEX "\#define CHROME_VERSION_BUILD ")
file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CHROME_VERSION_PATCH_STR REGEX "\#define CHROME_VERSION_PATCH ")
string(TIMESTAMP BUILDNUMBER "#define DULLAHAN_VERSION_BUILD %Y%m%d%H%M")
configure_file(
    "${PROJECT_SOURCE_DIR}/src/dullahan_version.h.in"
    "${PROJECT_SOURCE_DIR}/src/dullahan_version.h"
)






#string(REGEX MATCH "[^\+]*" CEF_VERSION_STR_SHORT_PRE "${CEF_VERSION_STR}")
#string(REPLACE "CEF_VERSION" "CEF_VERSION_SHORT" CEF_VERSION_STR_SHORT "${CEF_VERSION_STR_SHORT_PRE}")





#file(STRINGS ${cef_prebuild_SOURCE_DIR}/include/cef_version.h CEF_VERSION_MAJOR_STR_PRE REGEX "\#define CEF_VERSION_MAJOR ")
#string(REGEX MATCH "[0-9]+" CEF_VERSION_MAJOR_STR "${CEF_VERSION_MAJOR_STR_PRE}")
#[[
#string(REGEX MATCH "#define CEF_VERSION_MAJOR [0-9]+" macrodef "${header}")
#string(REGEX MATCH "[0-9]+" CEF_VERSION_MAJOR_STR "${macrodef}")
#string(REGEX MATCH "#define CEF_VERSION_MINOR [0-9]+" macrodef "${header}")
#string(REGEX MATCH "[0-9]+" CEF_VERSION_MINOR_STR "${macrodef}")
#string(REGEX MATCH "#define CEF_VERSION_PATCH [0-9]+" macrodef "${header}")
#string(REGEX MATCH "[0-9]+" CEF_VERSION_PATCH_STR "${macrodef}")
]]

target_include_directories(
    dullahan
    PUBLIC
    "${cef_prebuild_SOURCE_DIR}/include" 
    "${cef_prebuild_SOURCE_DIR}/include/.."
)

# ######################################################################################################################
# Dullahan Host Executable(s)
#
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_executable(
        dullahan_host
        WIN32
        src/host/dullahan_host.cpp
        src/win/compatibility.manifest
    )

    target_include_directories(
        dullahan_host
        PUBLIC
        "${cef_prebuild_SOURCE_DIR}/include" 
        "${cef_prebuild_SOURCE_DIR}/include/.."
    )

    target_link_libraries(
        dullahan_host
        "${cef_prebuild_SOURCE_DIR}/Release/libcef.lib"
        "${cef_prebuild_BINARY_DIR}/libcef_dll_wrapper/Release/libcef_dll_wrapper.lib"
    )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # TODO
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # TODO
endif()

###############################################################################
# OpenGL Example Application
#
set (THIRD-PARTY-PREFIX ${CMAKE_SOURCE_DIR}/examples/opengl-example/third-party)
set (GLAD_FOLDER "glad-0.1.36")
set (IMGUI_FOLDER "imgui-1.92.2")
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (GLFW_FOLDER "glfw-3.4.bin.WIN64")
    set (GLFW_LIB_PATH "lib-vc2022/glfw3.lib")
    set (BUNDLE_ID "")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (GLFW_FOLDER "glfw-3.4.bin.MACOS")
    set (GLFW_LIB_PATH "lib-universal/libglfw3.a")
    set (GLFW_FRAMEWORKS "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
    set (BUNDLE_ID "MACOSX_BUNDLE")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endif()

add_executable(
    opengl-example
    ${BUNDLE_ID}
    examples/opengl-example/src/opengl-example.cpp
    examples/opengl-example/src/opengl-example.h
    ${THIRD-PARTY-PREFIX}/${GLAD_FOLDER}/src/glad.c
    ${THIRD-PARTY-PREFIX}/${IMGUI_FOLDER}/imgui.cpp
    ${THIRD-PARTY-PREFIX}/${IMGUI_FOLDER}/imgui_draw.cpp
    ${THIRD-PARTY-PREFIX}/${IMGUI_FOLDER}/imgui_impl_glfw.cpp
    ${THIRD-PARTY-PREFIX}/${IMGUI_FOLDER}/imgui_impl_opengl2.cpp
    ${THIRD-PARTY-PREFIX}/${IMGUI_FOLDER}/imgui_tables.cpp
    ${THIRD-PARTY-PREFIX}/${IMGUI_FOLDER}/imgui_widgets.cpp
    examples/opengl-example/README.md
)

target_include_directories(
    opengl-example
    PUBLIC
    src
    ${THIRD-PARTY-PREFIX}/${GLAD_FOLDER}/include
    ${THIRD-PARTY-PREFIX}/${GLFW_FOLDER}/include
    ${THIRD-PARTY-PREFIX}/${IMGUI_FOLDER}
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries (
        opengl-example
        dullahan
        opengl32
        "${cef_prebuild_SOURCE_DIR}/Release/libcef.lib"
        "${cef_prebuild_BINARY_DIR}/libcef_dll_wrapper/Release/libcef_dll_wrapper.lib"
        ${THIRD-PARTY-PREFIX}/${GLFW_FOLDER}/${GLFW_LIB_PATH}
    )

    SET_PROPERTY(DIRECTORY PROPERTY VS_STARTUP_PROJECT "opengl-example")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # TODO

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # TODO

endif()

# ######################################################################################################################
# Copy runtime files
#
# Also writes an extra copy of the folders to somewhere autobuild
# will know where to look and doesn't need to know the details
# of the CEF build in '_deps' etc.that might change in the future.
# Not needed for manual builds using build.bat|sh
#
file(MAKE_DIRECTORY "${DESTINATION_DIR}")
foreach(file_path IN LISTS CEF_BINARY_FILES)
    set(SOURCE_FILE "${cef_prebuild_SOURCE_DIR}/Release/${file_path}")

    set(DESTINATION_DIR "${CMAKE_BINARY_DIR}/Release")
    file(COPY "${SOURCE_FILE}" DESTINATION "${DESTINATION_DIR}")

    set(DESTINATION_DIR "${CMAKE_BINARY_DIR}/_ab_bin")
    file(COPY "${SOURCE_FILE}" DESTINATION "${DESTINATION_DIR}")
endforeach()
foreach(file_path IN LISTS CEF_RESOURCE_FILES)
    set(SOURCE_FILE "${cef_prebuild_SOURCE_DIR}/Resources/${file_path}")

    set(DESTINATION_DIR "${CMAKE_BINARY_DIR}/Release")
    file(COPY "${SOURCE_FILE}" DESTINATION "${DESTINATION_DIR}")

    set(DESTINATION_DIR "${CMAKE_BINARY_DIR}/_ab_resources")
    file(COPY "${SOURCE_FILE}" DESTINATION "${DESTINATION_DIR}")
endforeach()

# ######################################################################################################################
# General options
#
add_dependencies(dullahan libcef_dll_wrapper)
add_dependencies(opengl-example dullahan)
set(CMAKE_CONFIGURATION_TYPES "Release")
