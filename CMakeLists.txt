###############################################################################
# Dullahan CMake file - Callum Prentice - 2020-05-03

###############################################################################
cmake_minimum_required(VERSION 3.11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

###############################################################################
# Dullahan main project/solution
project(dullahan)

###############################################################################
# functions
function(check_exists variable)
    if(NOT ${variable})
        message(FATAL_ERROR ${variable} " is not set")
    else()
        message(STATUS "--> ${variable} is ${${variable}}")
    endif()
endfunction()

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

###############################################################################
# location of CEF libraries we link against
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
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

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_library(
        CEF_DLL_LIBRARY_RELEASE
        NAMES libcef_dll_wrapper.a
        PATHS ${CEF_RELEASE_DLL_LIB_DIR}
    )

    set(CEF_FRAMEWORK "'${CEF_RELEASE_BIN_DIR}/Chromium Embedded Framework.framework'")

    find_library(OPENGL_FRAMEWORK OpenGL)
    find_library(COCOA_FRAMEWORK Cocoa)

    # Check that we were able to find our build components
    check_exists(CEF_DLL_LIBRARY_RELEASE)
    check_exists(CEF_FRAMEWORK)
    check_exists(OPENGL_FRAMEWORK)
    check_exists(COCOA_FRAMEWORK)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_LINK_FLAGS "-Wl,--no-keep-memory -Wl,--build-id -Wl,-rpath,'$ORIGIN:$ORIGIN/../lib' -Wl,--exclude-libs,ALL")
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
endif()

###############################################################################
# Final layer of finding the right libs for each combination
# of name, platform, configuration, type etc.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CEF_LIBRARY ${CEF_LIBRARY_RELEASE} )
  set(CEF_DLL_LIBRARY ${CEF_DLL_LIBRARY_RELEASE} )
else()
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
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -W4 -wd4100 -wd4127 -wd4505")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -xobjective-c++")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    if( NOT ENABLE_CXX11_ABI )
      add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    endif()
endif()

###############################################################################
# Dullahan libary

# add source files to library
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_win.cpp)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(KEYBOARD_IMPL_SRC_FILE src/dullahan_impl_keyboard_mac.mm)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
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
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
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
    # Info.plist.in template adds a new key called "LSUIElement" with value "true"
    # to surpress the appearance of the DullahanHelper app icon in the dock
    set_target_properties(DullahanHelper PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "DullahanHelper"
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/src/host/Info.plist.in")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
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
        "-F ${CEF_FRAMEWORK}"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(
        dullahan_host
        ${CEF_DLL_LIBRARY}
        ${CEF_LIBRARY}
    )
endif()

# we are building Windows executable, not a console app (default) and turn off spurious linker warnings
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(dullahan_host PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(dullahan_host PROPERTIES LINK_FLAGS "/ignore:4099")

    add_custom_command(
        TARGET dullahan_host POST_BUILD
        COMMAND "${CMAKE_MT}" -manifest \"$(ProjectDir)..\\src\\win\\compatibility.manifest\" -outputresource:"$(TargetDir)$(TargetFileName)"\;\#1
        COMMENT "Adding custom manifest...")
endif()

# Windows commands to copy CEF binaries and 'resources' folders to
# executable dir since they're needed at runtime

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
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
# examples
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    ###############################################################################
    # webcube example application

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

    # we are building Windows executable, not a console app (default) and turn off spurious linker warnings
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(webcube PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
        set_target_properties(webcube PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()

    # webcube example dependes on main library and host executable
    add_dependencies(webcube dullahan)
    add_dependencies(webcube dullahan_host)

    # set the web cube example as the default startup project in Visual Studio
    if("${CMAKE_VERSION}" VERSION_GREATER 3.6.2)
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            SET_PROPERTY(DIRECTORY PROPERTY VS_STARTUP_PROJECT "webcube")
        endif()
    endif()

    add_custom_command(
        TARGET webcube POST_BUILD
        COMMAND "${CMAKE_MT}" -manifest \"$(ProjectDir)..\\src\\win\\compatibility.manifest\" -outputresource:"$(TargetDir)$(TargetFileName)"\;\#1
        COMMENT "Adding custom manifest...")

    ###############################################################################
    # simplegl example application using FreeGLUT

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
        set(FREEGLUT_LIBRARY "${PROJECT_SOURCE_DIR}/examples/simplegl/freeglut/lib/freeglut.lib")
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

    # turn off spurious linker warnings
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(simplegl PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()

    add_custom_command(
        TARGET simplegl POST_BUILD
        COMMAND "${CMAKE_MT}" -manifest \"$(ProjectDir)..\\src\\win\\compatibility.manifest\" -outputresource:"$(TargetDir)$(TargetFileName)"\;\#1
        COMMENT "Adding custom manifest...")

    ###############################################################################
    # console example application

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

    # turn off spurious linker warnings
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(console PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()

    add_custom_command(
        TARGET console POST_BUILD
        COMMAND "${CMAKE_MT}" -manifest \"$(ProjectDir)..\\src\\win\\compatibility.manifest\" -outputresource:"$(TargetDir)$(TargetFileName)"\;\#1
        COMMENT "Adding custom manifest...")

    ###############################################################################
    # minimal CEF console example (doesn't use Dullahan)

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

    # turn off spurious linker warnings
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(cef_minimal PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()

    add_custom_command(
        TARGET cef_minimal POST_BUILD
        COMMAND "${CMAKE_MT}" -manifest \"$(ProjectDir)..\\src\\win\\compatibility.manifest\" -outputresource:"$(TargetDir)$(TargetFileName)"\;\#1
        COMMENT "Adding custom manifest...")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    ###############################################################################
    # osxgl example application

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
        "-F ${CEF_FRAMEWORK}"
        ${OPENGL_FRAMEWORK}
        ${COCOA_FRAMEWORK}
    )

    add_dependencies(osxgl dullahan)
    add_dependencies(osxgl DullahanHelper)

    # default Inof.plist.in template in CMake doesn't contain
    # the NSPrincipalClass definition so we must add it
    set(PRINCIPAL_CLASS "NSApplication")
    set_target_properties(osxgl PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "OSXGL Test"
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/examples/osxgl/Info.plist.in")
endif()

###############################################################################
# generic commands that have to go after everything else

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Release")
endif()
