/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

           Example: minimal CEF example that doesn't use
           Dullahan- useful for iterating quickly on tricky issues

    @author Callum Prentice - April 2017

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

#include <iostream>
#include <ctime>
#include <list>

#include "cef_app.h"
#include "cef_client.h"
#include "wrapper/cef_helpers.h"

bool gExitFlag = false;

class RenderHandler :
    public CefRenderHandler
{
    public:
        bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
        {
            int width = 512;
            int height = 512;
            rect = CefRect(0, 0, width, height);
            return true;
        }

        void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
        {
            std::cout << "OnPaint() for size: " << width << " x " << height << std::endl;
        }

        IMPLEMENT_REFCOUNTING(RenderHandler);
};

class BrowserClient :
    public CefClient,
    public CefLifeSpanHandler
{
    public:
        BrowserClient(RenderHandler* render_handler) :
            render_handler_(render_handler)
        {
        }

        CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler() OVERRIDE
        {
            return render_handler_;
        }

        CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
        {
            return this;
        }

        void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE
        {
            CEF_REQUIRE_UI_THREAD();

            browser_list_.push_back(browser);
        }

        void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE
        {
            CEF_REQUIRE_UI_THREAD();

            BrowserList::iterator bit = browser_list_.begin();
            for (; bit != browser_list_.end(); ++bit)
            {
                if ((*bit)->IsSame(browser))
                {
                    browser_list_.erase(bit);
                    break;
                }
            }

            if (browser_list_.empty())
            {
                gExitFlag = true;
            }
        }

        IMPLEMENT_REFCOUNTING(BrowserClient);

    private:
        CefRefPtr<CefRenderHandler> render_handler_;
        typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
        BrowserList browser_list_;
};

class CefMinimal : public CefApp
{
    public:
        ~CefMinimal()
        {
        }

        bool init()
        {
            CefMainArgs args(GetModuleHandle(NULL));

            CefSettings settings;
            CefString(&settings.browser_subprocess_path) = "dullahan_host.exe";

            if (CefInitialize(args, settings, this, NULL))
            {
                std::cout << "CefMinimal initialized okay" << std::endl;

                render_handler_ = new RenderHandler();

                browser_client_ = new BrowserClient(render_handler_);

                CefWindowInfo window_info;
                window_info.windowless_rendering_enabled = true;

                CefBrowserSettings browser_settings;
                browser_settings.windowless_frame_rate = 60;

                CefString url = "http://cnn.com";
                browser_ = CefBrowserHost::CreateBrowserSync(window_info, browser_client_.get(), url, browser_settings, nullptr);

                return true;
            }

            std::cout << "Unable to initialize" << std::endl;
            return false;
        }

        void update()
        {
            CefDoMessageLoopWork();
        }

        void requestExit()
        {
            if (browser_.get() && browser_->GetHost())
            {
                browser_->GetHost()->CloseBrowser(true);
            }
        }

        void shutdown()
        {
            render_handler_ = NULL;
            browser_client_ = NULL;
            browser_ = NULL;
            CefShutdown();
        }

        IMPLEMENT_REFCOUNTING(CefMinimal);

    private:
        CefRefPtr<RenderHandler> render_handler_;
        CefRefPtr<BrowserClient> browser_client_;
        CefRefPtr<CefBrowser> browser_;
};

int main(int argc, char* argv[])
{
    CefRefPtr<CefMinimal> cm = new CefMinimal();

    cm->init();

    time_t start_time;
    time(&start_time);

    MSG msg;
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            cm->update();

            if (gExitFlag == false)
            {
                if (time(NULL) > start_time + 3)
                {
                    cm->requestExit();
                }
            }
            else
            {
                PostQuitMessage(0);
            }
        }
    }
    while (msg.message != WM_QUIT);

    cm->shutdown();

    cm = NULL;

    return 0;
}
