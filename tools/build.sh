#!/usr/bin/env bash

# extra tracing
exec 4>&1; export BASH_XTRACEFD=4; set -x

# check command line arguments
if [ $# != 3 ]
then
    {
        echo -e "\nUsage:"
        echo -e "    $0 U <CEF package URL> <build directory>"
        echo -e "        E.G. $0 U http://opensource.spotify.com/cefbuilds/cef_binary_81.3.1%2Bgb2b49f1%2Bchromium-81.0.4044.113_macosx64.tar.bz2 ./build81"
        echo -e "    $0 F <CEF folder> <build directory>"
        echo -e "        E.G. $0 F /Users/foo/cef/81 ./build_local_81"
    } 2> /dev/null
    exit 1
fi

# For URL builds, the first parameter is the URL of the CEF unwrapped package otherwise, 
# for Folder builds, the first parameter is the folder containing the uncompressed raw CEF
# package that you downloaded or build yourself
if [ "$1" == "U" ]; then
    cef_bundle_url_darwin64=$2

    # confirm URL exists and bail out if not
    if ! curl --head --silent --fail "$cef_bundle_url_darwin64" 2> /dev/null; then
        echo "The CEF URL \""$cef_bundle_url_darwin64"\" does not exist."
        exit 1
    fi
elif [ "$1" == "F" ]; then
    cef_bundle_dir_darwin64=$2
    
    # confirm the source CEF folder exists
    if [ ! -d "$cef_bundle_dir_darwin64" ]; then
        echo "Selected CEF source directory does not exist"
        exit 1
    fi
else
    echo "First parameter is U (URL builds) or F (Folder build)"
    exit 1
fi

# The third command line parameter (in both URL and Folder builds) is the name of the 
# top level build directory where the CEF wrapper and then Dullahan will be built E.G. build
build_dir=$3

# collect locations and names where we build stuff
top="$(pwd)"
top_build_dir="$top/$build_dir"
cef_dir="$top_build_dir/cef"
cef_build_dir="$top_build_dir/cef/build"
cef_pkg="$cef_dir/cef_pkg.tar.bz2"

# we stipulate that the parent build dir must be empty - too hard
# to wrangle caching etc. otherwise and it doesn't take long to build
if [ -d "$top_build_dir" ];
then
    echo "Selected build directory already exists - remove it first"
    exit 1
fi

# Either download and uncompress the CEF source package for URL builds
# or copy the files for a Folder build - by this point it is guaranteed
# to be one or the other so no need to check for invalid first parameter
if [ "$1" == "U" ]; then
    # download the raw, unwrapped CEF package
    curl "$cef_bundle_url_darwin64" -o "$cef_pkg" --create-dirs

    # uncompress it and remove the top level directory because we
    # cannot (easily) tell its name and we don't care about it
    tar -xvf "$cef_pkg" --directory "$cef_dir" --strip-components 1
else
    # copy the files over from the source CEF folder
    mkdir -p $cef_dir
    cp -R "$cef_bundle_dir_darwin64"/. "$cef_dir"
fi

# generate Xcode project files and build both Debug and Release. We
# need both here even though typically we only use Release because the
# Dullahan CMakeLists.txt refers to both
mkdir "$cef_build_dir"
cd "$cef_build_dir"
cmake -G "Xcode" -DPROJECT_ARCH="x86_64" ..
xcodebuild -project cef.xcodeproj -target libcef_dll_wrapper -configuration 'Debug'
xcodebuild -project cef.xcodeproj -target libcef_dll_wrapper -configuration 'Release'

# now we have a wrapped version of CEF, we can build Dullahan using it
cd "$top_build_dir"
cmake -G Xcode \
    -DPROJECT_ARCH="x86_64" \
    -DCEF_WRAPPER_DIR="$cef_dir" \
    -DCEF_WRAPPER_BUILD_DIR="$cef_build_dir" \
    ..
xcodebuild -project dullahan.xcodeproj -target dullahan -configuration Release
xcodebuild -project dullahan.xcodeproj -target DullahanHelper -configuration Release
xcodebuild -project dullahan.xcodeproj -target osxgl -configuration 'Release'

# repoint where to find framework
install_name_tool -id \
    "@executable_path/../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework" \
    "$cef_dir/Release/Chromium Embedded Framework.framework/Chromium Embedded Framework"

# I don't know how to make app bundles in CMake with spaces and '()' chars in name
# so for now, I am copying base helper app bundle as needed and renaming
cp -r "$top_build_dir/Release/DullahanHelper.app" "$top_build_dir/Release/DullahanHelper (GPU).app"
mv "$top_build_dir/Release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper" "$top_build_dir/Release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper (GPU)"
cp -r "$top_build_dir/Release/DullahanHelper.app" "$top_build_dir/Release/DullahanHelper (Renderer).app"
mv "$top_build_dir/Release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper" "$top_build_dir/Release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper (Renderer)"
cp -r "$top_build_dir/Release/DullahanHelper.app" "$top_build_dir/Release/DullahanHelper (Plugin).app"
mv "$top_build_dir/Release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper" "$top_build_dir/Release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper (Plugin)"

mkdir "$top_build_dir/Release/osxgl.app/Contents/Frameworks"

# copy helper apps to the right place
cp -r "$top_build_dir/Release/DullahanHelper.app" "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper.app"
cp -r "$top_build_dir/Release/DullahanHelper (GPU).app" "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (GPU).app"
cp -r "$top_build_dir/Release/DullahanHelper (Renderer).app" "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (Renderer).app"
cp -r "$top_build_dir/Release/DullahanHelper (Plugin).app" "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (Plugin).app"

# copy framework to right place
cp -r "$cef_dir/Release/Chromium Embedded Framework.framework" \
      "$top_build_dir/Release/osxgl.app/Contents/Frameworks/Chromium Embedded Framework.framework"

# all the helper apps needs the framework too so make a symbolic link to existing one
pushd .
mkdir "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks"
cd "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks"
ln -s "../../../../Frameworks/Chromium Embedded Framework.framework" "Chromium Embedded Framework.framework"
popd

pushd .
mkdir "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (GPU).app/Contents/Frameworks"
cd "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (GPU).app/Contents/Frameworks"
ln -s "../../../../Frameworks/Chromium Embedded Framework.framework" "Chromium Embedded Framework.framework"
popd

pushd .
mkdir "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (Renderer).app/Contents/Frameworks"
cd "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (Renderer).app/Contents/Frameworks"
ln -s "../../../../Frameworks/Chromium Embedded Framework.framework" "Chromium Embedded Framework.framework"
popd

pushd .
mkdir "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (Plugin).app/Contents/Frameworks"
cd "$top_build_dir/Release/osxgl.app/Contents/Frameworks/DullahanHelper (Plugin).app/Contents/Frameworks"
ln -s "../../../../Frameworks/Chromium Embedded Framework.framework" "Chromium Embedded Framework.framework"
popd

# copy nib file
mkdir "$top_build_dir/Release/osxgl.app/Contents/Resources"
cp -r "$top/examples/osxgl/Resources/"* "$top_build_dir/Release/osxgl.app/Contents/Resources"

echo "Dullahan build is in $top_build_dir/Release"