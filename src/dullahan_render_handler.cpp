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

#define NOMINMAX

#include "wrapper/cef_helpers.h"

#include "dullahan_render_handler.h"

#include "dullahan_impl.h"
#include "dullahan_callback_manager.h"

#ifdef WIN32
#include "dullahan_shared_texture_flipper.h"
#endif


dullahan_render_handler::dullahan_render_handler(dullahan_impl* parent) :
    mParent(parent)
{
    // inidcates if we should flip the pixel buffer in Y direction
    mFlipYPixels = parent->getFlipPixelsY();

    // the pixel buffer
    mPixelBuffer = nullptr;
    mPixelBufferWidth = 0;
    mPixelBufferHeight = 0;

    // the popup buffer
    mPopupBuffer = nullptr;

    // a row in the pixel buffer - used as temp buffer when flipping
    mPixelBufferRow = nullptr;

    // depth is same for all buffer
    mBufferDepth = parent->getDepth();

#ifdef WIN32
    // created on demand in OnAcceleratedPaint (only when flipping is enabled)
    mSharedTextureFlipper = nullptr;
#endif
}

dullahan_render_handler::~dullahan_render_handler()
{
    delete[] mPixelBuffer;

    delete[] mPopupBuffer;

    delete[] mPixelBufferRow;

#ifdef WIN32
    delete mSharedTextureFlipper;
#endif
}

void dullahan_render_handler::resizePixelBuffer(int width, int height)
{
    if (mPixelBufferWidth != width || mPixelBufferHeight != height)
    {
        delete[] mPixelBuffer;
        mPixelBufferWidth = width;
        mPixelBufferHeight = height;
        mPixelBuffer = new unsigned char[mPixelBufferWidth * mPixelBufferHeight * mBufferDepth];
        memset(mPixelBuffer, 0xff, mPixelBufferWidth * mPixelBufferHeight * mBufferDepth);

        delete[] mPixelBufferRow;
        mPixelBufferRow = new unsigned char[mPixelBufferWidth * mBufferDepth];
    }
}

// CefRenderHandler override
void dullahan_render_handler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    CEF_REQUIRE_UI_THREAD();

    int width, height;
    mParent->getSize(width, height);

    resizePixelBuffer(width, height);

    rect = CefRect(0, 0, width, height);
}

void dullahan_render_handler::copyPopupIntoView()
{
    int popup_y = (mFlipYPixels ? (mPixelBufferHeight - mPopupBufferRect.y) : mPopupBufferRect.y);
    unsigned char* src = (unsigned char*)mPopupBuffer;
    unsigned char* dst = mPixelBuffer + popup_y * mPixelBufferWidth * mBufferDepth + mPopupBufferRect.x * mBufferDepth;
    while (src < (unsigned char*)mPopupBuffer + mPopupBufferRect.width * mPopupBufferRect.height * mBufferDepth)
    {
        memcpy(dst, src, mPopupBufferRect.width * mBufferDepth);
        src += mPopupBufferRect.width * mBufferDepth;
        dst += mPixelBufferWidth * mBufferDepth * (mFlipYPixels ? -1 : 1);
    }
}

// CefRenderHandler override
void dullahan_render_handler::OnPaint(CefRefPtr<CefBrowser> browser,
                                      PaintElementType type, const RectList& dirtyRects,
                                      const void* buffer, int width, int height)
{
    DLNOUT("onPaint called for size: " << width << " x " << height << " with type: " << type);

    CEF_REQUIRE_UI_THREAD();

    // whole page was updated
    if (type == PET_VIEW)
    {
        // create (firs time) or resize (browser size changed) a buffer for pixels
        // and copy them in
        resizePixelBuffer(width, height);
        memcpy(mPixelBuffer, buffer, width * height * mBufferDepth);

        // we need to flip pixel buffer in Y direction as per settings
        if (mFlipYPixels)
        {
            const size_t stride = mPixelBufferWidth * mBufferDepth;
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

        // if there is still a popup open, write it into the page too (it's pixels will have been
        // copied into it's buffer by a call to OnPaint with type of PET_POPUP earlier)
        if (mPopupBuffer != nullptr)
        {
            copyPopupIntoView();
        }
    }
    // popup was updated
    else if (type == PET_POPUP)
    {
        // copy over the popup pixels into it's buffer
        // (popup buffer created in onPopupSize() as we know the size there)
        memcpy(mPopupBuffer, buffer, width * height * mBufferDepth);

        // copy over popup pixels into page pixels. We need this for when popup is changing (e.g. highlighting or scrolling)
        // when the containing page is not changing and therefore doesn't get an OnPaint update
        copyPopupIntoView();
    }

    // if we have a buffer, indicate to consuming app that the page changed.
    if (mPixelBufferWidth > 0 && mPixelBufferHeight > 0)
    {
        mParent->getCallbackManager()->onPageChanged(mPixelBuffer, 0, 0, mPixelBufferWidth, mPixelBufferHeight);
    }
}

// CefRenderHandler override
void dullahan_render_handler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
    CEF_REQUIRE_UI_THREAD();

    DLNOUT("Popup state set to " << show);
    if (!show)
    {
        delete[] mPopupBuffer;
        mPopupBuffer = nullptr;

        mPopupBufferRect.Set(0,0, 0,0);
    }
}

