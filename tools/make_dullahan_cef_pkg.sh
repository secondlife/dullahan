#!/usr/bin/env bash

exec 4>&1; export BASH_XTRACEFD=4; set -x

CEF_BUILD="cef_binary_3.3239.1723.g071d1c1_macosx64"

SRC_DIR="/tmp/${CEF_BUILD}"

# change this to point to where you want to build the CEF package
DST_DIR="/Users/callum/Desktop/${CEF_BUILD}"

pushd .
cd /tmp
curl -O "http://opensource.spotify.com/cefbuilds/${CEF_BUILD}.tar.bz2"

tar -xvf "${CEF_BUILD}.tar.bz2"

cd ${CEF_BUILD}

# generate project files
mkdir build64
cd build64
cmake -G "Xcode" -DPROJECT_ARCH="x86_64" ..

# build release and debug configurations
xcodebuild -project cef.xcodeproj -target libcef_dll_wrapper -configuration 'Debug'
xcodebuild -project cef.xcodeproj -target libcef_dll_wrapper -configuration 'Release'

# copy frameworks
mkdir -p "${DST_DIR}/bin/Debug/Chromium Embedded Framework.framework"
cp -R "${SRC_DIR}/Debug/Chromium Embedded Framework.framework" "${DST_DIR}/bin/Debug/"
mkdir -p "${DST_DIR}/bin/Release/Chromium Embedded Framework.framework"
cp -R "${SRC_DIR}/Release/Chromium Embedded Framework.framework" "${DST_DIR}/bin/Release/"

# copy headers
mkdir -p "${DST_DIR}/include"
cp -R "${SRC_DIR}/include" "${DST_DIR}"

# copy libcef_dll library
mkdir -p "${DST_DIR}/lib/Debug"
cp "${SRC_DIR}/build64/libcef_dll_wrapper/Debug/libcef_dll_wrapper.a" "${DST_DIR}/lib/Debug"
mkdir -p "${DST_DIR}/lib/Release"
cp "${SRC_DIR}/build64/libcef_dll_wrapper/Release/libcef_dll_wrapper.a" "${DST_DIR}/lib/Release"

popd

