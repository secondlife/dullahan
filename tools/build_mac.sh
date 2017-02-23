#!/usr/bin/env bash

# point this at your CEF build
# (some of the script - e.g. CMake header generation needs this form of the
# folder name vs the ~/blah one - not sure why)
cef_base_dir="/Users/callum/work/cef_builds/cef_bin-3.2704-mac64"

# repoint where to find framework
install_name_tool -id "@executable_path/../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework" ${cef_base_dir}/bin/release/Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework

# clean build dir
rm -rf ./build
mkdir build
cd build

# CMake generates Xcode project and populate dullahan.h header
cmake -G "Xcode" \
    -DCMAKE_OSX_ARCHITECTURES="x86_64" \
    -DCEF_INCLUDE_DIR="${cef_base_dir}/include" \
    -DCEF_LIB_DIR="${cef_base_dir}/lib" \
    -DCEF_BIN_DIR="${cef_base_dir}/bin" \
    -DCEF_RESOURCE_DIR="${cef_base_dir}/resources" \
    ..

# build what we need plus an example
xcodebuild -project dullahan.xcodeproj -target dullahan -configuration 'Release'
xcodebuild -project dullahan.xcodeproj -target DullahanHelper -configuration 'Release'
xcodebuild -project dullahan.xcodeproj -target osxgl -configuration 'Release'

mkdir Release/osxgl.app/Contents/Frameworks

# copy helper app to right place
cp -r Release/DullahanHelper.app Release/osxgl.app/Contents/Frameworks/DullahanHelper.app

# copy framework to right place
cp -r ${cef_base_dir}/bin/release/Chromium\ Embedded\ Framework.framework Release/osxgl.app/Contents/Frameworks/Chromium\ Embedded\ Framework.framework

# helper app needs the framework too so make a symbolic link to existing one
pushd .
mkdir  Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks
cd Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks
ln -s '../../../../Frameworks/Chromium Embedded Framework.framework' 'Chromium Embedded Framework.framework'
popd

# copy meta data
cp ../src/host/Info.plist Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents
cp ../examples/osxgl/Info.plist Release/osxgl.app/Contents

# copy nib file
mkdir Release/osxgl.app/Contents/Resources
cp -r ../examples/osxgl/Resources/* Release/osxgl.app/Contents/Resources


