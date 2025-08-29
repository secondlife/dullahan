/**
 * @file   version.cpp
 * @author Callum Prentice
 * @date   2020-05-02
 * @brief  Report library version number - C++ version
 *         For a library whose version number is tracked in a C++ header
 *         file, it's more robust to build a helper program to report it than
 *         to manually parse the header file. The library might change the
 *         syntax with which it defines the version number, but we can assume
 *         it will remain valid C / C++.
 *
 * $LicenseInfo:firstyear=2014&license=internal$
 * Copyright (c) 2017, Linden Research, Inc.
 * $/LicenseInfo$
 */

#include "dullahan_version.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::cout << DULLAHAN_VERSION_MAJOR;
    std::cout << ".";
    std::cout << DULLAHAN_VERSION_MINOR;
    std::cout << ".";
    std::cout << DULLAHAN_VERSION_POINT;
    std::cout << ".";
    std::cout << DULLAHAN_VERSION_BUILD;
    std::cout << "_";
    std::cout << CEF_VERSION_MAJOR;
    std::cout << ".";
    std::cout << CEF_VERSION_MINOR;
    std::cout << ".";
    std::cout << CEF_VERSION_PATCH;

    std::cout << std::endl;

    return 0;
}
