#!/usr/bin/env bash

cd "$(dirname "$0")"

# turn on verbose debugging output for parabuild logs.
exec 4>&1; export BASH_XTRACEFD=4; set -x
# make errors fatal
set -e
# complain about unset env variables
set -u

if [ -z "$AUTOBUILD" ] ; then
    exit 1
fi

if [ "$OSTYPE" = "cygwin" ] ; then
    autobuild="$(cygpath -u $AUTOBUILD)"
else
    autobuild="$AUTOBUILD"
fi

top="$(pwd)"
stage="$(pwd)/stage"
DULLAHAN_SOURCE_DIR="$top/src"

# load autobuild provided shell functions and variables
source_environment_tempfile="$stage/source_environment.sh"
"$autobuild" source_environment > "$source_environment_tempfile"
. "$source_environment_tempfile"

build=${AUTOBUILD_BUILD_ID:=0}

pushd "$DULLAHAN_SOURCE_DIR"
    case "$AUTOBUILD_PLATFORM" in
        windows*)
            load_vsvars

            cd  "$stage"

            # create Visual Studio project files
            cmake .. \
                -G "$AUTOBUILD_WIN_CMAKE_GEN" \
                -DCEF_INCLUDE_DIR="$(cygpath -w "$stage/packages/include/cef/include")" \
                -DCEF_LIB_DIR="$(cygpath -w "$stage/packages/lib")" \
                -DCEF_BIN_DIR="$(cygpath -w "$stage/packages/bin")" \
                -DCEF_RESOURCE_DIR="$(cygpath -w "$stage/packages/resources")" \
                -DCMAKE_C_FLAGS="$LL_BUILD_RELEASE"

            # populate version_file (after CMake runs)
            cl \
                /Fo"$(cygpath -w "$stage/version.obj")" \
                /Fe"$(cygpath -w "$stage/version.exe")" \
                /I "$(cygpath -w "$stage/packages/include/cef")"  \
                /I "$(cygpath -w "$top/src")"  \
                "$(cygpath -w "$top/version.cpp")"
            "$stage/version.exe" > "$stage/version.txt"
            rm "$stage"/version.{obj,exe}

            # build individual projects but not examples
            build_sln "dullahan.sln" "Release|$AUTOBUILD_WIN_VSPLATFORM" dullahan
            build_sln "dullahan.sln" "Release|$AUTOBUILD_WIN_VSPLATFORM" dullahan_host

            # prepare the staging dirs
            cd ..
            mkdir -p "$stage/include/cef"
            mkdir -p "$stage/lib/release"
            mkdir -p "$stage/bin/release"
            mkdir -p "$stage/resources"
            mkdir -p "$stage/LICENSES"

            # Dullahan files
            cp "$DULLAHAN_SOURCE_DIR/dullahan.h" "$stage/include/cef/"
            cp "$DULLAHAN_SOURCE_DIR/dullahan_version.h" "$stage/include/cef/"
            cp "$stage/Release/dullahan.lib" "$stage/lib/release/"
            cp "$stage/Release/dullahan_host.exe" "$stage/bin/release/"

            # CEF libraries
            cp "$stage/packages/lib/Release/libcef.lib" "$stage/lib/release"
            cp "$stage/packages/lib/Release/libcef_dll_wrapper.lib" "$stage/lib/release"

            # CEF run time binaries
            cp -R "$stage/packages/bin/release/"* "$stage/bin/release/"

            # CEF resources
            cp -R "$stage/packages/resources/"* "$stage/resources/"

            # licenses
            cp "$top/CEF_LICENSE.txt" "$stage/LICENSES"
            cp "$top/LICENSE.txt" "$stage/LICENSES"
        ;;
        darwin*)
            cd  "$stage"

            # create Xcode project files
            cmake -G Xcode \
                -DCMAKE_OSX_ARCHITECTURES="$AUTOBUILD_CONFIGURE_ARCH" \
                -DCEF_INCLUDE_DIR="$stage/packages/include/cef/include" \
                -DCEF_LIB_DIR="$stage/packages/lib" \
                -DCEF_BIN_DIR="$stage/packages/bin" \
                -DCEF_RESOURCE_DIR="$stage/packages/resources" \
                -DCMAKE_C_FLAGS:STRING="$LL_BUILD_RELEASE" \
                -DCMAKE_CXX_FLAGS:STRING="$LL_BUILD_RELEASE" \
                ..

            # populate version_file (after CMake runs)
            g++ \
                -I "$stage/packages/include/cef" \
                -I "$DULLAHAN_SOURCE_DIR" \
                -o "$stage/version" \
                "$top/version.cpp"
            "$stage/version" > "$stage/version.txt"
            rm "$stage/version"

            # build projects
            xcodebuild -project dullahan.xcodeproj -target dullahan -configuration 'Release'
            xcodebuild -project dullahan.xcodeproj -target DullahanHelper -configuration 'Release'

            # copy files to staging ready to be packaged
            mkdir -p "$stage/include/cef"
            mkdir -p "$stage/lib/release"
            mkdir -p "$stage/LICENSES"
            cp "$stage/Release/libdullahan.a" "$stage/lib/release/"
            cp "$DULLAHAN_SOURCE_DIR/dullahan.h" "$stage/include/cef/"
            cp "$DULLAHAN_SOURCE_DIR/dullahan_version.h" "$stage/include/cef/"
            cp -R "$stage/Release/DullahanHelper.app" "$stage/lib/release"
            cp -R "$stage/Release/DullahanHelper.app" "$stage/lib/release/DullahanHelper (GPU).app"
            mv "$stage/lib/release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper" "$stage/lib/release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper (GPU)" 
            cp -R "$stage/Release/DullahanHelper.app" "$stage/lib/release/DullahanHelper (Renderer).app"
            mv "$stage/lib/release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper" "$stage/lib/release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper (Renderer)" 
            cp -R "$stage/Release/DullahanHelper.app" "$stage/lib/release/DullahanHelper (Plugin).app"
            mv "$stage/lib/release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper" "$stage/lib/release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper (Plugin)" 
            cp "$stage/packages/lib/release/libcef_dll_wrapper.a" "$stage/lib/release"
            cp -R "$stage/packages/bin/release/Chromium Embedded Framework.framework" "$stage/lib/release"
            cp "$top/CEF_LICENSE.txt" "$stage/LICENSES"
            cp "$top/LICENSE.txt" "$stage/LICENSES"
        ;;
        linux*)
            echo "This project is not currently supported for $AUTOBUILD_PLATFORM" 1>&2 ; exit 1
        ;;
    esac
popd
