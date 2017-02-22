/*
@brief Dullahan - a headless browser rendering engine
based around the Chromium Embedded Framework

@author Callum Prentice - September 2016

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

#include "dullahan_render_handler.h"

#include "dullahan_impl.h"
#include "dullahan_callback_manager.h"

dullahan_render_handler::dullahan_render_handler(dullahan_impl* parent) :
    mParent(parent)
{
    mFlipYPixels = parent->getFlipPixelsY();

    if (mFlipYPixels)
    {
        mFlipBufferWidth = 0;
        mFlipBufferHeight = 0;
        mFlipBufferDepth = parent->getDepth();
        mFlipBuffer = 0;
    }
}

dullahan_render_handler::~dullahan_render_handler()
{
    if (mFlipYPixels)
    {
        delete[] mFlipBuffer;
    }
}

void dullahan_render_handler::resizeFlipBuffer(int width, int height)
{
    if (mFlipYPixels)
    {
        if (mFlipBufferWidth != width || mFlipBufferHeight != height)
        {
            delete[] mFlipBuffer;
            mFlipBufferWidth = width;
            mFlipBufferHeight = height;
            mFlipBuffer = new unsigned char[mFlipBufferWidth * mFlipBufferHeight * mFlipBufferDepth];
            memset(mFlipBuffer, 0, mFlipBufferWidth * mFlipBufferHeight * mFlipBufferDepth);
        }
    }
}

// CefRenderHandler override
bool dullahan_render_handler::GetViewRect(CefRefPtr<CefBrowser> browser,
        CefRect& rect)
{
    int width, height;
    mParent->getSize(width, height);

    if (mFlipYPixels)
    {
        resizeFlipBuffer(width, height);
    }

    rect = CefRect(0, 0, width, height);

    return true;
}

// CefRenderHandler override
void dullahan_render_handler::OnPaint(CefRefPtr<CefBrowser> browser,
                                      PaintElementType type, const RectList& dirtyRects,
                                      const void* buffer, int width, int height)
{
    DLNOUT("onPaint called for size: " << width << " x " << height << " with type: " << type);

    int x = 0;
    int y = 0;
    bool is_popup = type == PET_POPUP ? true : false;

    if (type == PET_POPUP)
    {
        x = mPopupRect.x;
        y = mPopupRect.y;
    }

    if (mFlipYPixels)
    {
        resizeFlipBuffer(width, height);

        for (int y_line = 0; y_line < height; ++y_line)
        {
            memcpy(mFlipBuffer + y_line * width * mFlipBufferDepth, (unsigned char*)buffer + (height - y_line - 1) * width * mFlipBufferDepth, width * mFlipBufferDepth);
        }

        mParent->getCallbackManager()->onPageChanged(mFlipBuffer, x, y, width, height, is_popup);
    }
    else
    {
        mParent->getCallbackManager()->onPageChanged((unsigned char*)(buffer), x, y, width, height, is_popup);
    }
}

// CefRenderHandler override
void dullahan_render_handler::OnCursorChange(CefRefPtr<CefBrowser> browser,
        CefCursorHandle cursor,
        CursorType type, const CefCursorInfo& custom_cursor_info)
{
    DLNOUT("OnCursorChange called cursor: " << cursor << " and type " << type);

    mParent->getCallbackManager()->onCursorChanged((dullahan::ECursorType)type, (unsigned int)cursor);
}

// CefRenderHandler override
void dullahan_render_handler::OnPopupShow(CefRefPtr<CefBrowser> browser,
        bool show)
{
    DLNOUT("Popup show state changed to " << show);
    if (!show)
    {
        mPopupRect.Set(0, 0, 0, 0);

        mParent->getBrowser()->GetHost()->Invalidate(PET_VIEW);
    }
}

// CefRenderHandler override
void dullahan_render_handler::OnPopupSize(CefRefPtr<CefBrowser> browser,
        const CefRect& rect)
{
    DLNOUT("Popup sized to " << rect.x << ", " << rect.y << " - " << rect.width <<
           " x " << rect.height);
    setPopupLocation(rect);
}

void dullahan_render_handler::setPopupLocation(const CefRect& rect)
{
    mPopupRect = rect;
}

const CefRect& dullahan_render_handler::getPopupLocation()
{
    return mPopupRect;
}
