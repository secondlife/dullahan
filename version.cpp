/**
 * @file   version.cpp
 * @author Callum Prentice
 * @date   2017-02-13
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

#include "dullahan.h"
#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << DULLAHAN_VERSION_MAJOR;
    std::cout << ".";
    std::cout << DULLAHAN_VERSION_MINOR;
    std::cout << ".";
    std::cout << DULLAHAN_VERSION_POINT;
    std::cout << ".";
    std::cout << DULLAHAN_VERSION_BUILD;
    std::cout << "_";

    std::string cef_version = std::string(CEF_VERSION);

    // CEF version strings now (as of v75) contain '+' symbols and since we use the
    // version string to compose the name of the package and therefore the download
    // URL in TeamCity/CodeTicket, this character (not URL escaped) breaks downloads.
    // Rather than try to fix CodeTicket to URL escape the package name, we replace 
    // the bad characters with a good one that is allowed in a URL.
    std::string bad = "+";
    std::string good = "_";
    size_t pos;
    while ((pos = cef_version.find(bad)) != std::string::npos)
    {
        cef_version.replace(pos, 1, good);
    }

    std::cout << cef_version;
    std::cout << std::endl;

    return 0;
}
