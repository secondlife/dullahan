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

#include "dullahan.h"
#include "dullahan_impl.h"
#include "dullahan_debug.h"
#include "dullahan_callback_manager.h"

dullahan::dullahan() :
    mImpl(new dullahan_impl())
{
    DLNOUT("dullahan::dullahan()");
}

dullahan::~dullahan()
{
    DLNOUT("dullahan::~dullahan()");
}

bool dullahan::init(dullahan_settings& user_settings)
{
    return mImpl->init(user_settings);
}

void dullahan::shutdown()
{
    mImpl->shutdown();
}

void dullahan::requestExit()
{
    mImpl->requestExit();
}

void dullahan::getSize(int& width, int& height)
{
    mImpl->getSize(width, height);
}

void dullahan::setSize(int width, int height)
{
    mImpl->setSize(width, height);
}

int dullahan::getDepth()
{
    return mImpl->getDepth();
}

void dullahan::run()
{
    mImpl->run();
}

void dullahan::update()
{
    mImpl->update();
}

bool dullahan::canGoBack()
{
    return mImpl->canGoBack();
}

void dullahan::goBack()
{
    mImpl->goBack();
}

bool dullahan::canGoForward()
{
    return mImpl->canGoForward();
}

void dullahan::goForward()
{
    mImpl->goForward();
}

bool dullahan::isLoading()
{
    return mImpl->isLoading();
}

void dullahan::reload(const bool ignore_cache)
{
    mImpl->reload(ignore_cache);
}

void dullahan::stop()
{
    mImpl->stop();
}

const std::string dullahan::dullahan_cef_version(bool show_bitwidth)
{
    return mImpl->dullahan_cef_version(show_bitwidth);
}

const std::string dullahan::dullahan_chrome_version(bool show_bitwidth)
{
    return mImpl->dullahan_chrome_version(show_bitwidth);
}

const std::string dullahan::dullahan_version(bool show_bitwidth)
{
    return mImpl->dullahan_version(show_bitwidth);
}

const std::string dullahan::composite_version()
{
    return mImpl->composite_version();
}

std::string dullahan::makeCompatibleUserAgentString(const std::string base)
{
    return mImpl->makeCompatibleUserAgentString(base);
}

void dullahan::mouseMove(int x, int y)
{
    mImpl->mouseMove(x, y);
}

void dullahan::mouseButton(EMouseButton mouse_button,
                           EMouseEvent mouse_event,
                           int x, int y)
{
    mImpl->mouseButton(mouse_button, mouse_event, x, y);
}

void dullahan::mouseWheel(int delta_x, int delta_y)
{
    mImpl->mouseWheel(delta_x, delta_y);
}

void dullahan::nativeKeyboardEvent(uint32_t msg,
                                   uint32_t wparam,
                                   uint64_t lparam)
{
    mImpl->nativeKeyboardEvent(msg, wparam, lparam);
}

void dullahan::navigate(const std::string url)
{
    if (url.length() > 0)
    {
        mImpl->navigate(url);
    }
}

void dullahan::setFocus()
{
    mImpl->setFocus();
}

void dullahan::setPageZoom(const double zoom_val)
{
    mImpl->setPageZoom(zoom_val);
}

bool dullahan::editCanCopy()
{
    return mImpl->editCanCopy();
}

bool dullahan::editCanCut()
{
    return mImpl->editCanCut();
}

bool dullahan::editCanPaste()
{
    return mImpl->editCanPaste();
}

void dullahan::editCopy()
{
    mImpl->editCopy();
}

void dullahan::editCut()
{
    mImpl->editCut();
}

void dullahan::editPaste()
{
    mImpl->editPaste();
}

void dullahan::showDevTools()
{
    mImpl->showDevTools();
}

void dullahan::closeDevTools()
{
    mImpl->closeDevTools();
}

void dullahan::printToPDF(const std::string path)
{
    mImpl->printToPDF(path);
}

