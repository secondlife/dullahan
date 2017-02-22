/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework
    @author Callum Prentice 2015

    Copyright (c) 2016, Linden Research, Inc.

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

#ifndef _DULLAHAN_RENDER_HANDLER
#define _DULLAHAN_RENDER_HANDLER

#include "cef_render_handler.h"

class dullahan_impl;

class dullahan_render_handler :
    public CefRenderHandler
{
    public:
        dullahan_render_handler(dullahan_impl* parent);
        ~dullahan_render_handler();

        // CefRenderHandler interface
        bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;
        void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                     const RectList& dirtyRects,
                     const void* buffer, int width, int height) OVERRIDE;
        void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor,
                            CursorType type,
                            const CefCursorInfo& custom_cursor_info) OVERRIDE;
        void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;
        void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) OVERRIDE;

        // utility functions
        void setPopupLocation(const CefRect& rect);
        const CefRect& getPopupLocation();

        IMPLEMENT_REFCOUNTING(dullahan_render_handler);

    private:
        void resizeFlipBuffer(int width, int height);

        CefRect mPopupRect;
        bool mFlipYPixels;
        unsigned char* mFlipBuffer;
        int mFlipBufferWidth;
        int mFlipBufferHeight;
        int mFlipBufferDepth;
        dullahan_impl* mParent;
};

#endif // _DULLAHAN_RENDER_HANDLER
