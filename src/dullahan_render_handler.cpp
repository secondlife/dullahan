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

#include "dullahan_render_handler.h"

#include "dullahan_impl.h"
#include "dullahan_callback_manager.h"

dullahan_render_handler::dullahan_render_handler(dullahan_impl* parent) :
    mParent(parent)
{
    // inidcates if we should flip the pixel buffer in Y direction
    mFlipYPixels = parent->getFlipPixelsY();

    // the pixel buffer
    mPixelBuffer = 0;
    mPixelBufferWidth = 0;
    mPixelBufferHeight = 0;
    mPixelBufferDepth = parent->getDepth();

    // a row in the pixel buffer - used as temp buffer when flipping
    mPixelBufferRow = 0;

    // the popup buffer
    mPopupBuffer = 0;
    mPopupBufferWidth = 0;
    mPopupBufferHeight = 0;
    mPopupBufferDepth = parent->getDepth();
    mPopupBufferDrawn = false;
}

dullahan_render_handler::~dullahan_render_handler()
{
    delete[] mPixelBuffer;

    delete[] mPopupBuffer;

    delete[] mPixelBufferRow;
}

void dullahan_render_handler::resizePixelBuffer(int width, int height)
{
    if (mPixelBufferWidth != width || mPixelBufferHeight != height)
    {
        delete[] mPixelBuffer;
        mPixelBufferWidth = width;
        mPixelBufferHeight = height;
        mPixelBuffer = new unsigned char[mPixelBufferWidth * mPixelBufferHeight * mPixelBufferDepth];
        memset(mPixelBuffer, 0, mPixelBufferWidth * mPixelBufferHeight * mPixelBufferDepth);

        delete[] mPixelBufferRow;
        mPixelBufferRow = new unsigned char[mPixelBufferWidth * mPixelBufferDepth];
    }
}

void dullahan_render_handler::resizePopupBuffer(int width, int height)
{
    if (mPopupBufferWidth != width || mPopupBufferHeight != height)
    {
        delete[] mPopupBuffer;
        mPopupBufferWidth = width;
        mPopupBufferHeight = height;
        mPopupBuffer = new unsigned char[mPopupBufferWidth * mPopupBufferHeight * mPopupBufferDepth];
    }
}

void dullahan_render_handler::destroyPopupBuffer()
{
    delete[] mPopupBuffer;

    mPopupBuffer = 0;
    mPopupBufferWidth = 0;
    mPopupBufferHeight = 0;
    mPopupBufferDrawn = false;
}

// CefRenderHandler override
bool dullahan_render_handler::GetViewRect(CefRefPtr<CefBrowser> browser,
        CefRect& rect)
{
    int width, height;
    mParent->getSize(width, height);

    resizePixelBuffer(width, height);

    rect = CefRect(0, 0, width, height);

    return true;
}

// CefRenderHandler override
void dullahan_render_handler::OnPaint(CefRefPtr<CefBrowser> browser,
                                      PaintElementType type, const RectList& dirtyRects,
                                      const void* buffer, int width, int height)
{
    DLNOUT("onPaint called for size: " << width << " x " << height << " with type: " << type);

    // popup (dropdown menu) wants to be drawn
    if (type == PET_POPUP)
    {
        // copy the pixels into the popup buffer (guaranteed to have been created in OnPopupShow()
        // and mark the pixels as drawn so we don't copy in empty rectangle
        memcpy(mPopupBuffer, buffer, mPopupBufferWidth * mPopupBufferHeight * mPopupBufferDepth);
        mPopupBufferDrawn = true;
    }
    // whole page wants to be drawn
    else if (type == PET_VIEW)
    {
        // OnPaint is drawing whole page but  we still have a popup/dropdown open so
        // invalidate it which will cause a further paint call later on
        if (mPopupBufferRect.width > 0 && mPopupBufferRect.height > 0)
        {
            browser->GetHost()->Invalidate(PET_POPUP);
        }

        // resize the buffer for the whole page and save the pixels
        resizePixelBuffer(width, height);
        memcpy(mPixelBuffer, (unsigned char*)buffer, mPixelBufferWidth * mPixelBufferHeight * mPixelBufferDepth);

        // we need to flip pixel buffer in Y direction as per settings
        if (mFlipYPixels)
        {
            // fast Y flip
            const size_t stride = mPixelBufferWidth * mPixelBufferDepth;
            unsigned char* lower = mPixelBuffer;
            unsigned char* upper = mPixelBuffer + (mPixelBufferHeight - 1) * stride;
            while (lower < upper)
            {
                memcpy(mPixelBufferRow, lower, stride);
                memcpy(lower, upper, stride);
                memcpy(upper, mPixelBufferRow, stride);
                lower += stride;
                upper -= stride;
            }
        }
    }

    // if something was drawn to the popup/dropdown
    // (i.e.) it's not a black, empty buffer which will look ugly
    if (mPopupBufferDrawn)
    {
        int popup_buffer_line_stride = mPopupBufferWidth * mPopupBufferDepth;

        // if we are flipping Y pixels, we need to take account of this in position
        // of the popup/dropdown
        int y_position = mPopupBufferRect.y;
        if (mFlipYPixels)
        {
            y_position = mPixelBufferHeight - mPopupBufferRect.y - mPopupBufferHeight;
        }

        // the location in the pixel buffer where we start copying
        int offset = y_position * mPixelBufferWidth * mPixelBufferDepth + mPopupBufferRect.x * mPopupBufferDepth;

        // check we're not going to overwrite memory if popup/dropdown extends off page
        if (offset + (mPopupBufferHeight - 1) * mPixelBufferWidth * mPixelBufferDepth + popup_buffer_line_stride < mPixelBufferWidth * mPixelBufferHeight * mPixelBufferDepth)
        {
            // copy popup/dropdown into main page buffer
            for (int line = 0; line < mPopupBufferHeight; ++line)
            {
                // start from top or bottom depending of Y flip setting
                int src_line = line;
                if (mFlipYPixels)
                {
                    src_line = mPopupBufferHeight - line - 1;
                }
                int src = src_line * mPopupBufferWidth * mPopupBufferDepth;
                int dst = offset + line * mPixelBufferWidth * mPixelBufferDepth;
                memcpy(mPixelBuffer + dst, mPopupBuffer + src, popup_buffer_line_stride);
            }
        }
    }

    // if we have a buffer, indicate to consuming app that the page changed.
    if (mPixelBufferWidth > 0 && mPixelBufferHeight > 0)
    {
        mParent->getCallbackManager()->onPageChanged(mPixelBuffer, 0, 0, mPixelBufferWidth, mPixelBufferHeight);
    }
}

// CefRenderHandler override
void dullahan_render_handler::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& custom_cursor_info)
{
    DLNOUT("OnCursorChange called cursor: " << cursor << " and type " << type);
    mParent->getCallbackManager()->onCursorChanged((dullahan::ECursorType)type);
}

// CefRenderHandler override
void dullahan_render_handler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
    DLNOUT("Popup state set to " << show);
    if (!show)
    {
        mPopupBufferRect.Set(0, 0, 0, 0);
        destroyPopupBuffer();
        browser->GetHost()->Invalidate(PET_VIEW);
    };
}

// CefRenderHandler override
void dullahan_render_handler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
    DLNOUT("Popup sized to " << rect.x << ", " << rect.y << " - " << rect.width << " x " << rect.height);
    mPopupBufferRect = rect;
    resizePopupBuffer(mPopupBufferRect.width, mPopupBufferRect.height);
}
