#!/usr/bin/env bash

exec 4>&1; export BASH_XTRACEFD=4; set -x

# The URL of the Linden Lab autobuild 3p-cef package we want to extract from
CEF_ARCHIVE="http://automated-builds-secondlife-com.s3.amazonaws.com/ct2/55931/524495/cef-80.1.6_g919206c_chromium-80.0.3987.149.539660-darwin64-539660.tar.bz2"
CEF_BUILD="80.1.6"

DST_DIR="$(pwd)/cef_builds/"
SRC_DIR="/tmp"
CEF_DIR="cef_${CEF_BUILD}"

mkdir -p "${SRC_DIR}/${CEF_DIR}"
mkdir -p "${DST_DIR}/${CEF_DIR}"

pushd .

cd ${SRC_DIR}

curl "$CEF_ARCHIVE" -o "${CEF_DIR}.tar.bz2"

tar -xvf "${CEF_DIR}.tar.bz2" --directory "${CEF_DIR}"

cp -R "${SRC_DIR}/${CEF_DIR}/" "${DST_DIR}/${CEF_DIR}"

popd
