/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    @author Callum Prentice - September 2016

    LICENSE FILE TO GO HERE
*/

#include "dullahan_context_handler.h"
#include "dullahan_debug.h"

dullahan_context_handler::dullahan_context_handler(const std::string
        cookieStorageDirectory)
{
    bool persist_session_cookies = false;
    CefRefPtr<CefCompletionCallback> callback = nullptr;
    mCookieManager = CefCookieManager::CreateManager(CefString(
                         cookieStorageDirectory), persist_session_cookies, callback);
};

CefRefPtr<CefCookieManager> dullahan_context_handler::GetCookieManager()
{
    return mCookieManager;
}

bool dullahan_context_handler::OnBeforePluginLoad(const CefString& mime_type,
        const CefString& plugin_url,
        const CefString& top_origin_url,
        CefRefPtr<CefWebPluginInfo> plugin_info,
        PluginPolicy* plugin_policy)
{
    if (*plugin_policy != PLUGIN_POLICY_ALLOW &&
            mime_type == "application/pdf")
    {
        *plugin_policy = PLUGIN_POLICY_ALLOW;
        return true;
    }

    return false;
}
