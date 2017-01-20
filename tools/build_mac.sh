#!/usr/bin/env bash

# todo update info.plist with real info
# make a real menu.nib
# wtf is osx helper ?
# use symlink for second framework

install_name_tool -id "@executable_path/../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework" ~/Work/cef_builds/cef_bin-3.2704-mac64/bin/release/Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework

rm -rf ./build
mkdir build
cd build
cmake -G "Xcode" cmake -DCMAKE_OSX_ARCHITECTURES="x86_64" -DCEF_DIR="~/work/cef_builds/cef_bin-3.2704-mac64" ..

xcodebuild -project dullahan.xcodeproj -target dullahan -configuration 'Release'
xcodebuild -project dullahan.xcodeproj -target DullahanHelper -configuration 'Release'
xcodebuild -project dullahan.xcodeproj -target osxgl -configuration 'Release'

mkdir Release/osxgl.app/Contents/Frameworks
cp -r ~/Work/cef_builds/cef_bin-3.2704-mac64/bin/release/Chromium\ Embedded\ Framework.framework Release/osxgl.app/Contents/Frameworks/Chromium\ Embedded\ Framework.framework

cp -r Release/DullahanHelper.app Release/osxgl.app/Contents/Frameworks/DullahanHelper.app

#todo replace this with a symlink
mkdir Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks
cp -r ~/Work/cef_builds/cef_bin-3.2704-mac64/bin/release/Chromium\ Embedded\ Framework.framework Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks/Chromium\ Embedded\ Framework.framework

# todo generate from cmake
cp ~/Desktop/Info.plist Release/osxgl.app/Contents
cp ~/Desktop/Info.plist Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents

# todo generate if possible
mkdir Release/osxgl.app/Contents/Resources
cp -r ~/Desktop/Resources/* Release/osxgl.app/Contents/Resources
