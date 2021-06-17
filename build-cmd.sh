#!/usr/bin/env bash

cd "$(dirname "$0")"

# turn on verbose debugging output for parabuild logs.
exec 4>&1; export BASH_XTRACEFD=4; set -x

# make errors fatal
set -e

# bleat on references to undefined shell variables
set -u

# Check autobuild is around or fail
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
dullahan_source_dir=$top/src

# The name of the directory that `autobuild install` drops the
# CEF package into - no way to share this name between CEF and
# Dullahan so they have to be kept in lock step manually
cef_no_wrapper_dir="$stage/packages/cef"

# The name of the directory that the libcef_dll_wrapper gets
# built in - we also need to refer to it to get the build result
# libraries for Dullahan so we must specify it exactly once here.
cef_no_wrapper_build_dir="$cef_no_wrapper_dir/build"

# load autobuild provided shell functions and variables
source_environment_tempfile="$stage/source_environment.sh"
"$autobuild" source_environment > "$source_environment_tempfile"
. "$source_environment_tempfile"

build=${AUTOBUILD_BUILD_ID:=0}

case "$AUTOBUILD_PLATFORM" in
    windows*)
        load_vsvars

        # We've observed some weird failures in which the PATH is too big to be
        # passed to a child process! When that gets munged, we start seeing errors
        # like failing to find the 'mt.exe' command. Thing is, by this point
        # in the script we've acquired a shocking number of duplicate entries.
        # Dedup the PATH using Python's OrderedDict, which preserves the order in
        # which you insert keys.
        # We find that some of the Visual Studio PATH entries appear both with and
        # without a trailing slash, which is pointless. Strip those off and dedup
        # what's left.
        # Pass the existing PATH as an explicit argument rather than reading it
        # from the environment to bypass the fact that cygwin implicitly converts
        # PATH to Windows form when running a native executable. Since we're
        # setting bash's PATH, leave everything in cygwin form. That means
        # splitting and rejoining on ':' rather than on os.pathsep, which on
        # Windows is ';'.
        # Use python -u, else the resulting PATH will end with a spurious '\r'.
        export PATH="$(python -u -c "import sys
