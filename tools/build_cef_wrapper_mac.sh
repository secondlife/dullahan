#!/usr/bin/env bash

exec 4>&1; export BASH_XTRACEFD=4; set -x

# CHANGE this to the version of CEF you want to build
CEF_VERSION="3626.1895.g7001d56"

# CHANGE this to point to the folder where you want to build CEF
DST_DIR="$HOME/work/cef_builds/"

# probably don't want to change anything from now on
CEF_BUILD="cef_binary_3.${CEF_VERSION}_macosx64"
SRC_DIR="/tmp"

pushd .
cd ${SRC_DIR}

# there is a script that pulls the CEF builds from Spotify over to a Linden
# owned S3 bucket on a regular basis - just in case Spotify ever goes away
curl -O "https://secondlife-cef-builds.s3.amazonaws.com/${CEF_BUILD}.tar.bz2"

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
mkdir -p "${DST_DIR}/${CEF_BUILD}/bin/Debug/Chromium Embedded Framework.framework"
cp -R "${SRC_DIR}/${CEF_BUILD}/Debug/Chromium Embedded Framework.framework" "${DST_DIR}/${CEF_BUILD}/bin/Debug/"
mkdir -p "${DST_DIR}/${CEF_BUILD}/bin/Release/Chromium Embedded Framework.framework"
cp -R "${SRC_DIR}/${CEF_BUILD}/Release/Chromium Embedded Framework.framework" "${DST_DIR}/${CEF_BUILD}/bin/Release/"

# copy headers
mkdir -p "${DST_DIR}/${CEF_BUILD}/include"
cp -R "${SRC_DIR}/${CEF_BUILD}/include" "${DST_DIR}/${CEF_BUILD}"

# copy libcef_dll library
mkdir -p "${DST_DIR}/${CEF_BUILD}/lib/Debug"
cp "${SRC_DIR}/${CEF_BUILD}/build64/libcef_dll_wrapper/Debug/libcef_dll_wrapper.a" "${DST_DIR}/${CEF_BUILD}/lib/Debug"
mkdir -p "${DST_DIR}/${CEF_BUILD}/lib/Release"
cp "${SRC_DIR}/${CEF_BUILD}/build64/libcef_dll_wrapper/Release/libcef_dll_wrapper.a" "${DST_DIR}/${CEF_BUILD}/lib/Release"

popd
