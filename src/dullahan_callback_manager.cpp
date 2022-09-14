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

#include "dullahan_impl.h"
#include "dullahan_callback_manager.h"

void dullahan_callback_manager::setOnAddressChangeCallback(std::function<void(const std::string url)> callback)
{
    mOnAddressChangeCallbackFunc = callback;
}

void dullahan_callback_manager::onAddressChange(const std::string url)
{
    if (mOnAddressChangeCallbackFunc)
    {
        mOnAddressChangeCallbackFunc(url);
    }
}

void dullahan_callback_manager::setOnConsoleMessageCallback(
    std::function<void(const std::string message, const std::string source, int line)> callback)
{
    mOnConsoleMessageCallbackFunc = callback;
}

void dullahan_callback_manager::onConsoleMessage(const std::string message, const std::string source, int line)
{
    if (mOnConsoleMessageCallbackFunc)
    {
        mOnConsoleMessageCallbackFunc(message, source, line);
    }
}

void dullahan_callback_manager::setOnCursorChangedCallback(
    std::function<void(const dullahan::ECursorType type)> callback)
{
    mOnCursorChangedCallbackFunc = callback;
}

void dullahan_callback_manager::onCursorChanged(const dullahan::ECursorType type)
{
    if (mOnCursorChangedCallbackFunc)
    {
        mOnCursorChangedCallbackFunc(type);
    }
}

void dullahan_callback_manager::setOnCustomSchemeURLCallback(std::function<void(const std::string url, bool user_gesture, bool is_redirec)> callback)
{
    mOnCustomSchemeURLCallbackFunc = callback;
}

void dullahan_callback_manager::onCustomSchemeURL(const std::string url, bool user_gesture, bool is_redirect)
{
    if (mOnCustomSchemeURLCallbackFunc)
    {
        mOnCustomSchemeURLCallbackFunc(url, user_gesture, is_redirect);
    }
}

void dullahan_callback_manager::setOnHTTPAuthCallback(
    std::function<bool(const std::string host, const std::string realm, std::string& username, std::string& password)> callback)
{
    mOnHTTPAuthCallbackFunc = callback;
}

bool dullahan_callback_manager::onHTTPAuth(const std::string host, const std::string realm, std::string& username, std::string& password)
{
    if (mOnHTTPAuthCallbackFunc)
    {
        return mOnHTTPAuthCallbackFunc(host, realm, username, password);
    }

    // default to cancel request if no callback set up
    return false;
}

void dullahan_callback_manager::setOnLoadEndCallback(std::function<void(int status, const std::string url)> callback)
{
    mOnLoadEndCallbackFunc = callback;
}

void dullahan_callback_manager::onLoadEnd(int status, const std::string url)
{
    if (mOnLoadEndCallbackFunc)
    {
        mOnLoadEndCallbackFunc(status, url);
    }
}

void dullahan_callback_manager::setOnLoadErrorCallback(std::function<void(int status, const std::string error_text)> callback)
{
    mOnLoadErrorCallbackFunc = callback;
}

void dullahan_callback_manager::onLoadError(int status, const std::string error_text)
{
    if (mOnLoadErrorCallbackFunc)
    {
        mOnLoadErrorCallbackFunc(status, error_text);
    }
}

void dullahan_callback_manager::setOnLoadStartCallback(std::function<void()> callback)
{
    mOnLoadStartCallbackFunc = callback;
}

void dullahan_callback_manager::onLoadStart()
{
    if (mOnLoadStartCallbackFunc)
    {
        mOnLoadStartCallbackFunc();
    }
}

void dullahan_callback_manager::setOnOpenPopupCallback(
    std::function<void(const std::string url, const std::string target)> callback)
{
    mOnOpenPopupCallbackFunc = callback;
}

void dullahan_callback_manager::onOpenPopup(const std::string url, const std::string target)
{
    if (mOnOpenPopupCallbackFunc)
    {
        mOnOpenPopupCallbackFunc(url, target);
    }
}

void dullahan_callback_manager::setOnPageChangedCallback(
    std::function<void(const unsigned char* pixels, int x, int y, int width, int height)> callback)
{
    mOnPageChangedCallbackFunc = callback;
}

