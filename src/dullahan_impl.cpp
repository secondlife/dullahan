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

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#endif

#include "dullahan_impl.h"
#include "dullahan_render_handler.h"
#include "dullahan_browser_client.h"
#include "dullahan_callback_manager.h"
#include "dullahan_context_handler.h"

#include <iostream>

dullahan_impl::dullahan_impl() :
    mBrowser(0),
    mCallbackManager(new dullahan_callback_manager),
    mViewWidth(0),
    mViewHeight(0),
    mSystemFlashEnabled(false),
    mMediaStreamEnabled(false),
    mBeginFrameScheduling(false),
    mForceWaveAudio(false),
    mDisableGPU(true),
    mDisableWebSecurity(false),
    mFlipPixelsY(false),
    mFlipMouseY(false),
    mRequestedPageZoom(1.0)
{
    DLNOUT("dullahan_impl::dullahan_impl()");
}

dullahan_impl::~dullahan_impl()
{
    DLNOUT("dullahan_impl::~dullahan_impl()");
    delete mCallbackManager;
    mCallbackManager = nullptr;
}

void dullahan_impl::OnBeforeCommandLineProcessing(const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line)
{
    if (process_type.empty())
    {
        if (mMediaStreamEnabled == true)
        {
            command_line->AppendSwitch("disable-surfaces");
            command_line->AppendSwitch("enable-media-stream");
        }

        if (mSystemFlashEnabled == true)
        {
            command_line->AppendSwitch("enable-system-flash");
        }

        if (mBeginFrameScheduling == true)
        {
            command_line->AppendSwitch("enable-begin-frame-scheduling");
        }

        if (mForceWaveAudio == true)
        {
            command_line->AppendSwitch("force-wave-audio");
        }

        if (mDisableGPU == true)
        {
            command_line->AppendSwitch("disable-gpu");
            command_line->AppendSwitch("disable-gpu-compositing");
        }

        if (mDisableWebSecurity)
        {
            command_line->AppendSwitch("--disable-web-security");
        }
    }
}

