#!/usr/bin/env bash



# TODO
# fix up install_name
# copy helpers apps around




# extra tracing
exec 4>&1; export BASH_XTRACEFD=4; set -x

# check command line arguments
if [ $# != 2 ]
then
    {
        echo -e "\nUsage:"
        echo -e "    $0 <CEF package URL> <build directory>"
    } 2> /dev/null
    exit 1
fi

# The first command line parameter is the URL of the CEF unwrapped package
# E.G. http://opensource.spotify.com/cefbuilds/cef_binary_81.3.1%2Bgb2b49f1%2Bchromium-81.0.4044.113_macosx64.tar.bz2
cef_bundle_url_darwin64=$1

# confirm URL exists and bail out if not
if ! curl --head --silent --fail "$cef_bundle_url_darwin64" 2> /dev/null;
    then
        echo "The CEF URL \""$cef_bundle_url_darwin64"\" does not exist."
        exit 1
fi

# The second command line parameter is the name of the top level build directory
# where the CEF wrapper and then Dullahan will be built E.G. build
build_dir=$2

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

# download the raw, unwrapped CEF package
curl "$cef_bundle_url_darwin64" -o "$cef_pkg" --create-dirs

# uncompress it and remove the top level directory because we
# cannot (easily) tell its name and we don't care about it
tar -xvf "$cef_pkg" --directory "$cef_dir" --strip-components 1

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
