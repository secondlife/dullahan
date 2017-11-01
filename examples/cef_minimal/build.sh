#!/usr/bin/env bash

cef_folder=~/work/cef_builds/cef_binary_3.3071.1637.gcb6cf75_macosx64/

if [ -f cef_host ]; then
    rm cef_host
fi

if [ -f cef_minimal ]; then
    rm cef_minimal
fi

if [ -d Frameworks ]; then
    rm -rf Frameworks
fi
mkdir Frameworks
cp -r ${cef_folder}bin/Release/Chromium\ Embedded\ Framework.framework ./Frameworks

clang++ \
    cef_host.cpp \
    -o cef_host \
    -arch x86_64 \
    -std=c++11 \
    -Wno-deprecated-declarations \
    -I ${cef_folder} \
    -I ${cef_folder}include \
    -F ${cef_folder}bin/Release/ \
    -framework Chromium\ Embedded\ Framework \
    ${cef_folder}lib/Release/libcef_dll_wrapper.a

install_name_tool -add_rpath @executable_path/ cef_host
install_name_tool -add_rpath @executable_path/Frameworks/ cef_host

clang++ \
    cef_minimal.cpp \
    -o cef_minimal \
    -arch x86_64 \
    -std=c++11 \
    -Wno-deprecated-declarations \
    -I ${cef_folder} \
    -I ${cef_folder}include \
    -F ${cef_folder}bin/Release/ \
    -framework Chromium\ Embedded\ Framework \
    ${cef_folder}lib/Release/libcef_dll_wrapper.a

install_name_tool -add_rpath @executable_path/ cef_minimal
install_name_tool -add_rpath @executable_path/Frameworks/ cef_minimal
