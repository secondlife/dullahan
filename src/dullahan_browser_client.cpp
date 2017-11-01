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

#include "cef_browser.h"
#include "wrapper/cef_helpers.h"

#include "dullahan_render_handler.h"
#include "dullahan_browser_client.h"
#include "dullahan_callback_manager.h"

#include "dullahan_impl.h"

dullahan_browser_client::dullahan_browser_client(dullahan_impl* parent,
        dullahan_render_handler* render_handler) :
    mParent(parent),
    mRenderHandler(render_handler)
{
    DLNOUT("dullahan_browser_client::dullahan_browser_client - parent ptr = " << parent);
}

dullahan_browser_client::~dullahan_browser_client()
{
    mRenderHandler = NULL;
}

// CefClient override
CefRefPtr<CefRenderHandler> dullahan_browser_client::GetRenderHandler()
{
    return mRenderHandler;
}

// CefLifeSpanHandler override
bool dullahan_browser_client::OnBeforePopup(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& target_url, const CefString& target_frame_name,
        CefLifeSpanHandler::WindowOpenDisposition target_disposition,
        bool user_gesture, const CefPopupFeatures& popupFeatures,
        CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client,
        CefBrowserSettings& settings, bool* no_javascript_access)
{
    CEF_REQUIRE_IO_THREAD();

    std::string url = std::string(target_url);
    std::string target = std::string(target_frame_name);

    // note target string is empty if target in HTML is "_blank" so don't test
    // for target length - just url which cannot be empty
    if (url.length())
    {
        mParent->getCallbackManager()->onNavigateURL(url, target);
        return true;
    }

    browser->GetMainFrame()->LoadURL(target_url);

    return true;
}

// CefLifeSpanHandler override
void dullahan_browser_client::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    mBrowserList.push_back(browser);
}

// CefLifeSpanHandler override
void dullahan_browser_client::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    BrowserList::iterator bit = mBrowserList.begin();
    for (; bit != mBrowserList.end(); ++bit)
    {
        if ((*bit)->IsSame(browser))
        {
            mBrowserList.erase(bit);
            break;
        }
    }

    if (mBrowserList.empty())
    {
        // TODO: mark this call as only needed if using CEFs message loop and not CefDoMessageLoopWork()
        CefQuitMessageLoop();

        mParent->getCallbackManager()->onRequestExit();
    }
}

// CefLifeSpanHandler override
bool dullahan_browser_client::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    return false;
}

// CefDisplayhandler override
void dullahan_browser_client::OnAddressChange(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame, const CefString& url)
{
    CEF_REQUIRE_UI_THREAD();

    mParent->getCallbackManager()->onAddressChange(std::string(url));
}

// CefDisplayhandler override
bool dullahan_browser_client::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
        const CefString& message, const CefString& source, int line)
{
    CEF_REQUIRE_UI_THREAD();

    mParent->getCallbackManager()->onConsoleMessage(std::string(message),
            std::string(source), line);

    return true;
}

// CefDisplayhandler override
void dullahan_browser_client::OnStatusMessage(CefRefPtr<CefBrowser> browser,
        const CefString& value)
{
    CEF_REQUIRE_UI_THREAD();

    mParent->getCallbackManager()->onStatusMessage(std::string(value));
}

// CefDisplayhandler overrides
void dullahan_browser_client::OnTitleChange(CefRefPtr<CefBrowser> browser,
        const CefString& title)
{
    CEF_REQUIRE_UI_THREAD();

    mParent->getCallbackManager()->onTitleChange(std::string(title));
}

// CefLoadHandler override
void dullahan_browser_client::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
        bool isLoading, bool canGoBack, bool canGoForward)
{
    CEF_REQUIRE_UI_THREAD();

    // Terrible hack but AFAICT, this is the only (and perhaps even official) way to
    // establish a zoom across the browser - there ought to be a setting at startup to change this.
    // Each time a page load starts/ends, this will re-request the zoom. The page zoom is reset
    // between pages so the effect is very jarring but it's the best we can do right now.
    mParent->requestPageZoom();
}

// CefLoadHandler override
void dullahan_browser_client::OnLoadStart(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        TransitionType transition_type)
{
    CEF_REQUIRE_UI_THREAD();

    if (frame->IsMain())
    {
        mParent->getCallbackManager()->onLoadStart();
    }
}

// CefLoadHandler override
void dullahan_browser_client::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    CEF_REQUIRE_UI_THREAD();

    if (frame->IsMain())
    {
        mParent->getCallbackManager()->onLoadEnd(httpStatusCode);
    }
}

// CefLoadHandler override
void dullahan_browser_client::OnLoadError(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame, ErrorCode errorCode,
        const CefString& errorText, const CefString& failedUrl)
{
    CEF_REQUIRE_UI_THREAD();

    if (errorCode == ERR_ABORTED)
    {
        return;
    }

    if (frame->IsMain())
    {
        mParent->getCallbackManager()->onLoadError(errorCode, std::string(errorText));
    }
}

