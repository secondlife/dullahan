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
        void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
        void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                     const RectList& dirtyRects,
                     const void* buffer, int width, int height) override;
        void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
        void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
        bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;

        IMPLEMENT_REFCOUNTING(dullahan_render_handler);

    private:
        void resizePixelBuffer(int width, int height);
        void copyPopupIntoView();

        unsigned char* mPixelBuffer;
        int mPixelBufferWidth;
        int mPixelBufferHeight;
        unsigned char* mPopupBuffer;
        unsigned char* mPixelBufferRow;
        CefRect mPopupBufferRect;
        int mBufferDepth;

        bool mFlipYPixels;

        dullahan_impl* mParent;
};

#endif // _DULLAHAN_RENDER_HANDLER