bool dullahan_impl::init(dullahan::dullahan_settings& user_settings)
{
    DLNOUT("dullahan_impl::init()");

#ifdef WIN32
    CefMainArgs args(GetModuleHandle(NULL));
#elif __APPLE__
    CefMainArgs args(0, NULL);
#endif

    CefSettings settings;

    // point to host application
#ifdef WIN32
    CefString(&settings.browser_subprocess_path) = "dullahan_host.exe";
#elif __APPLE__
    NSString* appBundlePath = [[NSBundle mainBundle] bundlePath];
    CefString(&settings.browser_subprocess_path) =
        [[NSString stringWithFormat:
          @"%@/Contents/Frameworks/DullahanHelper.app/Contents/MacOS/DullahanHelper", appBundlePath] UTF8String];
#endif

    // use a single thread for the message loop
    settings.multi_threaded_message_loop = false;

    // turn on only for Windows 7+
    CefEnableHighDPISupport();

    // set path to cache if enabled and set
    if (user_settings.cache_enabled && user_settings.cache_path.length())
    {
        CefString(&settings.cache_path) = user_settings.cache_path;
    }

    // insert a new string into user agent
    if (user_settings.user_agent_substring.length())
    {
        std::string user_agent(user_settings.user_agent_substring);
        cef_string_utf8_to_utf16(user_agent.c_str(), user_agent.size(),
                                 &settings.product_version);
    }
    else
    {
        std::string user_agent = makeCompatibleUserAgentString("");
        cef_string_utf8_to_utf16(user_agent.c_str(), user_agent.size(),
                                 &settings.product_version);
    }

    // list of language locale codes used to configure the Accept-Language HTTP header value
    if (user_settings.accept_language_list.length())
    {
        std::string accept_language_list(user_settings.accept_language_list);
        cef_string_utf8_to_utf16(accept_language_list.c_str(),
                                 accept_language_list.size(), &settings.accept_language_list);
    }

    // enable/disable use of system Flash
    mSystemFlashEnabled = user_settings.plugins_enabled & user_settings.flash_enabled;

    // enable/disable media stream (web cams etc.)
    // IMPORTANT: there is no "Use Your WebCam OK?" dialog so enable this at your peril
    mMediaStreamEnabled = user_settings.media_stream_enabled;

    // this flag needed for some video cards to force onPaints to work - off by default
    mBeginFrameScheduling = user_settings.begin_frame_scheduling;

    // this flag forces Windows WaveOut/In audio API even if Core Audio is supported
    mForceWaveAudio = user_settings.force_wave_audio;

    // this flag if set, adds command line options to disable the GPU and GPU compositing.
    // Appears to be needed to make sites like Google Maps work now. The GPU compositing
    // needs to be off to allow videos to play back without stutter. For the moment, it is
    // recommended that this option always be enabled.
    mDisableGPU = user_settings.disable_gpu;

    // this flag if set, adds command line parameters to disable the web security component
    // that prohibits you from browsing local files.  It is used in the 360 Capture feature
    // in the viewer to open a web page that references locally generated images without
    // needing a web server.
    mDisableWebSecurity = user_settings.disable_web_security;

    // if true, this setting inverts the pixels in Y direction - useful if your texture
    // coords are upside down compared to default for Dullahan
    mFlipPixelsY = user_settings.flip_pixels_y;

    // if true, this setting inverts the injected mouse coordinates in Y direction
    // useful for matching the setting for flipPixelsY
    mFlipMouseY = user_settings.flip_mouse_y;

    // initiaize CEF
    bool result = CefInitialize(args, settings, this, NULL);
    if (! result)
    {
        return false;
    }

    // initial size of the headless browser
    setSize(user_settings.initial_width, user_settings.initial_height);

    // off with it's head
    CefWindowInfo window_info;
    window_info.windowless_rendering_enabled = true;

    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = user_settings.frame_rate;
    browser_settings.webgl = user_settings.webgl_enabled ? STATE_ENABLED : STATE_DISABLED;
    browser_settings.javascript = user_settings.javascript_enabled ? STATE_ENABLED : STATE_DISABLED;
    browser_settings.plugins = user_settings.plugins_enabled ? STATE_ENABLED : STATE_DISABLED;
    browser_settings.application_cache = user_settings.cache_enabled ? STATE_ENABLED : STATE_DISABLED;
    browser_settings.background_color = user_settings.background_color;
    browser_settings.file_access_from_file_urls = user_settings.file_access_from_file_urls ? STATE_ENABLED : STATE_DISABLED;
    browser_settings.image_shrink_standalone_to_fit = user_settings.image_shrink_standalone_to_fit ? STATE_ENABLED : STATE_DISABLED;

    mRenderHandler = new dullahan_render_handler(this);
    mBrowserClient = new dullahan_browser_client(this, mRenderHandler);

    // if this is NULL for CreateBrowserSync, the global request context will be used
    CefRefPtr<CefRequestContext> request_context = NULL;

    // Add a custom context handler that implements a
    // CookieManager so cookies will persist to disk.
    // (if cookies enabled)
    if (user_settings.cookies_enabled)
    {
        std::string cookiePath = ".\\cookies";
        if (user_settings.cookie_store_path.length())
        {
            cookiePath = std::string(user_settings.cookie_store_path);
        }

        CefRequestContextSettings contextSettings;
        mContextHandler = new dullahan_context_handler(cookiePath);
        request_context = CefRequestContext::CreateContext(contextSettings,
                          mContextHandler.get());
    }

    CefString url = "";
    mBrowser = CefBrowserHost::CreateBrowserSync(window_info, mBrowserClient.get(),
               url, browser_settings, request_context);

    return true;
}

void dullahan_impl::shutdown()
{
    mBrowser = NULL;
    mRenderHandler = NULL;
    mBrowserClient = NULL;
    mContextHandler = NULL;

    CefShutdown();
}

void dullahan_impl::requestExit()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        bool force_close = false;

        mBrowser->GetHost()->CloseBrowser(force_close);
    }
}

void dullahan_impl::getSize(int& width, int& height)
{
    width = mViewWidth;
    height = mViewHeight;
}

void dullahan_impl::setSize(int width, int height)
{
    DLNOUT("dullahan_impl::setSize() << width << " << width << " x " << height);

    if (mBrowser.get() && mBrowser->GetHost())
    {
        mViewWidth = width;
        mViewHeight = height;
        mBrowser->GetHost()->WasResized();
    }
}