// CefRenderHandler override
void dullahan_render_handler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
    CEF_REQUIRE_UI_THREAD();

    mPopupBufferRect = rect;
    if (mPopupBuffer == nullptr)
    {
        mPopupBuffer = new unsigned char[rect.width * rect.height * mBufferDepth];
        memset(mPopupBuffer, 0xff, rect.width * rect.height * mBufferDepth);
    }
}

// CefRenderHandler override
bool dullahan_render_handler::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
{
    // this is how the value for "color depth" in the User Agent string is populated
    screen_info.depth = mBufferDepth * 8;

    // indicate we changed the structure
    return true;
}

// CefRenderHandler override
void dullahan_render_handler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                                  PaintElementType type,
                                                  const RectList& dirtyRects,
                                                  const CefAcceleratedPaintInfo& info)
{
    CEF_REQUIRE_UI_THREAD();

    // convert CEF dirty rects to dullahan rects
    std::vector<dullahan::dullahan_rect> dullahan_dirty_rects;
    dullahan_dirty_rects.reserve(dirtyRects.size());
    for (const auto& rect : dirtyRects)
    {
        dullahan::dullahan_rect dr;
        dr.x = rect.x;
        dr.y = rect.y;
        dr.width = rect.width;
        dr.height = rect.height;
        dullahan_dirty_rects.push_back(dr);
    }

    // Dullahan for Linux does not currently support accelerated painting and will fall back to OnPaint
#ifdef WIN32
    void* handle = info.shared_texture_handle;

    // When flip_pixels_y is set we hand the consumer a bottom-up texture so it
    // no longer has to flip on its side (see the header comment on
    // setOnAcceleratedPageChangedCallback). This is a GPU blit through a device
    // we own; the device, pipeline and destination texture are created once and
    // reused across frames.
    if (mFlipYPixels)
    {
        if (!mSharedTextureFlipper)
        {
            mSharedTextureFlipper = new dullahan_shared_texture_flipper();

            bool use_luid = false;
            uint32_t luid_low = 0;
            int32_t luid_high = 0;
            mParent->getAdapterLUID(use_luid, luid_low, luid_high);

            if (!mSharedTextureFlipper->init(use_luid, luid_low, luid_high))
            {
                // Could not stand up the flip device (a hard failure). Fall back
                // to forwarding CEF's top-down handle so the consumer still gets
                // content rather than a black frame.
                DLNOUT("accelerated flip disabled - could not create flip device");
                delete mSharedTextureFlipper;
                mSharedTextureFlipper = nullptr;
            }
        }

        if (mSharedTextureFlipper)
        {
            void* flipped = mSharedTextureFlipper->flip(info.shared_texture_handle);
            if (flipped)
            {
                handle = flipped;
            }
            else
            {
                // Transient flip failure (e.g. mutex timeout). Skip this frame
                // rather than emit an unflipped (upside-down) texture.
                return;
            }
        }
    }

    mParent->getCallbackManager()->onAcceleratedPageChanged(handle, dullahan_dirty_rects);
#elif defined(__APPLE__)
    mParent->getCallbackManager()->onAcceleratedPageChanged(info.shared_texture_io_surface, dullahan_dirty_rects);
#endif
}
