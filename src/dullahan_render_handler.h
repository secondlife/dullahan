/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework
    @author Callum Prentice 2015

    LICENSE FILE TO GO HERE
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
        CefRect mPopupRect;
        dullahan_impl* mParent;
};

#endif // _DULLAHAN_RENDER_HANDLER
