#!/usr/bin/env bash

exec 4>&1; export BASH_XTRACEFD=4; set -x

# The URL of the Linden Lab autobuild 3p-cef package we want to extract from
CEF_ARCHIVE="http://automated-builds-secondlife-com.s3.amazonaws.com/ct2/55931/524495/cef-80.1.6_g919206c_chromium-80.0.3987.149.539660-darwin64-539660.tar.bz2"

# The version string you want to use (also used as the directory name for the final build result).  This
# version information used to be used to construct the URL directly but the version numbers are now so
# complex that this is infeasible - you can use any name you like here but it's sensible to base it on
# the contents of the URL - perhaps with a tag to indicate if media codecs are enabled for example.
CEF_BUILD="80.1.6"

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

# extract into the named directory
tar -xvf "${CEF_DIR}.tar.bz2" --directory "${CEF_DIR}"

# copy the CEF prebuilt directory tree over to the CEF builds folder we specified
cp -R "${SRC_DIR}/${CEF_DIR}/" "${DST_DIR}/${CEF_DIR}"

popd