bool dullahan::setCookie(const std::string url, const std::string name,
                         const std::string value, const std::string domain,
                         const std::string path, bool httponly, bool secure)
{
    return mImpl->setCookie(url, name, value, domain, path, httponly, secure);
}

void dullahan::deleteAllCookies()
{
    mImpl->deleteAllCookies();
}

void dullahan::postData(const std::string url,
                        const std::string data,
                        const std::string headers)
{
    mImpl->postData(url, data, headers);
}

bool dullahan::executeJavaScript(const std::string cmd)
{
    return mImpl->executeJavaScript(cmd);
}

void dullahan::showBrowserMessage(const std::string msg)
{
    mImpl->showBrowserMessage(msg);
}

void dullahan::setOnAddressChangeCallback(
    std::function<void(const std::string url)> callback)
{
    mImpl->getCallbackManager()->setOnAddressChangeCallback(callback);
}

void dullahan::setOnConsoleMessageCallback(
    std::function<void(const std::string message,
                       const std::string source, int line)> callback)
{
    mImpl->getCallbackManager()->setOnConsoleMessageCallback(callback);
}

void dullahan::setOnCursorChangedCallback(
    std::function<void(const ECursorType type)> callback)
{
    mImpl->getCallbackManager()->setOnCursorChangedCallback(callback);
}

void dullahan::setOnCustomSchemeURLCallback(
    std::function<void(const std::string url)> callback)
{
    mImpl->getCallbackManager()->setOnCustomSchemeURLCallback(callback);
}

void dullahan::setOnHTTPAuthCallback(
    std::function<bool(const std::string host,
                       const std::string realm,
                       std::string& username, std::string& password)> callback)
{
    mImpl->getCallbackManager()->setOnHTTPAuthCallback(callback);
}

void dullahan::setOnLoadEndCallback(std::function<void(int status)> callback)
{
    mImpl->getCallbackManager()->setOnLoadEndCallback(callback);
}

void dullahan::setOnLoadErrorCallback(
    std::function<void(int status, const std::string error_text)> callback)
{
    mImpl->getCallbackManager()->setOnLoadErrorCallback(callback);
}

void dullahan::setOnLoadStartCallback(std::function<void()> callback)
{
    mImpl->getCallbackManager()->setOnLoadStartCallback(callback);
}

void dullahan::setOnNavigateURLCallback(
    std::function<void(const std::string url,
                       const std::string target)> callback)
{
    mImpl->getCallbackManager()->setOnNavigateURLCallback(callback);
}

void dullahan::setOnPageChangedCallback(
    std::function<void(const unsigned char* pixels,
                       int x, int y,
                       int width, int height,
                       bool is_popup)> callback)
{
    mImpl->getCallbackManager()->setOnPageChangedCallback(callback);
}

void dullahan::setOnRequestExitCallback(std::function<void()> callback)
{
    mImpl->getCallbackManager()->setOnRequestExitCallback(callback);
}

void dullahan::setOnStatusMessageCallback(
    std::function<void(const std::string message)> callback)
{
    mImpl->getCallbackManager()->setOnStatusMessageCallback(callback);
}

void dullahan::setOnTitleChangeCallback(
    std::function<void(const std::string title)> callback)
{
    mImpl->getCallbackManager()->setOnTitleChangeCallback(callback);
}

void dullahan::setOnPdfPrintFinishedCallback(
    std::function<void(const std::string path, bool ok)> callback)
{
    mImpl->getCallbackManager()->setOnPdfPrintFinishedCallback(callback);
}

void dullahan::setOnFileDownloadCallback(
    std::function<void(const std::string filename)> callback)
{
    mImpl->getCallbackManager()->setOnFileDownloadCallback(callback);
}

void dullahan::setOnFileDialogCallback(std::function<const std::string()> callback)
{
    mImpl->getCallbackManager()->setOnFileDialogCallback(callback);
}
