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

#ifndef _DULLAHAN_IMPL
#define _DULLAHAN_IMPL

#define NOMINMAX

#include <functional>
#include <sstream>

#include "cef_app.h"
#ifndef CEF_INCLUDE_CEF_VERSION_H_
#include "cef_version.h"
#endif

#include "dullahan.h"
#include "dullahan_debug.h"

class dullahan_browser_client;
class dullahan_render_handler;
class dullahan_callback_manager;
class CefRequestContext;

class dullahan_impl :
    public CefApp,
    public CefPdfPrintCallback
{
        void platormInitWidevine(std::string cachePath);
        void platformAddCommandLines(CefRefPtr<CefCommandLine> command_line);
    public:
        dullahan_impl();
        ~dullahan_impl();

        // CefApp overrides
        virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

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
        void mouseWheel(int x, int y, int deltaX, int deltaY);

#ifndef __linux__
        void nativeKeyboardEventWin(uint32_t msg, uint32_t wparam, uint64_t lparam);
        void nativeKeyboardEventOSX(void* event);
        void nativeKeyboardEventOSX(dullahan::EKeyEvent event_type, uint32_t event_modifiers, uint32_t event_keycode,
                                    uint32_t event_chars, uint32_t event_umodchars, bool event_isrepeat);
#else
        void nativeKeyboardEvent(dullahan::EKeyEvent key_event, uint32_t native_scan_code, uint32_t native_virtual_key, uint32_t native_modifiers);
        void nativeKeyboardEventSDL2(dullahan::EKeyEvent key_event, uint32_t key_data, uint32_t key_modifiers, bool keypad_input);
#endif

        void navigate(const std::string url);
        void setFocus();

        void setPageZoom(const double zoom_val);

        bool editCanUndo();
        bool editCanRedo();
        bool editCanCopy();
        bool editCanCut();
        bool editCanPaste();
        bool editCanDelete();
        bool editCanSelectAll();
        void editUndo();
        void editRedo();
        void editCopy();
        void editCut();
        void editPaste();
        void editDelete();
        void editSelectAll();

        void viewSource();

        void showDevTools();
        void closeDevTools();
        void printToPDF(const std::string path);

        bool setCookie(const std::string url, const std::string name,
                       const std::string value,
                       const std::string domain, const std::string path, bool httponly, bool secure);
        const std::vector<std::string> getAllCookies();
        void deleteAllCookies();
        void flushAllCookies();
        void postData(const std::string url, const std::string data,
                      const std::string headers);
        bool executeJavaScript(const std::string cmd);

        dullahan_callback_manager* getCallbackManager();

        bool getFlipPixelsY();
        bool getFlipMouseY();

        void requestPageZoom();

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
        void OnPdfPrintFinished(const CefString& path, bool ok) override;

    private:
        bool initCEF(dullahan::dullahan_settings& user_settings);

        CefRefPtr<dullahan_browser_client> mBrowserClient;
        CefRefPtr<dullahan_render_handler> mRenderHandler;
        CefRefPtr<CefRequestContext> mRequestContext;
        CefRefPtr<CefBrowser> mBrowser;
        dullahan_callback_manager* mCallbackManager;

        bool mInitialized;
        int mViewWidth;
        int mViewHeight;
        std::string mProxyHostPort;
        bool mSystemFlashEnabled;
        bool mMediaStreamEnabled;
        bool mBeginFrameScheduling;
        bool mForceWaveAudio;
        bool mDisableGPU;
        bool mDisableWebSecurity;
        bool mAllowFileAccessFromFiles;
        bool mDisableNetworkService;
        bool mUseMockKeyChain;
        bool mAutoPlayWithoutGesture;
        bool mFakeUIForMediaStream;
        bool mDisableCookieDatabaseLocking;
        bool mFlipPixelsY;
        bool mFlipMouseY;
        double mRequestedPageZoom;
        const int mViewDepth = 4;
        std::vector<std::string> mCustomSchemes;

        IMPLEMENT_REFCOUNTING(dullahan_impl);
};

#endif // _DULLAHAN_IMPL
