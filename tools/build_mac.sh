#!/usr/bin/env bash

# CHANGE this to the version of CEF you want to use to build Dullahan
CEF_VERSION="3626.1895.g7001d56"

# CHANGE this to point to the folder where you built CEF 
# (should match the value of DST_DIR in make_dullahan_cef_pkg.sh)
CEF_BUILDS_DIR="$HOME/work/cef_builds/"

# probably don't want to change anything from now on
CEF_SRC_DIR="${CEF_BUILDS_DIR}/cef_binary_3.${CEF_VERSION}_macosx64"

# repoint where to find framework
install_name_tool -id "@executable_path/../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework" ${CEF_SRC_DIR}/bin/release/Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework

# clean build dir
rm -rf ./build64
mkdir build64
cd build64

# CMake generates Xcode project and populate dullahan.h header
cmake -G "Xcode" \
    -DCMAKE_OSX_ARCHITECTURES="x86_64" \
    -DCEF_INCLUDE_DIR="${CEF_SRC_DIR}/include/" \
    -DCEF_LIB_DIR="${CEF_SRC_DIR}/lib" \
    -DCEF_BIN_DIR="${CEF_SRC_DIR}/bin" \
    -DCEF_RESOURCE_DIR="${CEF_SRC_DIR}/resources" \
    ..

# build what we need plus examples
xcodebuild -project dullahan.xcodeproj -target dullahan -configuration 'Release'
xcodebuild -project dullahan.xcodeproj -target DullahanHelper -configuration 'Release'
xcodebuild -project dullahan.xcodeproj -target osxgl -configuration 'Release'

mkdir Release/osxgl.app/Contents/Frameworks

# copy helper app to right place
cp -r Release/DullahanHelper.app Release/osxgl.app/Contents/Frameworks/DullahanHelper.app

# copy framework to right place
cp -r ${CEF_SRC_DIR}/bin/release/Chromium\ Embedded\ Framework.framework Release/osxgl.app/Contents/Frameworks/Chromium\ Embedded\ Framework.framework

# helper app needs the framework too so make a symbolic link to existing one
pushd .
mkdir  Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks
cd Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks
ln -s '../../../../Frameworks/Chromium Embedded Framework.framework' 'Chromium Embedded Framework.framework'
popd

# copy nib file
mkdir Release/osxgl.app/Contents/Resources
cp -r ../examples/osxgl/Resources/* Release/osxgl.app/Contents/Resources
