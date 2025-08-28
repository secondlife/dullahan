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
cef_dll_wrapper_dir="$stage/packages/cef"

# load autobuild provided shell functions and variables
source_environment_tempfile="$stage/source_environment.sh"
"$autobuild" source_environment > "$source_environment_tempfile"
. "$source_environment_tempfile"

source "$(dirname "$AUTOBUILD_VARIABLES_FILE")/functions"

build=${AUTOBUILD_BUILD_ID:=0}

case "$AUTOBUILD_PLATFORM" in
    windows*)
        load_vsvars

        cd "$stage"

        cmake .. \
           -G "$AUTOBUILD_WIN_CMAKE_GEN" -A "$AUTOBUILD_WIN_VSPLATFORM" \
            -DCEF_RUNTIME_LIBRARY_FLAG=/MD \
            -DUSE_SANDBOX=Off \
            -DCEF_PACKAGE_DIR="$(cygpath -m "$cef_dll_wrapper_dir")"

        cmake --build . --config Release --target dullahan --parallel $AUTOBUILD_CPU_COUNT
        cmake --build . --config Release --target dullahan_host --parallel $AUTOBUILD_CPU_COUNT

        # prepare the staging dirs
        mkdir -p "$stage/include/cef"
        mkdir -p "$stage/lib/release"
        mkdir -p "$stage/bin/release"
        mkdir -p "$stage/resources"
        mkdir -p "$stage/LICENSES"

        # Dullahan files
        cp "$dullahan_source_dir/dullahan.h" "$stage/include/cef/"
        cp "$dullahan_source_dir/dullahan_version.h" "$stage/include/cef/"
        cp "$stage/Release/dullahan.lib" "$stage/lib/release/"
        cp "$stage/Release/dullahan_host.exe" "$stage/bin/release/"

        # CEF libraries
        cp "$cef_dll_wrapper_dir/Release/libcef.lib" "$stage/lib/release"
        cp "$stage/_deps/cef_prebuild-build/libcef_dll_wrapper/Release/libcef_dll_wrapper.lib" "$stage/lib/release"

        # CEF binary files
        cp "$cef_dll_wrapper_dir/Release/"* "$stage/bin/release/"
        rm "$stage/bin/release/libcef.lib"

        # CEF resources
        cp -R "$cef_dll_wrapper_dir/Resources/"* "$stage/resources/"

        # licenses
        cp "$top/CEF_LICENSE.txt" "$stage/LICENSES"
        cp "$top/LICENSE.txt" "$stage/LICENSES"

        # populate version_file (after CMake runs)
        cl \
            -Fo"$(cygpath -w "$stage/version.obj")" \
            -Fe"$(cygpath -w "$stage/version.exe")" \
            -I "$(cygpath -w "$cef_dll_wrapper_dir/include/")" \
            -I "$(cygpath -w "$top/src")"  \
            "$(cygpath -w "$top/tools/autobuild_version.cpp")"
        "$stage/version.exe" > "$stage/version.txt"
        rm "$stage"/version.{obj,exe}
    ;;
    darwin*)
        export MACOSX_DEPLOYMENT_TARGET="$LL_BUILD_DARWIN_DEPLOY_TARGET"

        opts="$LL_BUILD_RELEASE"
        plainopts="$(remove_cxxstd $opts)"

        # build the CEF c->C++ wrapper "libcef_dll_wrapper"
        mkdir -p "$cef_no_wrapper_build_dir/x86_64"
        pushd "$cef_no_wrapper_build_dir/x86_64"
            cmake -G Xcode -DCMAKE_BUILD_TYPE=Release \
              -DPROJECT_ARCH="x86_64" \
              -DCMAKE_C_FLAGS="$plainopts" \
              -DCMAKE_CXX_FLAGS="$opts" \
              -DCMAKE_OSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} \
              $(cmake_cxx_standard $opts) \
              ../../x86_64
            cmake --build . --config Release --target libcef_dll_wrapper --parallel $AUTOBUILD_CPU_COUNT
        popd

        # build Dullahan
        mkdir -p "$stage/build_x86_64"
        pushd "$stage/build_x86_64"
            cmake "$top" -G Xcode -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_OSX_ARCHITECTURES="x86_64" \
                -DCEF_WRAPPER_DIR="$cef_no_wrapper_dir/x86_64" \
                -DCEF_WRAPPER_BUILD_DIR="$cef_no_wrapper_build_dir/x86_64" \
                -DCMAKE_C_FLAGS:STRING="$plainopts" \
                -DCMAKE_CXX_FLAGS:STRING="$opts" \
                -DCMAKE_OSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} \
                $(cmake_cxx_standard $opts)

            cmake --build . --config Release --target dullahan
            cmake --build . --config Release --target dullahan_host --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host_alerts --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host_gpu --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host_renderer --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host_plugin --parallel $AUTOBUILD_CPU_COUNT
        popd

        mkdir -p "$cef_no_wrapper_build_dir/arm64"
        pushd "$cef_no_wrapper_build_dir/arm64"
            cmake -G Xcode -DCMAKE_BUILD_TYPE=Release \
                -DPROJECT_ARCH="arm64" \
                -DCMAKE_C_FLAGS="$plainopts" \
                -DCMAKE_CXX_FLAGS="$opts" \
                -DCMAKE_OSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} \
                $(cmake_cxx_standard $opts) \
                ../../arm64
            cmake --build . --config Release --target libcef_dll_wrapper --parallel $AUTOBUILD_CPU_COUNT
        popd

        # build Dullahan
        mkdir -p "$stage/build_arm64"
        pushd "$stage/build_arm64"
            cmake "$top" -G Xcode -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_OSX_ARCHITECTURES="arm64" \
                -DCEF_WRAPPER_DIR="$cef_no_wrapper_dir/arm64" \
                -DCEF_WRAPPER_BUILD_DIR="$cef_no_wrapper_build_dir/arm64" \
                -DCMAKE_C_FLAGS:STRING="$plainopts" \
                -DCMAKE_CXX_FLAGS:STRING="$opts" \
                -DCMAKE_OSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} \
                $(cmake_cxx_standard $opts)

            cmake --build . --config Release --target dullahan --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host_alerts --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host_gpu --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host_renderer --parallel $AUTOBUILD_CPU_COUNT
            cmake --build . --config Release --target dullahan_host_plugin --parallel $AUTOBUILD_CPU_COUNT
        popd

        # copy files to staging ready to be packaged
        mkdir -p "$stage/include/cef"
        mkdir -p "$stage/lib/release"
        mkdir -p "$stage/LICENSES"
        lipo -create -output $stage/lib/release/libdullahan.a "$stage/build_x86_64/Release/libdullahan.a" "$stage/build_arm64/Release/libdullahan.a"
        cp "$dullahan_source_dir/dullahan.h" "$stage/include/cef/"
        cp "$dullahan_source_dir/dullahan_version.h" "$stage/include/cef/"
        cp -R "$stage/build_x86_64/Release/DullahanHelper.app" "$stage/lib/release"

        cp -R "$stage/build_x86_64/Release/DullahanHelper (Alerts).app" "$stage/lib/release/DullahanHelper (Alerts).app"
        cp -R "$stage/build_x86_64/Release/DullahanHelper (GPU).app" "$stage/lib/release/DullahanHelper (GPU).app"
        cp -R "$stage/build_x86_64/Release/DullahanHelper (Renderer).app" "$stage/lib/release/DullahanHelper (Renderer).app"
        cp -R "$stage/build_x86_64/Release/DullahanHelper (Plugin).app" "$stage/lib/release/DullahanHelper (Plugin).app"

        lipo -create "$stage/build_x86_64/Release/DullahanHelper.app/Contents/MacOS/DullahanHelper" "$stage/build_arm64/Release/DullahanHelper.app/Contents/MacOS/DullahanHelper" -output "$stage/lib/release/DullahanHelper.app/Contents/MacOS/DullahanHelper"
        lipo -create "$stage/build_x86_64/Release/DullahanHelper (Alerts).app/Contents/MacOS/DullahanHelper (Alerts)" "$stage/build_arm64/Release/DullahanHelper (Alerts).app/Contents/MacOS/DullahanHelper (Alerts)" -output "$stage/lib/release/DullahanHelper (Alerts).app/Contents/MacOS/DullahanHelper (Alerts)"
        lipo -create "$stage/build_x86_64/Release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper (GPU)" "$stage/build_arm64/Release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper (GPU)" -output "$stage/lib/release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper (GPU)"
        lipo -create "$stage/build_x86_64/Release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper (Plugin)" "$stage/build_arm64/Release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper (Plugin)" -output "$stage/lib/release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper (Plugin)"
        lipo -create "$stage/build_x86_64/Release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper (Renderer)" "$stage/build_arm64/Release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper (Renderer)" -output "$stage/lib/release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper (Renderer)"

        lipo -create -output "$stage/lib/release/libcef_dll_wrapper.a" "$cef_no_wrapper_build_dir/x86_64/libcef_dll_wrapper/Release/libcef_dll_wrapper.a" "$cef_no_wrapper_build_dir/arm64/libcef_dll_wrapper/Release/libcef_dll_wrapper.a"

        cp -R "$cef_no_wrapper_dir/x86_64/Release/Chromium Embedded Framework.framework" "$stage/lib/release"

        lipo -create "$cef_no_wrapper_dir/x86_64/Release/Chromium Embedded Framework.framework/Chromium Embedded Framework" "$cef_no_wrapper_dir/arm64/Release/Chromium Embedded Framework.framework/Chromium Embedded Framework" -output "$stage/lib/release/Chromium Embedded Framework.framework/Chromium Embedded Framework"

        lipo -create "$cef_no_wrapper_dir/x86_64/Release/Chromium Embedded Framework.framework/Libraries/libcef_sandbox.dylib" "$cef_no_wrapper_dir/arm64/Release/Chromium Embedded Framework.framework/Libraries/libcef_sandbox.dylib" -output "$stage/lib/release/Chromium Embedded Framework.framework/Libraries/libcef_sandbox.dylib"
        lipo -create "$cef_no_wrapper_dir/x86_64/Release/Chromium Embedded Framework.framework/Libraries/libEGL.dylib" "$cef_no_wrapper_dir/arm64/Release/Chromium Embedded Framework.framework/Libraries/libEGL.dylib" -output "$stage/lib/release/Chromium Embedded Framework.framework/Libraries/libEGL.dylib"
        lipo -create "$cef_no_wrapper_dir/x86_64/Release/Chromium Embedded Framework.framework/Libraries/libGLESv2.dylib" "$cef_no_wrapper_dir/arm64/Release/Chromium Embedded Framework.framework/Libraries/libGLESv2.dylib" -output "$stage/lib/release/Chromium Embedded Framework.framework/Libraries/libGLESv2.dylib"
        lipo -create "$cef_no_wrapper_dir/x86_64/Release/Chromium Embedded Framework.framework/Libraries/libvk_swiftshader.dylib" "$cef_no_wrapper_dir/arm64/Release/Chromium Embedded Framework.framework/Libraries/libvk_swiftshader.dylib" -output "$stage/lib/release/Chromium Embedded Framework.framework/Libraries/libvk_swiftshader.dylib"

        # Set up snapshot blobs for universal
        cp -R "$cef_no_wrapper_dir/arm64/Release/Chromium Embedded Framework.framework/Resources/v8_context_snapshot.arm64.bin" "$stage/lib/release/Chromium Embedded Framework.framework/Resources/"

        cp "$top/CEF_LICENSE.txt" "$stage/LICENSES"
        cp "$top/LICENSE.txt" "$stage/LICENSES"

        # sign the binaries (both CEF and DullahanHelper)
        CONFIG_FILE="${build_secrets_checkout:-<no build_secrets_checkout>}/code-signing-osx/config.sh"
        if [[ -n "${build_secrets_checkout:-}" && -f "$CONFIG_FILE" ]]; then
            source $CONFIG_FILE

            pushd "$stage/lib/release/Chromium Embedded Framework.framework/Libraries"
            for dylib in lib*.dylib;
            do
                if [ -f "$dylib" ]; then
                    codesign --force --timestamp --options runtime --entitlements "$dullahan_source_dir/dullahan.entitlements" --sign "$APPLE_SIGNATURE" "$dylib"
                fi
            done
            codesign --force --timestamp --options runtime --entitlements "$dullahan_source_dir/dullahan.entitlements" --sign "$APPLE_SIGNATURE" "../Chromium Embedded Framework"
            popd

            pushd "$stage/lib/release/"
            for app in DullahanHelper*.app;
            do
                if [ -d "$app" ]; then
                    sed -i "" "s/DullahanHelper/${app%.*}/" "$app/Contents/Info.plist"
                    codesign --force --timestamp --options runtime --entitlements "$dullahan_source_dir/dullahan.entitlements" --sign "$APPLE_SIGNATURE" "$app"
                fi
            done
            popd
        else
            echo "No config file $CONFIG_FILE found; skipping codesign."
        fi

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

        cmake -S . -B stage/build  -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_INSTALL_PREFIX=stage -DENABLE_CXX11_ABI=ON \
        -DUSE_SPOTIFY_CEF=TRUE -DSPOTIFY_CEF_URL=https://cef-builds.spotifycdn.com/cef_binary_118.4.1%2Bg3dd6078%2Bchromium-118.0.5993.54_linux64_beta_minimal.tar.bz2
        cmake --build stage/build
        cmake --install stage/build

        strip stage/lib/release/libcef.so
        rm stage/bin/release/*.so*
        rm stage/bin/release/*.json

        g++ -std=c++17  -I "${cef_no_wrapper_dir}/include"  -I "${dullahan_source_dir}" -o "$stage/version"  "$top/tools/autobuild_version.cpp"

        "$stage/version" > "$stage/VERSION.txt"
        rm "$stage/version"

        mkdir -p "$stage/LICENSES"
        mkdir -p "$stage/include/cef"

        cp ${dullahan_source_dir}/dullahan.h ${stage}/include/cef/
        cp ${dullahan_source_dir}/dullahan_version.h ${stage}/include/cef/
        cp "$top/CEF_LICENSE.txt" "$stage/LICENSES"
        cp "$top/LICENSE.txt" "$stage/LICENSES"
        ;;
esac
