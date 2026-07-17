/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    Copyright (c) 2026, Linden Research, Inc.

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

// embed:// custom scheme handler
//
// Implements the embed:// scheme used to serve trusted local resources
// bundled by the host application. Files are mapped as
//
//     embed://<host>/<path>   =>   <root>/<host>/<path>
//
// Only paths listed in the registry (relative to <root>, using forward
// slashes) are served; any other request produces a 404.

#ifndef _DULLAHAN_EMBED_SCHEME
#define _DULLAHAN_EMBED_SCHEME

#include "cef_scheme.h"

class dullahan_impl;

// Factory returned to CEF via CefRegisterSchemeHandlerFactory.
// Holds a raw pointer to the owning dullahan_impl for access to
// the configured filesystem root and allowlist.
class dullahan_embed_scheme_factory : public CefSchemeHandlerFactory
{
    public:
        explicit dullahan_embed_scheme_factory(dullahan_impl* parent);

        CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                             CefRefPtr<CefFrame> frame,
                                             const CefString& scheme_name,
                                             CefRefPtr<CefRequest> request) override;

    private:
        dullahan_impl* mParent;
        IMPLEMENT_REFCOUNTING(dullahan_embed_scheme_factory);
};

#endif // _DULLAHAN_EMBED_SCHEME
