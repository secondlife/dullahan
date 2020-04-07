#!/usr/bin/env bash

exec 4>&1; export BASH_XTRACEFD=4; set -x

# The URL of the CEF package we want to build the CEF wrapper for
#
#  Explain how to get here
#
CEF_ARCHIVE="https://secondlife-cef-builds.s3.amazonaws.com/cef_binary_80.1.15%2Bg7b802c9%2Bchromium-80.0.3987.163_macosx64.tar.bz2"
CEF_BUILD="80.1.15.spot"

DST_DIR="$(pwd)/cef_builds/"
SRC_DIR="/tmp"
CEF_DIR="cef_${CEF_BUILD}"

mkdir -p "${SRC_DIR}/${CEF_DIR}"
mkdir -p "${DST_DIR}/${CEF_DIR}"

pushd .

cd ${SRC_DIR}

curl "$CEF_ARCHIVE" -o "${CEF_DIR}.tar.bz2"

tar -xvf "${CEF_DIR}.tar.bz2" --directory "${CEF_DIR}" --strip 1

cd "${CEF_DIR}"

# generate project files
mkdir build64
cd build64
cmake -G "Xcode" -DPROJECT_ARCH="x86_64" ..

# build release and debug configurations
xcodebuild -project cef.xcodeproj -target libcef_dll_wrapper -configuration 'Debug'
xcodebuild -project cef.xcodeproj -target libcef_dll_wrapper -configuration 'Release'

# copy frameworks
mkdir -p "${DST_DIR}/${CEF_DIR}/bin/Debug/Chromium Embedded Framework.framework"
cp -R "${SRC_DIR}/${CEF_DIR}/Debug/Chromium Embedded Framework.framework" "${DST_DIR}/${CEF_DIR}/bin/Debug/"
mkdir -p "${DST_DIR}/${CEF_DIR}/bin/Release/Chromium Embedded Framework.framework"
cp -R "${SRC_DIR}/${CEF_DIR}/Release/Chromium Embedded Framework.framework" "${DST_DIR}/${CEF_DIR}/bin/Release/"

# copy headers
mkdir -p "${DST_DIR}/${CEF_DIR}/include/cef/include"
cp -R "${SRC_DIR}/${CEF_DIR}/include" "${DST_DIR}/${CEF_DIR}/include/cef/"

# copy libcef_dll library
mkdir -p "${DST_DIR}/${CEF_DIR}/lib/Debug"
cp "${SRC_DIR}/${CEF_DIR}/build64/libcef_dll_wrapper/Debug/libcef_dll_wrapper.a" "${DST_DIR}/${CEF_DIR}/lib/Debug"
mkdir -p "${DST_DIR}/${CEF_DIR}/lib/Release"
cp "${SRC_DIR}/${CEF_DIR}/build64/libcef_dll_wrapper/Release/libcef_dll_wrapper.a" "${DST_DIR}/${CEF_DIR}/lib/Release"

popd