// CefRequestHandler override
bool dullahan_browser_client::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request, bool isRedirect)
{
    CEF_REQUIRE_UI_THREAD();

    std::string url = request->GetURL();

    // for conmparison
    std::transform(url.begin(), url.end(), url.begin(), ::tolower);

    std::vector<std::string>::iterator iter = mParent->getCustomSchemes().begin();
    while (iter != mParent->getCustomSchemes().end())
    {
        if (url.substr(0, (*iter).length()) == (*iter))
        {
            // get URL again since we lower cased it for comparison
            url = request->GetURL();
            mParent->getCallbackManager()->onCustomSchemeURL(url);

            // don't continue with navigation
            return true;
        }

        ++iter;
    }

    return false;
}

// CefRequestHandler override
bool dullahan_browser_client::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        bool isProxy, const CefString& host, int port, const CefString& realm,
        const CefString& scheme, CefRefPtr<CefAuthCallback> callback)
{
    CEF_REQUIRE_IO_THREAD();

    std::string host_str = host;
    std::string realm_str = realm;
    std::string scheme_str = scheme;

    std::string username = "";
    std::string password = "";
    bool proceed = mParent->getCallbackManager()->onHTTPAuth(host_str, realm_str,
                   username, password);

    if (proceed)
    {
        callback->Continue(username.c_str(), password.c_str());
        return true; // continue with request
    }
    else
    {
        callback->Cancel();
        return false; // cancel request
    }
}

// CefRequestHandler override
bool dullahan_browser_client::OnQuotaRequest(CefRefPtr<CefBrowser> browser,
        const CefString& origin_url,
        int64 new_size,
        CefRefPtr<CefRequestCallback> callback)
{
    CEF_REQUIRE_IO_THREAD();

    // 10MB hard coded for now
    static const int64 max_size = 1024 * 1024 * 10;

    callback->Continue(new_size <= max_size);
    return true;
}

// CefDownloadHandler overrides
void dullahan_browser_client::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback)
{
    CEF_REQUIRE_UI_THREAD();


    bool show_file_dialog = true;
    callback->Continue(suggested_name, show_file_dialog);
    mParent->getCallbackManager()->onFileDownload(std::string(suggested_name));
}

void dullahan_browser_client::OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        CefRefPtr<CefDownloadItemCallback> callback)
{
    CEF_REQUIRE_UI_THREAD();

    bool is_in_progress = download_item->IsInProgress();
    int percent_complete = max(0, download_item->GetPercentComplete());
    bool is_complete = download_item->IsComplete();

    if (is_in_progress)
    {
        mParent->getCallbackManager()->onFileDownloadProgress(percent_complete, is_complete);
    }
}

// CefDialogHandler orerrides
bool dullahan_browser_client::OnFileDialog(CefRefPtr<CefBrowser> browser,
        FileDialogMode mode,
        const CefString& title,
        const CefString& default_file_path,
        const std::vector<CefString>& accept_filters,
        int selected_accept_filter,
        CefRefPtr<CefFileDialogCallback> callback)
{
    CEF_REQUIRE_UI_THREAD();

    dullahan::EFileDialogType dialog_type = dullahan::FD_UNKNOWN;
    if ((mode & 0x0f) ==  FileDialogMode::FILE_DIALOG_OPEN)
    {
        dialog_type = dullahan::FD_OPEN_FILE;
    }
    else if ((mode & 0x0f) == FileDialogMode::FILE_DIALOG_OPEN_FOLDER)
    {
        dialog_type = dullahan::FD_OPEN_FOLDER;
    }
    else if ((mode & 0x0f) == FileDialogMode::FILE_DIALOG_OPEN_MULTIPLE)
    {
        dialog_type = dullahan::FD_OPEN_MULTIPLE_FILES;
    }
    else if ((mode & 0x0f) == FileDialogMode::FILE_DIALOG_SAVE)
    {
        dialog_type = dullahan::FD_SAVE_FILE;
    }

    const std::string dialog_title = std::string(title);
    std::string dialog_accept_filter = std::string();
    if (accept_filters.size() > 0)
    {
        std::string dialog_accept_filter = std::string(accept_filters[0]);
    }

    bool use_default = true;
    const CefString file_path = mParent->getCallbackManager()->onFileDialog(dialog_type, dialog_title, dialog_accept_filter, use_default);
    if (use_default)
    {
        return false;
    }

    if (file_path.length())
    {
        std::vector<CefString> file_paths;
        file_paths.push_back(CefString(file_path));

        const int file_path_index = 0;
        callback->Continue(file_path_index, file_paths);
    }
    else
    {
        callback->Cancel();
    }

    return true;
}
