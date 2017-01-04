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
