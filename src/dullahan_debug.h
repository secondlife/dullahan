/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    @author Callum Prentice - September 2016

    LICENSE FILE TO GO HERE
*/

#ifndef _DULLAHAN_DEBUG
#define _DULLAHAN_DEBUG

#define ENABLE_DEBUG_OUTPUT 0

#if (ENABLE_DEBUG_OUTPUT)
#include <iostream>
#include <sstream>

#define DLNOUT( x ) \
    std::ostringstream s; \
    s << "DLNOUT> " << x << std::endl; \
    std::cout  << s.str(); \
    OutputDebugStringA(s.str().c_str());
#else
#define DLNOUT( x )
#endif

#endif // _DULLAHAN_DEBUG