int dullahan_impl::getDepth()
{
    return mViewDepth;
}

bool dullahan_impl::getFlipPixelsY()
{
    return mFlipPixelsY;
}

bool dullahan_impl::getFlipMouseY()
{
    return mFlipMouseY;
}

void dullahan_impl::run()
{
    CefRunMessageLoop();
}

void dullahan_impl::update()
{
    CefDoMessageLoopWork();

    // CEF/Chromium resets page zoom in between pages
    // so we continually try to set it to the value selected
    // in calls to setPageZoom. Once the required zoom
    // level is reached this call is almost free.
    requestPageZoom();
}

bool dullahan_impl::canGoBack()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        return mBrowser->CanGoBack();
    }

    return true;
}

void dullahan_impl::goBack()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        mBrowser->GoBack();
    }
}

bool dullahan_impl::canGoForward()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        return mBrowser->CanGoForward();
    }

    return true;
}

void dullahan_impl::goForward()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        mBrowser->GoForward();
    }
}

bool dullahan_impl::isLoading()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        return mBrowser->IsLoading();
    }

    return true;
}

void dullahan_impl::reload(const bool ignore_cache)
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        if (ignore_cache)
        {
            mBrowser->ReloadIgnoreCache();
        }
        else
        {
            mBrowser->Reload();
        }
    }
}

void dullahan_impl::stop()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        mBrowser->StopLoad();
    }
}

std::string dullahan_impl::makeCompatibleUserAgentString(const std::string base)
{
    std::string frag = "";
    frag += "(";
    frag += "Dullahan";
    frag += ":";
    frag += dullahan_version(true);
    frag += " - ";
    frag += base;
    frag += ")";
    frag += "  ";
    frag += "Chrome/";
    frag += dullahan_chrome_version(true);

    return frag;
}

void dullahan_impl::navigate(const std::string url)
{
    if (mBrowser.get() && mBrowser->GetMainFrame())
    {
        mBrowser->GetMainFrame()->LoadURL(url);
    }
}

void dullahan_impl::setFocus()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        mBrowser->GetHost()->SendFocusEvent(true);
    }
}

bool dullahan_impl::editCanCopy()
{
    // TODO: ask CEF if we can do this
    return true;
}

bool dullahan_impl::editCanCut()
{
    // TODO: ask CEF if we can do this
    return true;
}

bool dullahan_impl::editCanPaste()
{
    // TODO: ask CEF if we can do this
    return true;
}

void dullahan_impl::editCopy()
{
    if (mBrowser.get() && mBrowser->GetFocusedFrame())
    {
        mBrowser->GetFocusedFrame()->Copy();
    }
}

void dullahan_impl::editCut()
{
    if (mBrowser.get() && mBrowser->GetFocusedFrame())
    {
        mBrowser->GetFocusedFrame()->Cut();
    }
}

void dullahan_impl::editPaste()
{
    if (mBrowser.get() && mBrowser->GetFocusedFrame())
    {
        mBrowser->GetFocusedFrame()->Paste();
    }
}

// Internal call to request that zoom level you asked for with setPageZoom() be actioned.
// We need to do it like this because a plain call to the CEF code SetZoomLevel() can fail
// if then complex multi-process nature of CEF isn't yet established. This is potentially
// called multiple times - likely from CefLoadHandler::OnLoadingStateChange(..)
void dullahan_impl::requestPageZoom()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        // special case the non-zoomed version since slight floating point rounding errors
        // in the formula below result in a few pixels difference - best example of this is
        // when 1024x1024 images in 1024x1024 browser cause scroll bars to appear
        if (mRequestedPageZoom == 1.0)
        {
            // reset zoom level according to CEF docs
            mBrowser->GetHost()->SetZoomLevel(0.0);
            return;
        }

        // Convert "Dullahan page zoom" to "CEF/Chromium page zoom"
        // The value we pass into CEF::SetZoomLevel is not on a linear scale and described here:
        // http://www.magpcss.org/ceforum/viewtopic.php?f=6&t=11491
        // Dullahan scale: 1.0 is 1:1 scale, 2.0 is double, 0.5 is half etc.
        // CEF scale is more complex :) and from that post above, this is the best we can do for now:
        // note: CEF/Chromium max scale seems to be 5 x normal - values higher than that are ignored
        double cef_zoom_level = 5.46149645 * log(mRequestedPageZoom * 100.0) - 25.1511206;

        // if the zoom has not been established (being careful for floating point issues)
        if (fabs(mBrowser->GetHost()->GetZoomLevel() - cef_zoom_level) > 0.001)
        {
            mBrowser->GetHost()->SetZoomLevel(cef_zoom_level);
        }
    }
}

