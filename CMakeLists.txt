################################################################################
cmake_minimum_required(VERSION 3.4.3)

################################################################################
## Dullahan main project/solution
project(dullahan)

################################################################################
## generics

# ensure location of CEF files is set at command line
if (CEF_INCLUDE_DIR STREQUAL "")
    MESSAGE("CEF_INCLUDE_DIR not set" FATAL)
endif()
if (CEF_LIB_DIR STREQUAL "")
    MESSAGE("CEF_LIB_DIR not set" FATAL)
endif()
if (CEF_BIN_DIR STREQUAL "")
    MESSAGE("CEF_BIN_DIR not set" FATAL)
endif()
if (CEF_RESOURCE_DIR STREQUAL "")
    MESSAGE("CEF_RESOURCE_DIR not set" FATAL)
endif()

# location of CEF libraries we link against
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_library(
        CEF_LIBRARY_DEBUG
        NAMES libcef.lib
        PATHS ${CEF_LIB_DIR}
        PATH_SUFFIXES debug
    )
    find_library(
        CEF_DLL_LIBRARY_DEBUG
        NAMES libcef_dll_wrapper.lib
        PATHS ${CEF_LIB_DIR}
        PATH_SUFFIXES debug
    )
    find_library(
        CEF_LIBRARY_RELEASE
        NAMES libcef.lib
        PATHS ${CEF_LIB_DIR}
        PATH_SUFFIXES release
    )
    find_library(
        CEF_DLL_LIBRARY_RELEASE
        NAMES libcef_dll_wrapper.lib
        PATHS ${CEF_LIB_DIR}
        PATH_SUFFIXES release
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_library(
        CEF_DLL_LIBRARY_DEBUG
        NAMES libcef_dll_wrapper.a
        PATHS ${CEF_LIB_DIR}
        PATH_SUFFIXES debug
    )
    find_library(
        CEF_DLL_LIBRARY_RELEASE
        NAMES libcef_dll_wrapper.a
        PATHS ${CEF_LIB_DIR}
        PATH_SUFFIXES release
    )
    find_library(
        CEF_FRAMEWORK_DEBUG
        NAMES "Chromium Embedded Framework"
        PATHS ${CEF_BIN_DIR}
        PATH_SUFFIXES debug
    )
    find_library(
        CEF_FRAMEWORK_RELEASE
        NAMES "Chromium Embedded Framework"
        PATHS ${CEF_BIN_DIR}/release
        PATH_SUFFIXES release
    )
    set(CEF_FRAMEWORK
        debug ${CEF_FRAMEWORK_DEBUG}
        optimized ${CEF_FRAMEWORK_RELEASE}
    )

    FIND_LIBRARY(OPENGL_FRAMEWORK OpenGL)
    FIND_LIBRARY(COCOA_FRAMEWORK Cocoa)
endif()

set(CEF_LIBRARY
    debug ${CEF_LIBRARY_DEBUG}
    optimized ${CEF_LIBRARY_RELEASE}
)
set(CEF_DLL_LIBRARY
    debug ${CEF_DLL_LIBRARY_DEBUG}
    optimized ${CEF_DLL_LIBRARY_RELEASE}
)

# set C and C++ flags
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -W3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -W3")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11  -xobjective-c++")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -xobjective-c++")
endif()

################################################################################
## dullahan libary

# add source files to library
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_win.cpp)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_mac.mm)
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
    src/dullahan_context_handler.cpp
    src/dullahan_context_handler.h
    src/dullahan_debug.h
    src/dullahan_impl.cpp
    src/dullahan_impl.h
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

################################################################################
## parse CEF version header and process into dullahan header
FILE(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CEF_VERSION_STR REGEX "\#define CEF_VERSION ")
FILE(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CHROME_VERSION_MAJOR_STR REGEX "\#define CHROME_VERSION_MAJOR ")
FILE(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CHROME_VERSION_MINOR_STR REGEX "\#define CHROME_VERSION_MINOR ")
FILE(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CHROME_VERSION_BUILD_STR REGEX "\#define CHROME_VERSION_BUILD ")
FILE(STRINGS ${CEF_INCLUDE_DIR}/cef_version.h CHROME_VERSION_PATCH_STR REGEX "\#define CHROME_VERSION_PATCH ")
CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/src/dullahan_version.h.in"
  "${PROJECT_SOURCE_DIR}/src/dullahan_version.h"
)

################################################################################
## dullahan host executable

# add source files to the application
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(
        dullahan_host
        src/host/dullahan_host.cpp
    )

    # define which include directories to pull in
    target_include_directories(
        dullahan_host
        PUBLIC
        ${CEF_INCLUDE_DIR}
        ${CEF_INCLUDE_DIR}/..
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_executable(
        DullahanHelper
        MACOSX_BUNDLE
        src/host/dullahan_host.cpp
    )
    # define which include directories to pull in
    target_include_directories(
        DullahanHelper
        PUBLIC
        ${CEF_INCLUDE_DIR}
        ${CEF_INCLUDE_DIR}/..
    )
endif()

# define which libs to link against
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(
        dullahan_host
        ${CEF_LIBRARY}
        ${CEF_DLL_LIBRARY}
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(
        DullahanHelper
        ${CEF_DLL_LIBRARY}
        ${CEF_FRAMEWORK}
    )
endif()

# we are building Windows executable, not a console app (default)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(dullahan_host PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(dullahan_host PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

# Windows commands to copy CEF 'bin', 'resources' folders to executable dir (needed at runtime)
# as well as the local HTML page of test links
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_custom_command(
    TARGET dullahan_host POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CEF_BIN_DIR}/$<CONFIGURATION>"
            "$<TARGET_FILE_DIR:dullahan_host>"
    COMMENT "Copying runtime files to executable directory")

add_custom_command(
    TARGET dullahan_host POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CEF_RESOURCE_DIR}"
            "$<TARGET_FILE_DIR:dullahan_host>"
    COMMENT "Copying resource files to executable directory")

