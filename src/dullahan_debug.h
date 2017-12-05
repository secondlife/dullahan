/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework
    @author Callum Prentice 2017

    Copyright (c) 2017, Linden Research, Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#ifndef _DULLAHAN_DEBUG
#define _DULLAHAN_DEBUG

/* 
	Note: turning debug output on works well with the SysInternals tool that
	captures OutputDebugStringA from here: 
	https://docs.microsoft.com/en-us/sysinternals/downloads/debugview
*/

#define ENABLE_DEBUG_OUTPUT 1

#if (ENABLE_DEBUG_OUTPUT)
#ifdef WIN32
#include <iostream>
#include <sstream>
#define DLNOUT( x ) \
    std::ostringstream s; \
    s << "DLNOUT> " << x << std::endl; \
    std::cout << s.str(); \
    OutputDebugStringA(s.str().c_str());
#endif // WIN32
#else
#define DLNOUT( x )
#endif // ENABLE_DEBUG_OUTPUT

#endif // _DULLAHAN_DEBUG
