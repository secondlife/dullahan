/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

           Example: A minimal CEF example that doesn't use Dullahan
                    to help with debugging issues by posting modified
                    versions of this to CEF forum that illustrate
                    a bug of problem I am having

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

#include <iostream>
#include <ctime>
#include <string>
#include <iostream>
#include <list>

#include "cef_app.h"
#include "cef_client.h"
#include "wrapper/cef_helpers.h"

bool gExitFlag = false;

class RenderHandler :
    public CefRenderHandler
{
    public:
        void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
        {
            CEF_REQUIRE_UI_THREAD();

            int width = 1024;
            int height = 1024;
            rect = CefRect(0, 0, width, height);

            std::cout << "GetViewRect() offered size: " << width << " x " << height << std::endl;
        }

        void OnPaint(CefRefPtr<CefBrowser> browser,
                     PaintElementType type,
                     const RectList& dirtyRects,
                     const void* buffer,
                     int width, int height) override
        {
            CEF_REQUIRE_UI_THREAD();

            // display dirty rectangles - oddly each one comes in on its own (array size of 1) vs
            // collecting them together - probably a good reason :)
            std::cout << "OnPaint() called with size: " << width << " x " << height << std::endl;
            for (int r = 0; r < dirtyRects.size(); r++)
            {
                std::cout << "    Rect " << r << ": " << dirtyRects[r].width << " x " << dirtyRects[r].width << std::endl;
            }
            std::cout << std::endl;
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

        CefRefPtr<CefRenderHandler> GetRenderHandler() override
        {
            return render_handler_;
        }

        CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
        {
            return this;
        }

        void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
        {
            CEF_REQUIRE_UI_THREAD();

            browser_list_.push_back(browser);
        }

        void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
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
        CefMinimal()
        {
            mInitialized = false;
        }

        bool init(int argc, char* argv[])
        {
            CefSettings settings;
            settings.multi_threaded_message_loop = false;
            settings.windowless_rendering_enabled = true;

            CefEnableHighDPISupport();

            CefMainArgs args(GetModuleHandle(nullptr));

            if (CefInitialize(args, settings, this, nullptr))
            {
                std::cout << "CefMinimal initialized okay" << std::endl;

                CefWindowInfo window_info;
                window_info.windowless_rendering_enabled = true;
                window_info.shared_texture_enabled = false;

                CefBrowserSettings browser_settings;
                browser_settings.windowless_frame_rate = 60;
                browser_settings.background_color = 0xffffffff;

                render_handler_ = new RenderHandler();

                browser_client_ = new BrowserClient(render_handler_);

                CefString url = "https://news.google.com";
                browser_ = CefBrowserHost::CreateBrowserSync(window_info, browser_client_.get(), url, browser_settings, nullptr, nullptr);

                if (browser_.get() && browser_->GetHost())
                {
                    browser_->GetHost()->WasResized();
                }

                mInitialized = true;

                return true;
            }

            std::cout << "Unable to initialize" << std::endl;
            return false;
        }

        void update()
        {
            if (!mInitialized)
            {
                return;
            }

            CefDoMessageLoopWork();
        }

        void navigate(std::string url)
        {
            if (browser_.get() && browser_->GetMainFrame())
            {
                std::cout << "Navigating to " << url << std::endl;
                browser_->GetMainFrame()->LoadURL(url);
            }
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
            render_handler_ = nullptr;
            browser_client_ = nullptr;
            browser_ = nullptr;

            CefShutdown();
        }

        IMPLEMENT_REFCOUNTING(CefMinimal);

    private:
        CefRefPtr<RenderHandler> render_handler_;
        CefRefPtr<BrowserClient> browser_client_;
        CefRefPtr<CefBrowser> browser_;
        bool mInitialized;
};

int main(int argc, char* argv[])
{
    CefMainArgs main_args(GetModuleHandle(NULL));
    int exit_code = CefExecuteProcess(main_args, NULL, nullptr);
    if (exit_code >= 0)
    {
        return exit_code;
    }

    CefRefPtr<CefMinimal> cm = new CefMinimal();

    cm->init(argc, argv);

    time_t start_time;
    time(&start_time);

    MSG msg;
    int state = 0;
    do
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            cm->update();

            if (gExitFlag == false)
            {
                if (time(nullptr) > start_time + 2)
                {
                    if (state == 0)
                    {
                        cm->navigate("https://news.bbc.co.uk");
                        state = 1;
                    }
                }

                if (time(nullptr) > start_time + 4)
                {
                    if (state == 1)
                    {
                        cm->navigate("https://www.imdb.com");
                        state = 2;
                    }
                }

                if (time(nullptr) > start_time + 6)
                {
                    if (state == 2)
                    {
                        cm->navigate("https://www.reddit.com");
                        state = 3;
                    }
                }

                if (time(nullptr) > start_time + 15)
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

    cm = nullptr;

    return 0;
}