/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    @author Callum Prentice - September 2016

    LICENSE FILE TO GO HERE
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

static const int DULLAHAN_VERSION_MAJOR = 0;
static const int DULLAHAN_VERSION_MINOR = 2;

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

        void setCustomSchemes(std::vector<std::string> custom_schemes);
        std::vector<std::string>& getCustomSchemes();

        CefRefPtr<CefBrowser> getBrowser();
        void setBrowser(CefRefPtr<CefBrowser> browser);

        void showBrowserMessage(const std::string msg);

        const std::string dullahan_cef_version();
        const std::string dullahan_chrome_version();
        const std::string dullahan_version();
        const std::string composite_version();

        // CefPdfPrintCallback overrides
        void OnPdfPrintFinished(const CefString& path, bool ok) OVERRIDE;

    private:
        CefRefPtr<dullahan_browser_client> mBrowserClient;
        CefRefPtr<CefBrowser> mBrowser;
        CefRefPtr<dullahan_context_handler> mContextHandler;
        dullahan_callback_manager* mCallbackManager;
        int mViewWidth;
        int mViewHeight;
        const int mViewDepth = 4;
        std::vector<std::string> mCustomSchemes;
        bool mSystemFlashEnabled;
        bool mMediaStreamEnabled;
        bool mBeginFrameScheduling;

        IMPLEMENT_REFCOUNTING(dullahan_impl);
};

#endif // _DULLAHAN_IMPL
