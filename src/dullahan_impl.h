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

#ifndef _DULLAHAN_IMPL
#define _DULLAHAN_IMPL

#include <functional>
#include <sstream>

#include "cef_app.h"
#include "cef_version.h"

#include "dullahan.h"
#include "dullahan_debug.h"

class dullahan_browser_client;
class dullahan_context_handler;
class dullahan_callback_manager;

class dullahan_impl :
    public CefApp,
    public CefPdfPrintCallback
{
    public:
        dullahan_impl();
        ~dullahan_impl();

        // CefApp overrides
        virtual void OnBeforeCommandLineProcessing(const CefString& process_type,
                CefRefPtr<CefCommandLine> command_line) OVERRIDE;

        bool init(dullahan::dullahan_settings& user_settings);
        void shutdown();
        void requestExit();

        void getSize(int& width, int& height);
        void setSize(int width, int height);
        int getDepth();

        void run();
        void update();

        bool canGoBack();
        void goBack();
        bool canGoForward();
        void goForward();
        bool isLoading();
        void reload(const bool ignore_cache);
        void stop();

        std::string makeCompatibleUserAgentString(const std::string base);

        void mouseButton(dullahan::EMouseButton mouse_button,
                         dullahan::EMouseEvent mouse_event, int x, int y);
        void mouseMove(int x, int y);
        void mouseWheel(int deltaX, int deltaY);

        void nativeKeyboardEvent(uint32_t msg, uint32_t wparam, uint64_t lparam);

        void navigate(const std::string url);
        void setFocus();
        void setPageZoom(const double zoom_val);

        bool editCanCopy();
        bool editCanCut();
        bool editCanPaste();
        void editCopy();
        void editCut();
        void editPaste();

        void showDevTools();
        void closeDevTools();
        void printToPDF(const std::string path);

        bool setCookie(const std::string url, const std::string name,
                       const std::string value,
                       const std::string domain, const std::string path, bool httponly, bool secure);
        void deleteAllCookies();
        void postData(const std::string url, const std::string data,
                      const std::string headers);
        bool executeJavaScript(const std::string cmd);

        dullahan_callback_manager* getCallbackManager();

        bool getFlipPixelsY();
        bool getFlipMouseY();

        void setCustomSchemes(std::vector<std::string> custom_schemes);
        std::vector<std::string>& getCustomSchemes();

        CefRefPtr<CefBrowser> getBrowser();
        void setBrowser(CefRefPtr<CefBrowser> browser);

        void showBrowserMessage(const std::string msg);

        const std::string append_bitwidth_string(std::ostringstream& stream, bool show_bitwidth);
        const std::string dullahan_cef_version(bool show_bitwidth);
        const std::string dullahan_chrome_version(bool show_bitwidth);
        const std::string dullahan_version(bool show_bitwidth);
        const std::string composite_version();

        // CefPdfPrintCallback overrides
        void OnPdfPrintFinished(const CefString& path, bool ok) OVERRIDE;

    private:
        CefRefPtr<dullahan_browser_client> mBrowserClient;
        dullahan_callback_manager* mCallbackManager;
        int mViewWidth;
        int mViewHeight;
        CefRefPtr<CefBrowser> mBrowser;
        bool mSystemFlashEnabled;
        bool mMediaStreamEnabled;
        bool mBeginFrameScheduling;
        bool mForceWaveAudio;
        bool mFlipPixelsY;
        bool mFlipMouseY;
        CefRefPtr<dullahan_context_handler> mContextHandler;
        const int mViewDepth = 4;
        std::vector<std::string> mCustomSchemes;

        IMPLEMENT_REFCOUNTING(dullahan_impl);
};

#endif // _DULLAHAN_IMPL
