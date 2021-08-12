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

#ifndef _DULLAHAN_BROWSER_CLIENT
#define _DULLAHAN_BROWSER_CLIENT

#include <list>

#include "cef_client.h"

class dullahan_impl;
class dullahan_renderer_handler;

class dullahan_browser_client :
    public CefClient,
    public CefLifeSpanHandler,
    public CefDisplayHandler,
    public CefLoadHandler,
    public CefRequestHandler,
    public CefDownloadHandler,
    public CefDialogHandler,
    public CefJSDialogHandler
{
    public:
        dullahan_browser_client(dullahan_impl* parent,
                                dullahan_render_handler* render_handler);
        ~dullahan_browser_client();

        // CefClient override
        CefRefPtr<CefRenderHandler> GetRenderHandler() override;

        // CefLifeSpanHandler overrides
        CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
        {
            return this;
        }
        bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           const CefString& target_url,
                           const CefString& target_frame_name,
                           CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                           bool user_gesture,
                           const CefPopupFeatures& popupFeatures,
                           CefWindowInfo& windowInfo,
                           CefRefPtr<CefClient>& client,
                           CefBrowserSettings& settings,
                           CefRefPtr<CefDictionaryValue>& extra_info,
                           bool* no_javascript_access) override;
        void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
        void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
        bool DoClose(CefRefPtr<CefBrowser> browser) override;

        // CefDisplayhandler overrides
        CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
        {
            return this;
        }
        void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             const CefString& url) override;
        bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level,
                              const CefString& message, const CefString& source, int line) override;
        void OnStatusMessage(CefRefPtr<CefBrowser> browser,
                             const CefString& value) override;
        void OnTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) override;
        bool OnTooltip(CefRefPtr<CefBrowser> browser,
                       CefString& text) override;
        bool OnCursorChange(CefRefPtr<CefBrowser> browser,
                            CefCursorHandle cursor, cef_cursor_type_t type,
                            const CefCursorInfo& custom_cursor_info) override;

        // CefLoadHandler overrides
        CefRefPtr<CefLoadHandler> GetLoadHandler() override
        {
            return this;
        }
        void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading,
                                  bool canGoBack, bool canGoForward) override;
        void OnLoadStart(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         TransitionType transition_type) override;
        void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                       int httpStatusCode) override;
        void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                         ErrorCode errorCode,
                         const CefString& errorText, const CefString& failedUrl) override;

        // CefRequestHandler overrides
        CefRefPtr<CefRequestHandler> GetRequestHandler() override
        {
            return this;
        }
        bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request, bool user_gesture, bool isRedirect) override;
        bool GetAuthCredentials(CefRefPtr<CefBrowser> browser, const CefString& origin_url, bool isProxy,
                                const CefString& host, int port, const CefString& realm,
                                const CefString& scheme, CefRefPtr<CefAuthCallback> callback) override;

        bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
                            const CefString& origin_url,
                            int64 new_size,
                            CefRefPtr<CefRequestCallback> callback) override;

        // CefDownloadHandler overrides
        CefRefPtr<CefDownloadHandler> GetDownloadHandler() override
        {
            return this;
        }
        void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefDownloadItem> download_item,
                              const CefString& suggested_name,
                              CefRefPtr<CefBeforeDownloadCallback> callback) override;
        void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefDownloadItem> download_item,
                               CefRefPtr<CefDownloadItemCallback> callback) override;

        // CefDialogHandler orerrides
        CefRefPtr<CefDialogHandler> GetDialogHandler() override
        {
            return this;
        }
        bool OnFileDialog(CefRefPtr<CefBrowser> browser, FileDialogMode mode, const CefString& title,
                          const CefString& default_file_path, const std::vector<CefString>& accept_filters, int selected_accept_filter,
                          CefRefPtr<CefFileDialogCallback> callback) override;

        // CefJSDialogHandler overrides
        CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() override
        {
            return this;
        }
        bool OnJSDialog(CefRefPtr<CefBrowser> browser,
                        const CefString& origin_url,
                        JSDialogType dialog_type,
                        const CefString& message_text,
                        const CefString& default_prompt_text,
                        CefRefPtr<CefJSDialogCallback> callback,
                        bool& suppress_message) override;

        bool OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
                                  const CefString& message_text,
                                  bool is_reload,
                                  CefRefPtr<CefJSDialogCallback> callback) override;
    private:
        dullahan_impl* mParent;
        CefRefPtr<CefRenderHandler> mRenderHandler;
        typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
        BrowserList mBrowserList;

    public:
        IMPLEMENT_REFCOUNTING(dullahan_browser_client);
};

#endif // _DULLAHAN_BROWSER_CLIENT
