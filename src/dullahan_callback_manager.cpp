/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework
    @author Callum Prentice 2015

    LICENSE FILE TO GO HERE
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
    std::function<void(const std::string message, const std::string source, int line)>
    callback)
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
    std::function<void(const dullahan::ECursorType type, const unsigned int cursor)> callback)
{
    mOnCursorChangedCallbackFunc = callback;
}

void dullahan_callback_manager::onCursorChanged(const dullahan::ECursorType type, const unsigned int cursor)
{
    if (mOnCursorChangedCallbackFunc)
    {
        mOnCursorChangedCallbackFunc(type, cursor);
    }
}

void dullahan_callback_manager::setOnCustomSchemeURLCallback(std::function<void(const std::string url)> callback)
{
    mOnCustomSchemeURLCallbackFunc = callback;
}

void dullahan_callback_manager::onCustomSchemeURL(const std::string url)
{
    if (mOnCustomSchemeURLCallbackFunc)
    {
        mOnCustomSchemeURLCallbackFunc(url);
    }
}

void dullahan_callback_manager::setOnHTTPAuthCallback(
    std::function<bool(const std::string host, const std::string realm, std::string& username, std::string& password)>
    callback)
{
    mOnHTTPAuthCallbackFunc = callback;
}

bool dullahan_callback_manager::onHTTPAuth(const std::string host, const std::string realm, std::string& username,
        std::string& password)
{
    if (mOnHTTPAuthCallbackFunc)
    {
        return mOnHTTPAuthCallbackFunc(host, realm, username, password);
    }

    // default to cancel request if no callback set up
    return false;
}

void dullahan_callback_manager::setOnLoadEndCallback(std::function<void(int status)> callback)
{
    mOnLoadEndCallbackFunc = callback;
}

void dullahan_callback_manager::onLoadEnd(int status)
{
    if (mOnLoadEndCallbackFunc)
    {
        mOnLoadEndCallbackFunc(status);
    }
}

void dullahan_callback_manager::setOnLoadErrorCallback(std::function<void(int status, const std::string error_text)>
        callback)
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

void dullahan_callback_manager::setOnNavigateURLCallback(
    std::function<void(const std::string url, const std::string target)> callback)
{
    mOnNavigateURLCallbackFunc = callback;
}

void dullahan_callback_manager::onNavigateURL(const std::string url, const std::string target)
{
    if (mOnNavigateURLCallbackFunc)
    {
        mOnNavigateURLCallbackFunc(url, target);
    }
}

void dullahan_callback_manager::setOnPageChangedCallback(
    std::function<void(const unsigned char* pixels, int x, int y, int width, int height, bool is_popup)> callback)
{
    mOnPageChangedCallbackFunc = callback;
}

void dullahan_callback_manager::onPageChanged(const unsigned char* pixels, int x, int y, int width, int height,
        bool is_popup)
{
    if (mOnPageChangedCallbackFunc)
    {
        mOnPageChangedCallbackFunc(pixels, x, y, width, height, is_popup);
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

void dullahan_callback_manager::setOnFileDownloadCallback(std::function<void(const std::string filename)> callback)
{
    mOnFileDownloadCallbackFunc = callback;
}

void dullahan_callback_manager::onFileDownload(const std::string filename)
{
    if (mOnFileDownloadCallbackFunc)
    {
        mOnFileDownloadCallbackFunc(filename);
    }
}

void dullahan_callback_manager::setOnFileDialogCallback(std::function<const std::string()> callback)
{
    mOnFileDialogCallbackFunc = callback;
}

const std::string dullahan_callback_manager::onFileDialog()
{
    if (mOnFileDialogCallbackFunc)
    {
        return mOnFileDialogCallbackFunc();
    }

    return std::string();
}
