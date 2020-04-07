#!/usr/bin/env bash

exec 4>&1; export BASH_XTRACEFD=4; set -x

# The URL of the third party CEF package we want to extract from and build. This can either
# be a link to the Standard Distribution on the Spotify CEF Automated Builds page here:
# http://opensource.spotify.com/cefbuilds/index.html. These builds are also mirrored into
# a Linden Lab AWS bucket for posterity and so that can also be used.
CEF_ARCHIVE="https://secondlife-cef-builds.s3.amazonaws.com/cef_binary_80.1.15%2Bg7b802c9%2Bchromium-80.0.3987.163_macosx64.tar.bz2"

# The version string you want to use (also used as the directory name for the final build result).  This
# version information used to be used to construct the URL directly but the version numbers are now so
# complex that this is infeasible - you can use any name you like here but it's sensible to base it on
# the contents of the URL - perhaps with a tag to indicate if media codecs are enabled for example.
CEF_BUILD="80.1.15.spot"

# Build in the system tmp directory so we can store and process intermediate results
DST_DIR="$(pwd)/cef_builds/"
SRC_DIR="/tmp"
CEF_DIR="cef_${CEF_BUILD}"

mkdir -p "${SRC_DIR}/${CEF_DIR}"
mkdir -p "${DST_DIR}/${CEF_DIR}"

pushd .

cd ${SRC_DIR}

# download the archive
curl "$CEF_ARCHIVE" -o "${CEF_DIR}.tar.bz2"

# extract into the named directory nad strip off the top level as these builds have
# a long descriptive top level directory we do not need and cannot interrogate
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

# copy headers (remember, the layout Dullahan expects matches the Linden 3p CEF autobuild one with an
# extra level of nexting in cef/include/include vs the standard CEF include location)
mkdir -p "${DST_DIR}/${CEF_DIR}/include/cef/include"
cp -R "${SRC_DIR}/${CEF_DIR}/include" "${DST_DIR}/${CEF_DIR}/include/cef/"

# copy libcef_dll library
mkdir -p "${DST_DIR}/${CEF_DIR}/lib/Debug"
cp "${SRC_DIR}/${CEF_DIR}/build64/libcef_dll_wrapper/Debug/libcef_dll_wrapper.a" "${DST_DIR}/${CEF_DIR}/lib/Debug"
mkdir -p "${DST_DIR}/${CEF_DIR}/lib/Release"
cp "${SRC_DIR}/${CEF_DIR}/build64/libcef_dll_wrapper/Release/libcef_dll_wrapper.a" "${DST_DIR}/${CEF_DIR}/lib/Release"

popd
