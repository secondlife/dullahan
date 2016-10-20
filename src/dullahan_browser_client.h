/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework
    @author Callum Prentice 2015

    LICENSE FILE TO GO HERE
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
    public CefDialogHandler
{
    public:
        dullahan_browser_client(dullahan_impl* parent,
                                dullahan_render_handler* render_handler);

        // CefClient override
        CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE;

        // CefLifeSpanHandler overrides
        CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
        bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           const CefString& target_url, const CefString& target_frame_name,
                           CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                           bool user_gesture, const CefPopupFeatures& popupFeatures,
                           CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client,
                           CefBrowserSettings& settings, bool* no_javascript_access) OVERRIDE;
        void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
        void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
        bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

        // CefDisplayhandler overrides
        CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
        void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             const CefString& url) OVERRIDE;
        bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message,
                              const CefString& source, int line) OVERRIDE;
        void OnStatusMessage(CefRefPtr<CefBrowser> browser,
                             const CefString& value) OVERRIDE;
        void OnTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) OVERRIDE;

        // LCefLoadHandler overrides
        CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
        void OnLoadStart(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame) OVERRIDE;
        void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                       int httpStatusCode) OVERRIDE;
        void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                         ErrorCode errorCode,
                         const CefString& errorText, const CefString& failedUrl) OVERRIDE;

        // CefRequestHandler overrides
        CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }
        bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request, bool isRedirect) OVERRIDE;
        bool GetAuthCredentials(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                bool isProxy, const CefString& host, int port, const CefString& realm,
                                const CefString& scheme, CefRefPtr<CefAuthCallback> callback) OVERRIDE;
        bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
                            const CefString& origin_url,
                            int64 new_size,
                            CefRefPtr<CefRequestCallback> callback) OVERRIDE;

        // CefDownloadHandler overrides
        CefRefPtr<CefDownloadHandler> GetDownloadHandler() OVERRIDE{ return this; }
        void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefDownloadItem> download_item,
                              const CefString& suggested_name,
                              CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;

        // CefDialogHandler orerrides
        CefRefPtr<CefDialogHandler> GetDialogHandler() OVERRIDE { return this; }
        bool OnFileDialog(CefRefPtr<CefBrowser> browser, FileDialogMode mode, const CefString& title,
                          const CefString& default_file_path, const std::vector<CefString>& accept_filters, int selected_accept_filter,
                          CefRefPtr<CefFileDialogCallback> callback) OVERRIDE;

    private:
        dullahan_impl* mParent;
        CefRefPtr<CefRenderHandler> mRenderHandler;
        typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
        BrowserList mBrowserList;

    public:
        IMPLEMENT_REFCOUNTING(dullahan_browser_client);
};

#endif // _DULLAHAN_BROWSER_CLIENT