void dullahan_callback_manager::onPageChanged(const unsigned char* pixels, int x, int y, int width, int height)
{
    if (mOnPageChangedCallbackFunc)
    {
        mOnPageChangedCallbackFunc(pixels, x, y, width, height);
    }
}

void dullahan_callback_manager::setOnStatusMessageCallback(std::function<void(const std::string message)> callback)
{
    mOnStatusMessageCallbackFunc = callback;
}

void dullahan_callback_manager::onStatusMessage(const std::string message)
{
    if (mOnStatusMessageCallbackFunc)
    {
        mOnStatusMessageCallbackFunc(message);
    }
}

void dullahan_callback_manager::setOnRequestExitCallback(std::function<void()> callback)
{
    mOnRequestExitCallbackFunc = callback;
}

void dullahan_callback_manager::onRequestExit()
{
    if (mOnRequestExitCallbackFunc)
    {
        mOnRequestExitCallbackFunc();
    }
}

void dullahan_callback_manager::setOnTitleChangeCallback(std::function<void(const std::string title)> callback)
{
    mOnTitleChangeCallbackFunc = callback;
}

void dullahan_callback_manager::onTitleChange(const std::string title)
{
    if (mOnTitleChangeCallbackFunc)
    {
        mOnTitleChangeCallbackFunc(title);
    }
}

void dullahan_callback_manager::setOnTooltipCallback(std::function<void(const std::string text)> callback)
{
    mOnTooltipCallbackFunc = callback;
}

void dullahan_callback_manager::OnTooltip(const std::string text)
{
    if (mOnTooltipCallbackFunc)
    {
        mOnTooltipCallbackFunc(text);
    }
}

void dullahan_callback_manager::setOnPdfPrintFinishedCallback(std::function<void(const std::string path, bool ok)> callback)
{
    mOnPdfPrintFinishedCallbackFunc = callback;
}

void dullahan_callback_manager::onPdfPrintFinished(const std::string path, bool ok)
{
    if (mOnPdfPrintFinishedCallbackFunc)
    {
        mOnPdfPrintFinishedCallbackFunc(path, ok);
    }
}

void dullahan_callback_manager::setOnFileDownloadProgressCallback(std::function<void(int percent, bool complete)> callback)
{
    mOnFileDownloadProgressCallbackFunc = callback;
}

void dullahan_callback_manager::onFileDownloadProgress(int percent, bool level)
{
    if (mOnFileDownloadProgressCallbackFunc)
    {
        mOnFileDownloadProgressCallbackFunc(percent, level);
    }
}

void dullahan_callback_manager::setOnFileDialogCallback(std::function<const std::vector<std::string>(dullahan::EFileDialogType dialog_type, const std::string dialog_title, const std::string default_file, const std::string dialog_accept_filter, bool& use_default)> callback)
{
    mOnFileDialogCallbackFunc = callback;
}

const std::vector<std::string> dullahan_callback_manager::onFileDialog(dullahan::EFileDialogType dialog_type, const std::string dialog_title, const std::string default_file, const std::string dialog_accept_filter, bool& use_default)
{
    if (mOnFileDialogCallbackFunc)
    {
        return mOnFileDialogCallbackFunc(dialog_type, dialog_title, default_file, dialog_accept_filter, use_default);
    }

    return std::vector<std::string>();
}

void dullahan_callback_manager::setOnJSDialogCallback(
    std::function<bool(const std::string origin_url, const std::string message_text, const std::string default_prompt_text)> callback)
{
    mOnJSDialogCallbackFunc = callback;
}

bool dullahan_callback_manager::onJSDialogCallback(const std::string origin_url, const std::string message_text, const std::string default_prompt_text)
{
    if (mOnJSDialogCallbackFunc)
    {
        return mOnJSDialogCallbackFunc(origin_url, message_text, default_prompt_text);
    }

    // default to cancel request if no callback set up
    return false;
}

void dullahan_callback_manager::setOnJSBeforeUnloadCallback(
    std::function<bool()> callback)
{
    mOnJSBeforeUnloadCallbackFunc = callback;
}

bool dullahan_callback_manager::onJSBeforeUnloadCallback()
{
    if (mOnJSBeforeUnloadCallbackFunc)
    {
        return mOnJSBeforeUnloadCallbackFunc();
    }

    // default to cancel request if no callback set up
    return false;
}