// Set the page zoom directly.  once the page is loaded and waiting say, this will
// work as expected but if init() was just called, the CEF setup might still be in
// progress and this will not do anything - however, requestPageZoom() is called
// often and the zoom will eventually be actioned.
void dullahan_impl::setPageZoom(const double zoom_val)
{
    mRequestedPageZoom = zoom_val;

    requestPageZoom();
}

void dullahan_impl::showDevTools()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        CefWindowInfo window_info;
        window_info.x = 0;
        window_info.y = 0;
        window_info.width = 400;
        window_info.height = 400;
#ifdef WIN32
        window_info.SetAsPopup(NULL, "Dullahan Dev Tools");
#elif __APPLE__
        // TODO: need Apple equivalent
#endif
        CefRefPtr<CefClient> client = mBrowserClient;
        CefBrowserSettings browser_settings;
        CefPoint inspect_element_at;
        mBrowser->GetHost()->ShowDevTools(window_info, client, browser_settings,
                                          inspect_element_at);
    }
}

void dullahan_impl::closeDevTools()
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        mBrowser->GetHost()->CloseDevTools();
    }
}

void dullahan_impl::printToPDF(const std::string path)
{
    if (mBrowser.get() && mBrowser->GetHost())
    {
        CefPdfPrintSettings settings;
        settings.backgrounds_enabled = true;
        settings.landscape = true;
        settings.header_footer_enabled = true;
        CefRefPtr<CefPdfPrintCallback> callback = this;
        mBrowser->GetHost()->PrintToPDF(path, settings, callback);
    }
}

void dullahan_impl::OnPdfPrintFinished(const CefString& path, bool ok)
{
    getCallbackManager()->onPdfPrintFinished(path, ok);
}

bool dullahan_impl::setCookie(const std::string url, const std::string name,
                              const std::string value, const std::string domain,
                              const std::string path, bool httponly, bool secure)
{
    CefRefPtr<CefCookieManager> manager = mContextHandler->GetCookieManager();
    if (manager)
    {
        CefCookie cookie;
        CefString(&cookie.name) = name;
        CefString(&cookie.value) = value;
        CefString(&cookie.domain) = domain;
        CefString(&cookie.path) = path;

        // TODO set from input
        cookie.httponly = false;
        cookie.secure = true;
        cookie.has_expires = true;
        cookie.expires.year = 2064;
        cookie.expires.month = 4;
        cookie.expires.day_of_week = 5;
        cookie.expires.day_of_month = 10;

        const CefRefPtr<CefSetCookieCallback> set_cookie_callback = nullptr;
        bool result = manager->SetCookie(url, cookie, set_cookie_callback);

        const CefRefPtr<CefCompletionCallback> flush_store_callback = nullptr;
        manager->FlushStore(flush_store_callback);

        return result;
    }

    return false;
}

void dullahan_impl::deleteAllCookies()
{
    CefRefPtr<CefCookieManager> manager = mContextHandler->GetCookieManager();
    if (manager)
    {
        // empty URL deletes all cookies for all domains
        const CefString url("");
        const CefString name("");
        const CefRefPtr<CefDeleteCookiesCallback> callback = nullptr;
        manager->DeleteCookies(url, name, callback);
    }
}

void dullahan_impl::postData(const std::string url, const std::string data,
                             const std::string headers)
{
    if (mBrowser.get() && mBrowser->GetMainFrame())
    {
        CefRefPtr<CefRequest> request = CefRequest::Create();

        // this is a POST request
        request->SetURL(url);
        request->SetMethod("POST");

        // TODO - get this from the headers parameter
        CefRequest::HeaderMap headerMap;
        headerMap.insert(std::make_pair("Accept", "*/*"));
        headerMap.insert(std::make_pair("Content-Type",
                                        "application/x-www-form-urlencoded"));
        request->SetHeaderMap(headerMap);

        // set up data
        const std::string& upload_data = data;
        CefRefPtr<CefPostData> postData = CefPostData::Create();
        CefRefPtr<CefPostDataElement> element = CefPostDataElement::Create();
        element->SetToBytes(upload_data.size(), upload_data.c_str());
        postData->AddElement(element);
        request->SetPostData(postData);

        // make the post
        mBrowser->GetMainFrame()->LoadRequest(request);

        // TODO - where do we catch what comes back?
    }
}

