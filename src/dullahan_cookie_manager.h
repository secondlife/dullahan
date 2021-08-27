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

#ifndef _DULLAHAN_COOKIE_MANAGER
#define _DULLAHAN_COOKIE_MANAGER

#include "dullahan.h"

#include <atomic>

class dullahan_cookie_manager
{
    public:

        /*
            Initialize the cookie manager.
            Must be called before anything else.
            Returns true/false for success/failure.
         */
        bool init();

        /*
            Set a cookie given these parameters.
            Also flushes cookie store after the set, just to be sure!
            Returns true/false for success/failure.
         */
        bool setCookie(const std::string url,
                       const std::string name, const std::string value,
                       const std::string domain, const std::string path,
                       bool httponly, bool secure);

        /*
            Retrieve a list of cookies as a name/value cookie pair.
            Returns the list of name/value pairs which is empty on failure.
         */
        const dullahan::dullahan_cookie_list_t getCookies();

        /*
            Flush the cookie store - consider calling before your app exits.
            The flag wait_for_complete should almost always be true to indicate
            that it should wait for the callback to finish before returning.
            This function is used internally too where we do not want to invoke
            the callback. TODO: Investigate why bad things happen when you do.
            Returns true/false for success/failure.
         */
        bool flushCookies(bool wait_for_complete);

        /*
            Deletes all the cookies.
            Returns true/false for success/failure.
         */
        bool deleteCookies();

    private:
        CefRefPtr<CefCookieManager> mCookieManager;
};

#endif //_DULLAHAN_COOKIE_MANAGER
