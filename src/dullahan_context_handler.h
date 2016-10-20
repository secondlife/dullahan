/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    @author Callum Prentice - September 2016

    LICENSE FILE TO GO HERE
*/

#ifndef _DULLAHAN_CONTEXT_HANDLER
#define _DULLAHAN_CONTEXT_HANDLER

#include "cef_app.h"

class dullahan_context_handler :
    public CefRequestContextHandler
{
    public:
        dullahan_context_handler(const std::string cookieStorageDirectory);
        virtual ~dullahan_context_handler() {}

        CefRefPtr<CefCookieManager> GetCookieManager() OVERRIDE;

        bool OnBeforePluginLoad(const CefString& mime_type,
                                const CefString& plugin_url,
                                const CefString& top_origin_url,
                                CefRefPtr<CefWebPluginInfo> plugin_info,
                                PluginPolicy* plugin_policy) OVERRIDE;

    private:
        CefRefPtr<CefCookieManager> mCookieManager;

        IMPLEMENT_REFCOUNTING(dullahan_context_handler);
};

#endif  // _DULLAHAN_CONTEXT_HANDLER
