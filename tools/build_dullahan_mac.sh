#!/usr/bin/env bash

# make sure a path is absolute even if it starts off as relative
function abspath()
{
    pushd . > /dev/null;
    if [ -d "$1" ];
    then
        cd "$1"; dirs -l +0;
    else
        cd "`dirname \"$1\"`";
        cur_dir=`dirs -l +0`;
        if [ "$cur_dir" == "/" ];
        then 
            echo "$cur_dir`basename \"$1\"`";
        else
            echo "$cur_dir/`basename \"$1\"`";
        fi;
    fi; 
    popd > /dev/null;
}

# confirm existence of parameter that points to CEF dir
if [ -z "$1" ];
then
    echo "Usage: pass in Linden autobuild compatible CEF directory location"
    exit 1
fi

# confirm the directory exists
if [ ! -d "$1" ]; 
then
    echo "Specified CEF directory does not exist"
    exit 1
fi

# convert path to absolute one
CEF_SRC_DIR=$(abspath "$1")

# repoint where to find framework
install_name_tool -id "@executable_path/../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework" ${CEF_SRC_DIR}/bin/release/Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework

# clean build dir
rm -rf ./build64
mkdir build64
cd build64

# CMake generates Xcode project and populate dullahan.h header
cmake -G "Xcode" \
    -DCMAKE_OSX_ARCHITECTURES="x86_64" \
    -DCEF_INCLUDE_DIR="${CEF_SRC_DIR}/include/cef/include/" \
    -DCEF_LIB_DIR="${CEF_SRC_DIR}/lib" \
    -DCEF_BIN_DIR="${CEF_SRC_DIR}/bin" \
    -DCEF_RESOURCE_DIR="${CEF_SRC_DIR}/resources" \
    ..

# build what we need plus examples
xcodebuild -project dullahan.xcodeproj -target dullahan -configuration 'Release'
xcodebuild -project dullahan.xcodeproj -target DullahanHelper -configuration 'Release'
xcodebuild -project dullahan.xcodeproj -target osxgl -configuration 'Release'

# I don't know how to make app bundles in CMake with spaces and '()' chars in name
# so for now, I am copying base helper app bundle as needed and renaming 
cp -r Release/DullahanHelper.app "Release/DullahanHelper (GPU).app"
mv "Release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper" "Release/DullahanHelper (GPU).app/Contents/MacOS/DullahanHelper (GPU)"
cp -r Release/DullahanHelper.app "Release/DullahanHelper (Renderer).app"
mv "Release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper" "Release/DullahanHelper (Renderer).app/Contents/MacOS/DullahanHelper (Renderer)"
cp -r Release/DullahanHelper.app "Release/DullahanHelper (Plugin).app"
mv "Release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper" "Release/DullahanHelper (Plugin).app/Contents/MacOS/DullahanHelper (Plugin)"

mkdir Release/osxgl.app/Contents/Frameworks

# copy helper apps to the right place
cp -r Release/DullahanHelper.app Release/osxgl.app/Contents/Frameworks/DullahanHelper.app
cp -r "Release/DullahanHelper (GPU).app" "Release/osxgl.app/Contents/Frameworks/DullahanHelper (GPU).app"
cp -r "Release/DullahanHelper (Renderer).app" "Release/osxgl.app/Contents/Frameworks/DullahanHelper (Renderer).app"
cp -r "Release/DullahanHelper (Plugin).app" "Release/osxgl.app/Contents/Frameworks/DullahanHelper (Plugin).app"

# copy framework to right place
cp -r ${CEF_SRC_DIR}/bin/release/Chromium\ Embedded\ Framework.framework Release/osxgl.app/Contents/Frameworks/Chromium\ Embedded\ Framework.framework

# all the helper apps needs the framework too so make a symbolic link to existing one
pushd .
mkdir Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks
cd Release/osxgl.app/Contents/Frameworks/DullahanHelper.app/Contents/Frameworks
ln -s '../../../../Frameworks/Chromium Embedded Framework.framework' 'Chromium Embedded Framework.framework'
popd

pushd .
mkdir "Release/osxgl.app/Contents/Frameworks/DullahanHelper (GPU).app/Contents/Frameworks"
cd "Release/osxgl.app/Contents/Frameworks/DullahanHelper (GPU).app/Contents/Frameworks"
ln -s '../../../../Frameworks/Chromium Embedded Framework.framework' 'Chromium Embedded Framework.framework'
popd

pushd .
mkdir "Release/osxgl.app/Contents/Frameworks/DullahanHelper (Renderer).app/Contents/Frameworks"
cd "Release/osxgl.app/Contents/Frameworks/DullahanHelper (Renderer).app/Contents/Frameworks"
ln -s '../../../../Frameworks/Chromium Embedded Framework.framework' 'Chromium Embedded Framework.framework'
popd

pushd .
mkdir "Release/osxgl.app/Contents/Frameworks/DullahanHelper (Plugin).app/Contents/Frameworks"
cd "Release/osxgl.app/Contents/Frameworks/DullahanHelper (Plugin).app/Contents/Frameworks"
ln -s '../../../../Frameworks/Chromium Embedded Framework.framework' 'Chromium Embedded Framework.framework'
popd

# copy nib file
mkdir Release/osxgl.app/Contents/Resources
cp -r ../examples/osxgl/Resources/* Release/osxgl.app/Contents/Resources