bool dullahan_impl::executeJavaScript(const std::string cmd)
{
    if (mBrowser.get() && mBrowser->GetMainFrame())
    {
        mBrowser->GetMainFrame()->ExecuteJavaScript(cmd, std::string(), 0);
    }
    return false;
}

dullahan_callback_manager* dullahan_impl::getCallbackManager()
{
    return mCallbackManager;
}

void dullahan_impl::setCustomSchemes(std::vector<std::string> custom_schemes)
{
    mCustomSchemes = custom_schemes;
}

std::vector<std::string>& dullahan_impl::getCustomSchemes()
{
    return mCustomSchemes;
}

CefRefPtr<CefBrowser> dullahan_impl::getBrowser()
{
    return mBrowser;
}

void dullahan_impl::setBrowser(CefRefPtr<CefBrowser> browser)
{
    mBrowser = browser;
}

void dullahan_impl::showBrowserMessage(const std::string msg)
{
    std::stringstream url;

    url << "data:text/html;charset=utf-8,<html>%0D%0A<head>%0D%0A<style>%0D%0Abody%20%7B%0D%0Abackground-color%3A%20%23633%3B%0D%0Acolor%3A%23ccc%3B%0D%0A%7D%0D%0A%23msg%20%7B%0D%0Amargin-top";
    url << "%3A%20128px%3B%0D%0Amargin-left%3A%20128px%3B%0D%0Amargin-right%3A%20128px%3B%0D%0Afont-family%3AVerdana%3B%0D%0Afont-size%3A1.5em%3B%0D%0Abackground-color%3A%20%23422%3B%0D%0A";
    url << "line-height%3A%20150%25%3B%0D%0Apadding%3A%208px%3B%0D%0A%7D%0D%0A<%2Fstyle>%0D%0A<%2Fhead>%0D%0A<body>%0D%0A<div%20id%3D%27msg%27>%0D%0A";
    url << msg;
    url << "<%2Fdiv>%0D%0A<%2Fbody>%0D%0A<%2Fhtml>";

    navigate(url.str());
}

const std::string dullahan_impl::append_bitwidth_string(std::ostringstream& stream, bool show_bitwidth)
{
    if (show_bitwidth)
    {
        size_t bit_width = sizeof(void*) * 8;
        stream << " ";
        stream << "[";
        stream << bit_width;
        stream << "bit";
        stream << "]";
    }

    return stream.str();
}

const std::string dullahan_impl::dullahan_cef_version(bool show_bitwidth)
{
    std::ostringstream s;
    s << CEF_VERSION;

    return append_bitwidth_string(s, show_bitwidth);
}

const std::string dullahan_impl::dullahan_chrome_version(bool show_bitwidth)
{
    std::ostringstream s;
    s << CHROME_VERSION_MAJOR;
    s << ".";
    s << CHROME_VERSION_MINOR;
    s << ".";
    s << CHROME_VERSION_BUILD;
    s << ".";
    s << CHROME_VERSION_PATCH;

    return append_bitwidth_string(s, show_bitwidth);
}

const std::string dullahan_impl::dullahan_version(bool show_bitwidth)
{
    std::ostringstream s;

    s << DULLAHAN_VERSION_MAJOR;
    s << ".";
    s << DULLAHAN_VERSION_MINOR;
    s << ".";
    s << DULLAHAN_VERSION_BUILD;

    return append_bitwidth_string(s, show_bitwidth);
}

const std::string dullahan_impl::composite_version()
{
    std::ostringstream version;

    version << "Dullahan: ";
    version << dullahan_version(false);
    version << " (CEF: ";
    version << dullahan_cef_version(false);
    version << " - Chrome: ";
    version << dullahan_chrome_version(false);
    version << ")";

    return append_bitwidth_string(version, true);
}
