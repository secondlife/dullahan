################################################################################
## minimum version required - 3.6.3+ needed to set startup project
cmake_minimum_required(VERSION 3.6.3)

################################################################################
## Dullahan main project/solution
project(dullahan)

################################################################################
## generics

# ensure location of CEF files is set at command line
if (CEF_DIR STREQUAL "")
    MESSAGE("CEF_DIR not set" FATAL)
endif()

# location of CEF libraries we link against
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_library(
        CEF_LIBRARY_DEBUG
        NAMES libcef.lib
        PATHS ${CEF_DIR}/lib
        PATH_SUFFIXES debug
    )
    find_library(
        CEF_DLL_LIBRARY_DEBUG
        NAMES libcef_dll_wrapper.lib
        PATHS ${CEF_DIR}/lib
        PATH_SUFFIXES debug
    )
    find_library(
        CEF_LIBRARY_RELEASE
        NAMES libcef.lib
        PATHS ${CEF_DIR}/lib
        PATH_SUFFIXES release
    )
    find_library(
        CEF_DLL_LIBRARY_RELEASE
        NAMES libcef_dll_wrapper.lib
        PATHS ${CEF_DIR}/lib
        PATH_SUFFIXES release
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_library(
        CEF_DLL_LIBRARY_DEBUG
        NAMES libcef_dll_wrapper.a
        PATHS ${CEF_DIR}/lib
        PATH_SUFFIXES debug
    )
    find_library(
        CEF_DLL_LIBRARY_RELEASE
        NAMES libcef_dll_wrapper.a
        PATHS ${CEF_DIR}/lib
        PATH_SUFFIXES release
    )
    find_library(
        CEF_FRAMEWORK_DEBUG
        NAMES "Chromium Embedded Framework"
        PATHS ${CEF_DIR}/bin
        PATH_SUFFIXES debug
    )
    find_library(
        CEF_FRAMEWORK_RELEASE
        NAMES "Chromium Embedded Framework"
        PATHS ${CEF_DIR}/bin/release
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
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++14  -xobjective-c++")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++14 -xobjective-c++")
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
    ${CEF_DIR}
    ${CEF_DIR}/include
)

################################################################################
## dullahan host executable

# add source files to the application
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_executable(
    dullahan_host
    src/host/dullahan_host.cpp
)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
add_executable(
    dullahan_host
    MACOSX_BUNDLE
    src/host/dullahan_host.cpp
)
endif()


# define which include directories to pull in
target_include_directories(
    dullahan_host
    PUBLIC
    ${CEF_DIR}
    ${CEF_DIR}/include
)

# define which libs to link against
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_link_libraries(
        dullahan_host
        ${CEF_LIBRARY}
        ${CEF_DLL_LIBRARY}
)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
target_link_libraries(
        dullahan_host
        ${CEF_DLL_LIBRARY}
        ${CEF_FRAMEWORK}
)
endif()

# we are building Windows executable, not a console app (default)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(dullahan_host PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(dullahan_host PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

# dullahan host doesn't really depend on dullahan but we
# want to force it to be rebuilt as part of the whole project
add_dependencies(dullahan dullahan_host)

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
    )

    # we are building Windows executable, not a console app (default)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(webcube PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
        set_target_properties(webcube PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

    # command to copy CEF 'bin' folder to executable dir (needed at runtime)
    add_custom_command(
        TARGET webcube POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CEF_DIR}/bin/$<CONFIGURATION>"
                "$<TARGET_FILE_DIR:webcube>"
        COMMENT "Copying runtime files to executable directory")

    # command to copy CEF 'resources' folder to executable dir (needed at runtime)
    add_custom_command(
        TARGET webcube POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CEF_DIR}/resources"
                "$<TARGET_FILE_DIR:webcube>"
        COMMENT "Copying resource files to executable directory")

    # command to copy CEF start HTML page to executable dir (needed at runtime)
    add_custom_command(
        TARGET webcube POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/dullahan_test_urls.html"
                "$<TARGET_FILE_DIR:webcube>"
        COMMENT "Copying startup HTML page to executable directory")

    # webcube example dependes on main library and  host executable
    add_dependencies(webcube dullahan)
    add_dependencies(webcube dullahan_host)

    # set the web cube example as the default startup project in Visual Studio
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "webcube")
    endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")


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
    add_dependencies(osxgl dullahan_host)
endif()

################################################################################
# generic commands that have to go after everything else

###### set only a Release configuration
if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif()
