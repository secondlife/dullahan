###############################################################################
# Dullahan CMake file - Callum Prentice - 2020-05-03

###############################################################################
cmake_minimum_required(VERSION 3.13...4.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

###############################################################################
# Dullahan main project/solution
project(dullahan)

###############################################################################
#  Determine platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(IS_WINDOWS TRUE)
    set(BUILD_EXAMPLES TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(IS_MACOS TRUE)
    set(BUILD_EXAMPLES FALSE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_LINUX TRUE)
    set(BUILD_EXAMPLES FALSE)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

###############################################################################
# functions
function(check_exists variable)
    if(NOT ${variable})
        message(FATAL_ERROR ${variable} " is not set")
    else()
        message(STATUS "--> ${variable} is ${${variable}}")
    endif()
endfunction()

option( USE_SPOTIFY_CEF "Use a prebuild CEF from spotify" Off )
option( SPOTIFY_CEF_URL "URL to the prebuild CEF from spotify" "" )

if( USE_SPOTIFY_CEF )
    include(FetchContent)

    FetchContent_Declare( cef_prebuild URL  ${SPOTIFY_CEF_URL} )
    FetchContent_MakeAvailable(cef_prebuild)

    set(CEF_ROOT "${cef_prebuild_SOURCE_DIR}")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${cef_prebuild_SOURCE_DIR}/cmake")
    find_package(CEF REQUIRED)
endif()


if( NOT USE_SPOTIFY_CEF )
    ###############################################################################
    # The user must pass in CEF_WRAPPER_DIR and CEF_WRAPPER_BUILD_DIR then we
    # derrive all the other ones we need based on those
    set(CEF_INCLUDE_DIR ${CEF_WRAPPER_DIR}/include)
    set(CEF_RELEASE_LIB_DIR ${CEF_WRAPPER_DIR}/Release)
    set(CEF_RELEASE_DLL_LIB_DIR ${CEF_WRAPPER_BUILD_DIR}/libcef_dll_wrapper/Release)
    set(CEF_RELEASE_BIN_DIR ${CEF_WRAPPER_DIR}/Release)
    set(CEF_RESOURCES_DIR ${CEF_WRAPPER_DIR}/Resources)

    # Check if all our variables exist and bail with
    # a fatal error if any of them are missing
    check_exists(CEF_WRAPPER_DIR)
    check_exists(CEF_WRAPPER_BUILD_DIR)
    check_exists(CEF_INCLUDE_DIR)
    check_exists(CEF_RELEASE_LIB_DIR)
    check_exists(CEF_RELEASE_DLL_LIB_DIR)
    check_exists(CEF_RELEASE_BIN_DIR)
    check_exists(CEF_RESOURCES_DIR)
else()
    set(CEF_INCLUDE_DIR ${cef_prebuild_SOURCE_DIR}/include)
    set(CEF_RELEASE_LIB_DIR ${cef_prebuild_SOURCE_DIR}/Release)
    set(CEF_RELEASE_DLL_LIB_DIR ${CEF_WRAPPER_BUILD_DIR}/libcef_dll_wrapper/Release)
    set(CEF_RELEASE_BIN_DIR ${cef_prebuild_SOURCE_DIR}/Release)
    set(CEF_RESOURCES_DIR ${cef_prebuild_SOURCE_DIR}/Resources)

    set( CEF_DLL_LIBRARY libcef_dll_wrapper )
    set( CEF_LIBRARY ${CEF_LIB_RELEASE} )
endif()

###############################################################################
# location of CEF libraries we link against
if(IS_WINDOWS)
    find_library(
        CEF_LIBRARY_RELEASE
        NAMES libcef.lib
        PATHS ${CEF_RELEASE_LIB_DIR}
        NO_DEFAULT_PATH
    )

    find_library(
        CEF_DLL_LIBRARY_RELEASE
        NAMES libcef_dll_wrapper.lib
        PATHS ${CEF_RELEASE_DLL_LIB_DIR}
        NO_DEFAULT_PATH
    )

    # Confirm that we were able to find our link libs
    check_exists(CEF_LIBRARY_RELEASE)
    check_exists(CEF_DLL_LIBRARY_RELEASE)

elseif(IS_MACOS)
    find_library(
        CEF_DLL_LIBRARY_RELEASE
        NAMES libcef_dll_wrapper.a
        PATHS ${CEF_RELEASE_DLL_LIB_DIR}
    )

    find_library(OPENGL_FRAMEWORK OpenGL)
    find_library(COCOA_FRAMEWORK Cocoa)

    # Check that we were able to find our build components
    check_exists(CEF_DLL_LIBRARY_RELEASE)
    check_exists(OPENGL_FRAMEWORK)
    check_exists(COCOA_FRAMEWORK)
elseif(IS_LINUX)
    set(CMAKE_CXX_LINK_FLAGS "-Wl,--no-keep-memory -Wl,--build-id -Wl,-rpath,'$ORIGIN:$ORIGIN/../lib' -Wl,--exclude-libs,ALL")

    if( NOT USE_SPOTIFY_CEF )
        find_library(
            CEF_LIBRARY_RELEASE
            NAMES libcef.so
            PATHS ${CEF_RELEASE_LIB_DIR}
            PATH_SUFFIXES release
        )
        find_library(
            CEF_DLL_LIBRARY_RELEASE
            NAMES libcef_dll_wrapper.a
            PATHS ${CEF_WRAPPER_BUILD_DIR}/libcef_dll_wrapper/
            PATH_SUFFIXES release
        )
        set(CEF_LIBRARY ${CEF_LIBRARY_RELEASE} )
        set(CEF_DLL_LIBRARY ${CEF_DLL_LIBRARY_RELEASE} )
    else()
        set( CEF_DLL_LIBRARY libcef_dll_wrapper )
        set( CEF_LIBRARY ${CEF_LIB_RELEASE} )
    endif()
endif()

###############################################################################
# Final layer of finding the right libs for each combination
# of name, platform, configuration, type etc.
if(NOT IS_LINUX)
  set(CEF_LIBRARY
      optimized ${CEF_LIBRARY_RELEASE}
  )
  set(CEF_DLL_LIBRARY
      optimized ${CEF_DLL_LIBRARY_RELEASE}
  )
endif()

check_exists(CEF_LIBRARY)
check_exists(CEF_DLL_LIBRARY)

###############################################################################
# set C and C++ flags
# Warnings at level 4 (-W4 generates too much spew) but disable:
#    4100 "unreferenced parameter" -  too much spew for cef code
#    4127 "conditional is constant" - I use an explicity var to turn code on and off which triggers this
#    4505 "unreferenced local function has been removed" - supress meaningless freeglut warning
if(IS_WINDOWS)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -W4 -wd4100 -wd4127 -wd4505")
elseif(IS_MACOS)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -xobjective-c++")
elseif(IS_LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-array-bounds")
endif()

###############################################################################
# Dullahan libary

# add source files to library
if(IS_WINDOWS)
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_win.cpp)
elseif(IS_MACOS)
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_mac.mm)
elseif(IS_LINUX)
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

# define which include directories to pull in
target_include_directories(
    dullahan
    PUBLIC
    ${CEF_INCLUDE_DIR}
    ${CEF_INCLUDE_DIR}/..
)

# turn off spurious linker warnings
if(IS_WINDOWS)
    set_target_properties(dullahan PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

###############################################################################
# Parse CEF version header and process into the Dullahan header file.
# Only do this if the version input template changed to try to keep
# version/build number the same for the same set of source files)
if("${PROJECT_SOURCE_DIR}/src/dullahan_version.h.in" IS_NEWER_THAN "${PROJECT_SOURCE_DIR}/src/dullahan_version.h")

    # Extract CEF/Chrome version info from CEF header
    file(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CEF_VERSION_STR REGEX "\#define CEF_VERSION ")
    file(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CHROME_VERSION_MAJOR_STR REGEX "\#define CHROME_VERSION_MAJOR ")
    file(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CHROME_VERSION_MINOR_STR REGEX "\#define CHROME_VERSION_MINOR ")
    file(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CHROME_VERSION_BUILD_STR REGEX "\#define CHROME_VERSION_BUILD ")
    file(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CHROME_VERSION_PATCH_STR REGEX "\#define CHROME_VERSION_PATCH ")

    # Create a build number based on the actual build date/time
    string(TIMESTAMP BUILDNUMBER "#define DULLAHAN_VERSION_BUILD %Y%m%d%H%M")

    # Parse the version input file into the Dullahan version header
    configure_file(
        "${PROJECT_SOURCE_DIR}/src/dullahan_version.h.in"
        "${PROJECT_SOURCE_DIR}/src/dullahan_version.h"
    )
endif()

###############################################################################
# Dullahan host executable

# add source files to the application
if(IS_WINDOWS)
    add_executable(
        dullahan_host
        WIN32
        src/host/dullahan_host.cpp
        src/win/compatibility.manifest
    )

    # define which include directories to pull in
    target_include_directories(
        dullahan_host
        PUBLIC
        ${CEF_INCLUDE_DIR}
        ${CEF_INCLUDE_DIR}/..
    )

    set_target_properties(dullahan_host PROPERTIES LINK_FLAGS "/ignore:4099")

    target_link_libraries(
        dullahan_host
        ${CEF_LIBRARY}
        ${CEF_DLL_LIBRARY}
    )
elseif(IS_MACOS)
    # Below based on official CEF example code

    # CEF Helper app suffixes.
    # Format is "<name suffix>:<target suffix>:<plist suffix>".
    set(DULLAHAN_HELPER_APP_SUFFIXES
        "::"
        " (Alerts):_alerts:.alerts"
        " (GPU):_gpu:.gpu"
        " (Plugin):_plugin:.plugin"
        " (Renderer):_renderer:.renderer"
        )

    # Create the multiple Helper app bundle targets.
    foreach(_suffix_list ${DULLAHAN_HELPER_APP_SUFFIXES})
        # Convert to a list and extract the suffix values.
        string(REPLACE ":" ";" _suffix_list ${_suffix_list})
        list(GET _suffix_list 0 _name_suffix)
        list(GET _suffix_list 1 _target_suffix)
        list(GET _suffix_list 2 _plist_suffix)

        # Define Helper target and output names.
        set(_helper_target "dullahan_host${_target_suffix}")
        set(_helper_output_name "DullahanHelper${_name_suffix}")

        # Create Helper-specific variants of the helper-Info.plist file. Do this
        # manually because the configure_file command (which is executed as part of
        # MACOSX_BUNDLE_INFO_PLIST) uses global env variables and would insert the
        # wrong values with multiple targets.
        set(_helper_info_plist "${CMAKE_CURRENT_BINARY_DIR}/helper-Info${_target_suffix}.plist")
        file(READ "${PROJECT_SOURCE_DIR}/src/host/Info.plist.in" _plist_contents)
        string(REPLACE "\${EXECUTABLE_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
        string(REPLACE "\${PRODUCT_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
        string(REPLACE "\${BUNDLE_ID_SUFFIX}" "${_plist_suffix}" _plist_contents ${_plist_contents})
        file(WRITE ${_helper_info_plist} ${_plist_contents})

        # Create Helper executable target.
        add_executable(${_helper_target} MACOSX_BUNDLE src/host/dullahan_host.cpp)

        # define which include directories to pull in
        target_include_directories(
            ${_helper_target}
            PUBLIC
            ${CEF_INCLUDE_DIR}
            ${CEF_INCLUDE_DIR}/..
        )
        target_link_libraries(${_helper_target} ${CEF_DLL_LIBRARY} ${DULLAHAN_STANDARD_LIBS})
        set_target_properties(${_helper_target} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${_helper_info_plist}
        OUTPUT_NAME ${_helper_output_name}
        )
    endforeach()
elseif(IS_LINUX)
    add_executable(
        dullahan_host
        src/host/dullahan_host.cpp
    )
    target_include_directories(
        dullahan_host
        PUBLIC
        ${CEF_INCLUDE_DIR}
        ${CEF_INCLUDE_DIR}/..
    )

    target_link_libraries(
        dullahan_host
        ${CEF_DLL_LIBRARY}
        ${CEF_LIBRARY}
    )
endif()

# Windows commands to copy CEF binaries and 'resources' folders to
# executable dir since they're needed at runtime

if(IS_WINDOWS)
    add_custom_command(
        TARGET dullahan_host POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "$<$<CONFIG:release>:${CEF_RELEASE_BIN_DIR}>"
                "$<TARGET_FILE_DIR:dullahan_host>"
        COMMENT "Copying runtime files to executable directory")

    add_custom_command(
        TARGET dullahan_host POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CEF_RESOURCES_DIR}"
                "$<TARGET_FILE_DIR:dullahan_host>"
        COMMENT "Copying resource files to executable directory")
endif()

###############################################################################
# Examples
if (BUILD_EXAMPLES)
    set (THIRD-PARTY-PREFIX ${CMAKE_SOURCE_DIR}/examples/opengl-example/third-party)
    set (GLAD_FOLDER "glad-0.1.36")
    set (IMGUI_FOLDER "imgui-1.92.2")
    if(IS_WINDOWS)
        set (GLFW_FOLDER "glfw-3.4.bin.WIN64")
        set (GLFW_LIB_PATH "lib-vc2022/glfw3.lib")
    elseif(IS_MACOS)
        set (GLFW_FOLDER "glfw-3.4.bin.MACOS")
        set (GLFW_LIB_PATH "lib-universal/libglfw3.a")
        set (GLFW_FRAMEWORKS "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
    endif()

    ###############################################################################
    # OpenGL example application

    add_executable(
        opengl-example
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

    if(IS_WINDOWS)
        target_link_libraries (
            opengl-example
            dullahan
            opengl32
            ${CEF_LIBRARY}
            ${CEF_DLL_LIBRARY}
            ${THIRD-PARTY-PREFIX}/${GLFW_FOLDER}/${GLFW_LIB_PATH}
        )

        SET_PROPERTY(DIRECTORY PROPERTY VS_STARTUP_PROJECT "opengl-example")
    elseif(IS_MACOS)
        find_library (OPENGL_FRAMEWORK OpenGL)
        find_library (COCOA_FRAMEWORK Cocoa)
        find_library (IO_FRAMEWORK IOKit)
        target_link_libraries (
            opengl-example 
            ${THIRD-PARTY-PREFIX}/${GLFW_FOLDER}/${GLFW_LIB_PATH}
            ${OPENGL_FRAMEWORK}
            ${COCOA_FRAMEWORK} 
            ${IO_FRAMEWORK}
        )
    endif()

    add_dependencies(opengl-example dullahan)
    add_dependencies(opengl-example dullahan_host)

endif()

###############################################################################
# generic commands that have to go after everything else

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Release")
endif()

# Install the Dullahan library and host executable

if( USE_SPOTIFY_CEF )
  install( TARGETS dullahan_host DESTINATION bin/release )
  install( TARGETS dullahan ${CEF_DLL_LIBRARY} DESTINATION lib/release )

  foreach(binFile IN LISTS CEF_BINARY_FILES)
	if(IS_DIRECTORY ${CEF_BINARY_DIR_RELEASE}/${binFile} )
	  install( DIRECTORY ${CEF_BINARY_DIR_RELEASE}/${binFile} DESTINATION lib/release )
	  install( DIRECTORY ${CEF_BINARY_DIR_RELEASE}/${binFile} DESTINATION bin/release )
	else()
	  install( PROGRAMS ${CEF_BINARY_DIR_RELEASE}/${binFile} DESTINATION lib/release )
	  install( PROGRAMS ${CEF_BINARY_DIR_RELEASE}/${binFile} DESTINATION bin/release )
	endif()
  endforeach()
  foreach(resFile IN LISTS CEF_RESOURCE_FILES)
	if(IS_DIRECTORY ${CEF_RESOURCE_DIR}/${resFile} )
	  install( DIRECTORY  ${CEF_RESOURCE_DIR}/${resFile}  DESTINATION resources)
	else()
	  install( FILES ${CEF_RESOURCE_DIR}/${resFile}  DESTINATION resources)
	endif()
  endforeach()
endif()