add_custom_command(
    TARGET dullahan_host POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/dullahan_test_urls.html"
            "$<TARGET_FILE_DIR:dullahan_host>"
    COMMENT "Copying startup HTML page to executable directory")
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

################################################################################
## examples
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    ################################################################################
    ## webcube example application

    # add source file to application
    add_executable(
        webcube
        examples/webcube/webcube.cpp
        examples/webcube/webcube.h
        examples/webcube/webcube.rc
        examples/webcube/resource.h
    )

    # define which include directories to pull in
    target_include_directories(
        webcube
        PUBLIC
        src
    )

    # define which libs to link against
    target_link_libraries(
        webcube
        dullahan
        ${CEF_LIBRARY}
        ${CEF_DLL_LIBRARY}
        opengl32
        glu32
        comctl32
        winmm
    )

    # we are building Windows executable, not a console app (default)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(webcube PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
        set_target_properties(webcube PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

    # webcube example dependes on main library and host executable
    add_dependencies(webcube dullahan)
    add_dependencies(webcube dullahan_host)

    # set the web cube example as the default startup project in Visual Studio
    if("${CMAKE_VERSION}" VERSION_GREATER 3.6.2)
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "webcube")
        endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    endif()

    ################################################################################
    ## simplegl example application using FreeGLUT

    # add source file to application
    add_executable(
        simplegl
        examples/simplegl/simplegl.cpp
    )

    # define which include directories to pull in
    target_include_directories(
        simplegl
        PUBLIC
        src
        examples/simplegl/freeglut/include/GL
    )

    # determine location of FreeGLUT library to link against based on bitwidth
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FREEGLUT_LIBRARY "${PROJECT_SOURCE_DIR}/examples/simplegl/freeglut/lib/x64/freeglut.lib")
        set(FREEGLUT_DLL "${PROJECT_SOURCE_DIR}/examples/simplegl/freeglut/bin/x64/freeglut.dll")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(FREEGLUT_LIBRARY "${PROJECT_SOURCE_DIR}/examples/simplegl/freeglut/bin/freeglut.dll")
        set(FREEGLUT_DLL "${PROJECT_SOURCE_DIR}/examples/simplegl/freeglut/bin/freeglut.dll")
    endif()

    # copy over freeglut.dll
    add_custom_command(
        TARGET simplegl POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
                "${FREEGLUT_DLL}"
                "$<TARGET_FILE_DIR:simplegl>"
       COMMENT "Copying FreeGLUT DLL to executable directory")

    # define which libs to link against
    target_link_libraries(
        simplegl
        dullahan
        ${CEF_LIBRARY}
        ${CEF_DLL_LIBRARY}
        ${FREEGLUT_LIBRARY}
    )

    ################################################################################
    ## console example application

    # add source file to application
    add_executable(
        console
        examples/console/console.cpp
    )

    # define which include directories to pull in
    target_include_directories(
        console
        PUBLIC
        src
    )

    # define which libs to link against
    target_link_libraries(
        console
        dullahan
        ${CEF_LIBRARY}
        ${CEF_DLL_LIBRARY}
    )

    ################################################################################
    ## minimal CEF console example (doesn't use Dullahan)

    # add source file to the application
    add_executable(
        cef_minimal
        examples/cef_minimal/cef_minimal.cpp
    )

    # define which include directories to pull in
    target_include_directories(
        cef_minimal
        PUBLIC
        ${CEF_INCLUDE_DIR}
        ${CEF_INCLUDE_DIR}/..
    )

    # define which libs to link against
    target_link_libraries(
        cef_minimal
        ${CEF_LIBRARY}
        ${CEF_DLL_LIBRARY}
    )

    # cef_minimal example dependes on host executable
    add_dependencies(cef_minimal dullahan_host)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    ################################################################################
    ## osxgl example application

    # add source file to application
    add_executable(
        osxgl
        MACOSX_BUNDLE
        examples/osxgl/AppDelegate.h
        examples/osxgl/AppDelegate.mm
        examples/osxgl/LLOsxglView.h
        examples/osxgl/LLOsxglView.mm
        examples/osxgl/main.m
    )

    # define which include directories to pull in
    target_include_directories(
        osxgl
        PUBLIC
        src
    )

    # define which libs to link against
    target_link_libraries(
        osxgl
        dullahan
        ${CEF_DLL_LIBRARY}
        ${CEF_FRAMEWORK}
        ${OPENGL_FRAMEWORK}
        ${COCOA_FRAMEWORK}
    )

    add_dependencies(osxgl dullahan)
    add_dependencies(osxgl DullahanHelper)

    ################################################################################
    ## minimal CEF console example (doesn't use Dullahan)

    # add source file to application
    add_executable(
        cef_minimal
        MACOSX_BUNDLE
        examples/cef_minimal/cef_minimal.cpp
    )

    # define which include directories to pull in
    target_include_directories(
        cef_minimal
        PUBLIC
        ${CEF_INCLUDE_DIR}
        ${CEF_INCLUDE_DIR}/..
    )

    # define which libs to link against
    target_link_libraries(
        cef_minimal
        ${CEF_DLL_LIBRARY}
        ${CEF_FRAMEWORK}
        ${COCOA_FRAMEWORK}
    )

    add_dependencies(cef_minimal DullahanHelper)

endif()

################################################################################
# generic commands that have to go after everything else

###### set only a Release configuration
if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif()