from collections import OrderedDict
print(':'.join(OrderedDict((dir.rstrip('/'), 1) for dir in sys.argv[1].split(':'))))" "$PATH")"

        # build the CEF c->C++ wrapper "libcef_dll_wrapper"
        cd "$cef_no_wrapper_dir"
        rm -rf "$cef_no_wrapper_build_dir"
        mkdir -p "$cef_no_wrapper_build_dir"
        cd "$cef_no_wrapper_build_dir"
        cmake -G "$AUTOBUILD_WIN_CMAKE_GEN" -DCEF_RUNTIME_LIBRARY_FLAG=/MD -DUSE_SANDBOX=Off ..
        build_sln cef.sln "Release|$AUTOBUILD_WIN_VSPLATFORM" "libcef_dll_wrapper"
        build_sln cef.sln "Debug|$AUTOBUILD_WIN_VSPLATFORM" "libcef_dll_wrapper"

        # generate the project files for Dullahan
        cd "$stage"
        cmake .. \
            -G "$AUTOBUILD_WIN_CMAKE_GEN" \
            -DCEF_WRAPPER_DIR="$(cygpath -w "$cef_no_wrapper_dir")" \
            -DCEF_WRAPPER_BUILD_DIR="$(cygpath -w "$cef_no_wrapper_build_dir")" \
            -DCMAKE_C_FLAGS="$LL_BUILD_RELEASE"

        # build individual dullahan libraries but not examples
        build_sln "dullahan.sln" "Release|$AUTOBUILD_WIN_VSPLATFORM" dullahan
        build_sln "dullahan.sln" "Release|$AUTOBUILD_WIN_VSPLATFORM" dullahan_host

        # prepare the staging dirs
        cd ..
        mkdir -p "$stage/include/cef"
        mkdir -p "$stage/lib/release"
        mkdir -p "$stage/bin/release"
        mkdir -p "$stage/bin/release/swiftshader"
        mkdir -p "$stage/resources"
        mkdir -p "$stage/LICENSES"

        # Dullahan files
        cp "$dullahan_source_dir/dullahan.h" "$stage/include/cef/"
        cp "$dullahan_source_dir/dullahan_version.h" "$stage/include/cef/"
        cp "$stage/Release/dullahan.lib" "$stage/lib/release/"
        cp "$stage/Release/dullahan_host.exe" "$stage/bin/release/"

        # CEF libraries
        cp "$cef_no_wrapper_dir/Release/libcef.lib" "$stage/lib/release"
        cp "$cef_no_wrapper_build_dir/libcef_dll_wrapper/Release/libcef_dll_wrapper.lib" "$stage/lib/release"

        # CEF run time binaries (copy individually except SwiftShader so it's
        # obvious when a file is removed and this part of the script fails)
        cp -R "$cef_no_wrapper_dir/Release/swiftshader/"* "$stage/bin/release/swiftshader/"
        cp "$cef_no_wrapper_dir/Release/chrome_elf.dll" "$stage/bin/release/"
        cp "$cef_no_wrapper_dir/Release/d3dcompiler_47.dll" "$stage/bin/release/"
        cp "$cef_no_wrapper_dir/Release/libcef.dll" "$stage/bin/release/"
        cp "$cef_no_wrapper_dir/Release/libEGL.dll" "$stage/bin/release/"
        cp "$cef_no_wrapper_dir/Release/libGLESv2.dll" "$stage/bin/release/"
        cp "$cef_no_wrapper_dir/Release/snapshot_blob.bin" "$stage/bin/release/"
        cp "$cef_no_wrapper_dir/Release/v8_context_snapshot.bin" "$stage/bin/release/"

        # CEF resources
        cp -R "$cef_no_wrapper_dir/Resources/"* "$stage/resources/"

        # licenses
        cp "$top/CEF_LICENSE.txt" "$stage/LICENSES"
        cp "$top/LICENSE.txt" "$stage/LICENSES"

        # populate version_file (after CMake runs)
        cl \
            /Fo"$(cygpath -w "$stage/version.obj")" \
            /Fe"$(cygpath -w "$stage/version.exe")" \
            /I "$(cygpath -w "$cef_no_wrapper_dir/include/")"  \
            /I "$(cygpath -w "$top/src")"  \
            "$(cygpath -w "$top/tools/autobuild_version.cpp")"
        "$stage/version.exe" > "$stage/version.txt"
        rm "$stage"/version.{obj,exe}
    ;;
    darwin*)
        # build the CEF c->C++ wrapper "libcef_dll_wrapper"
        cd "$cef_no_wrapper_dir"
        rm -rf "$cef_no_wrapper_build_dir"
        mkdir -p "$cef_no_wrapper_build_dir"
        cd "$cef_no_wrapper_build_dir"
        cmake -G Xcode -DPROJECT_ARCH="x86_64" ..
        xcodebuild -project cef.xcodeproj -target libcef_dll_wrapper -configuration Debug
        xcodebuild -project cef.xcodeproj -target libcef_dll_wrapper -configuration Release

        # build Dullahan
        cd  "$stage"
        cmake -G Xcode \
            -DCMAKE_OSX_ARCHITECTURES="$AUTOBUILD_CONFIGURE_ARCH" \
            -DCEF_WRAPPER_DIR="$cef_no_wrapper_dir" \
            -DCEF_WRAPPER_BUILD_DIR="$cef_no_wrapper_build_dir" \
            -DCMAKE_C_FLAGS:STRING="$LL_BUILD_RELEASE" \
            -DCMAKE_CXX_FLAGS:STRING="$LL_BUILD_RELEASE" \
            ..
        xcodebuild -project dullahan.xcodeproj -target dullahan -configuration Release
        xcodebuild -project dullahan.xcodeproj -target DullahanHelper -configuration Release

        # copy files to staging ready to be packaged
        mkdir -p "$stage/include/cef"
        mkdir -p "$stage/lib/release"
        mkdir -p "$stage/LICENSES"
        cp "$stage/Release/libdullahan.a" "$stage/lib/release/"
        cp "$dullahan_source_dir/dullahan.h" "$stage/include/cef/"
        cp "$dullahan_source_dir/dullahan_version.h" "$stage/include/cef/"
        cp -R "$stage/Release/DullahanHelper.app" "$stage/lib/release"
        cp -R "$stage/Release/DullahanHelper.app" "$stage/lib/release/DullahanHelper (GPU).app"
        mv "$stage/lib/release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper" "$stage/lib/release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper (GPU)"
        cp -R "$stage/Release/DullahanHelper.app" "$stage/lib/release/DullahanHelper (Renderer).app"
        mv "$stage/lib/release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper" "$stage/lib/release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper (Renderer)"
        cp -R "$stage/Release/DullahanHelper.app" "$stage/lib/release/DullahanHelper (Plugin).app"
        mv "$stage/lib/release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper" "$stage/lib/release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper (Plugin)"
        cp "$cef_no_wrapper_build_dir/libcef_dll_wrapper/Release/libcef_dll_wrapper.a" "$stage/lib/release"
        cp -R "$cef_no_wrapper_dir/Release/Chromium Embedded Framework.framework" "$stage/lib/release"
        cp "$top/CEF_LICENSE.txt" "$stage/LICENSES"
        cp "$top/LICENSE.txt" "$stage/LICENSES"

        # populate version_file (after CMake runs)
        g++ \
            -I "$cef_no_wrapper_dir/include/" \
            -I "$top/src" \
            -o "$stage/version" \
            "$top/tools/autobuild_version.cpp"
        "$stage/version" > "$stage/version.txt"
        rm "$stage/version"
    ;;
    linux64)
		#Force version regeneration.
		rm -f src/dullahan_version.h

        # build the CEF c->C++ wrapper "libcef_dll_wrapper"
		cd "${cef_no_wrapper_dir}"
        rm -rf "${cef_no_wrapper_build_dir}"
        mkdir -p "${cef_no_wrapper_build_dir}"
        cd "${cef_no_wrapper_build_dir}"
        cmake -G  Ninja ..
		ninja libcef_dll_wrapper
		
        cd "$stage"
        cmake .. -G  Ninja -DCEF_WRAPPER_DIR="${cef_no_wrapper_dir}" \
            -DCEF_WRAPPER_BUILD_DIR="${cef_no_wrapper_build_dir}" \
			  -DCMAKE_C_FLAGS:STRING="$LL_BUILD_RELEASE -m${AUTOBUILD_ADDRSIZE}" \
			  -DCMAKE_CXX_FLAGS:STRING="$LL_BUILD_RELEASE -m${AUTOBUILD_ADDRSIZE}"

		ninja

		g++ -std=c++11 	-I "${cef_no_wrapper_dir}/include" 	-I "${dullahan_source_dir}" -o "$stage/version"  "$top/tools/autobuild_version.cpp"

		"$stage/version" > "$stage/VERSION.txt"
		rm "$stage/version"
		
		mkdir -p "$stage/LICENSES"
		mkdir -p "$stage/bin/release/"
		mkdir -p "$stage/include"
		mkdir -p "$stage/include/cef"
		mkdir -p "$stage/lib/release/swiftshader"
		mkdir -p "$stage/resources"
		 
		cp libdullahan.a ${stage}/lib/release/
		cp ${cef_no_wrapper_build_dir}/libcef_dll_wrapper/libcef_dll_wrapper.a $stage/lib/release

		cp -a ${cef_no_wrapper_dir}/Release/*.so ${stage}/lib/release/
		cp -a ${cef_no_wrapper_dir}/Release/swiftshader/* ${stage}/lib/release/swiftshader/

		cp dullahan_host ${stage}/bin/release/

		cp -a ${cef_no_wrapper_dir}/Release/*.bin ${stage}/bin/release/
		cp -a ${cef_no_wrapper_dir}/Release/chrome-sandbox ${stage}/bin/release/

		cp -R ${cef_no_wrapper_dir}/Resources/* ${stage}/resources/
		cp ${dullahan_source_dir}/dullahan.h ${stage}/include/cef/
		cp ${dullahan_source_dir}/dullahan_version.h ${stage}/include/cef/
        cp "$top/CEF_LICENSE.txt" "$stage/LICENSES"
        cp "$top/LICENSE.txt" "$stage/LICENSES"
		;;
esac
