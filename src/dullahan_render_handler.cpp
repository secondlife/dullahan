/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    @author Callum Prentice - September 2016

    LICENSE FILE TO GO HERE
*/

#include "dullahan_render_handler.h"

#include "dullahan_impl.h"
#include "dullahan_callback_manager.h"

dullahan_render_handler::dullahan_render_handler(dullahan_impl* parent) :
    mParent(parent)
{
}

// CefRenderHandler override
bool dullahan_render_handler::GetViewRect(CefRefPtr<CefBrowser> browser,
        CefRect& rect)
{
    int width, height;
    mParent->getSize(width, height);

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

    mParent->getCallbackManager()->onPageChanged((unsigned char*)(buffer), x, y,
            width, height, is_popup);
}

// CefRenderHandler override
void dullahan_render_handler::OnCursorChange(CefRefPtr<CefBrowser> browser,
        CefCursorHandle cursor,
        CursorType type, const CefCursorInfo& custom_cursor_info)
{
    DLNOUT("OnCursorChange called cursor: " << cursor << " and type " << type);

    mParent->getCallbackManager()->onCursorChanged((dullahan::ECursorType)type,
            (unsigned int)cursor);
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
