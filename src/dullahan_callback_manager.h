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

#ifndef _DULLAHAN_CALLBACK_MANAGER
#define _DULLAHAN_CALLBACK_MANAGER

#include <functional>

#include "dullahan.h"

class dullahan_callback_manager
{
    public:
        void setOnAddressChangeCallback(std::function<void(const std::string url)> callback);
        void onAddressChange(const std::string url);

        void setOnConsoleMessageCallback(std::function<void(const std::string message, const std::string source, int line)> callback);
        void onConsoleMessage(const std::string messgae, const std::string source, int line);

        void setOnCursorChangedCallback(std::function<void(const dullahan::ECursorType type)> callback);
        void onCursorChanged(const dullahan::ECursorType type);

        void setOnCustomSchemeURLCallback(std::function<void(const std::string url, bool gesture, bool in_redirect)> callback);
        void onCustomSchemeURL(const std::string url, bool gesture, bool in_redirect);

        void setOnHTTPAuthCallback(std::function<bool(const std::string host, const std::string realm, std::string& username, std::string& password)> callback);
        bool onHTTPAuth(const std::string host, const std::string realm, std::string& username, std::string& password);

        void setOnLoadEndCallback(std::function<void(int status, const std::string url)> callback);
        void onLoadEnd(int status, const std::string url);

        void setOnLoadErrorCallback(std::function<void(int status, const std::string error_text)> callback);
        void onLoadError(int status, const std::string error_text);

        void setOnLoadStartCallback(std::function<void()> callback);
        void onLoadStart();

        void setOnOpenPopupCallback(std::function<void(const std::string url, const std::string target)> callback);
        void onOpenPopup(const std::string url, const std::string target);

        void setOnPageChangedCallback(std::function<void(const unsigned char* pixels, int x, int y, int width, int height)> callback);
        void onPageChanged(const unsigned char* pixels, int x, int y, int width, int height);

        void setOnStatusMessageCallback(std::function<void(const std::string message)> callback);
        void onStatusMessage(const std::string message);

        void setOnRequestExitCallback(std::function<void()> callback);
        void onRequestExit();

        void setOnTitleChangeCallback(std::function<void(const std::string title)> callback);
        void onTitleChange(const std::string title);

        void setOnTooltipCallback(std::function<void(const std::string text)> callback);
        void OnTooltip(const std::string text);

        void setOnPdfPrintFinishedCallback(std::function<void(const std::string path, bool ok)> callback);
        void onPdfPrintFinished(const std::string path, bool ok);

        void setOnFileDownloadProgressCallback(std::function<void(int percent, bool complete)> callback);
        void onFileDownloadProgress(int percent, bool progress);

        void setOnFileDialogCallback(std::function<const std::vector<std::string>(dullahan::EFileDialogType dialog_type, const std::string dialog_title, const std::string default_file, const std::string dialog_accept_filter, bool& use_default)> callback);
        const std::vector<std::string> onFileDialog(dullahan::EFileDialogType dialog_type, const std::string dialog_title, const std::string default_file, const std::string dialog_accept_filter, bool& use_default);

        void setOnJSDialogCallback(std::function<bool(const std::string origin_url, const std::string message_text, const std::string default_prompt_text)> callback);
        bool onJSDialogCallback(const std::string origin_url, const std::string message_text, const std::string default_prompt_text);

        void setOnJSBeforeUnloadCallback(std::function<bool()> callback);
        bool onJSBeforeUnloadCallback();

    private:
        std::function<void(const std::string)> mOnAddressChangeCallbackFunc;
        std::function<void(const std::string, const std::string, int)> mOnConsoleMessageCallbackFunc;
        std::function<void(const dullahan::ECursorType)> mOnCursorChangedCallbackFunc;
        std::function<void(const std::string, bool, bool)> mOnCustomSchemeURLCallbackFunc;
        std::function<bool(const std::string, const std::string, std::string&, std::string&)> mOnHTTPAuthCallbackFunc;
        std::function<void(int, const std::string)> mOnLoadEndCallbackFunc;
        std::function<void(int, const std::string)> mOnLoadErrorCallbackFunc;
        std::function<void()> mOnLoadStartCallbackFunc;
        std::function<void(const std::string, const std::string)> mOnOpenPopupCallbackFunc;
        std::function<void(const unsigned char*, int, int, int, int)> mOnPageChangedCallbackFunc;
        std::function<void(const std::string)> mOnStatusMessageCallbackFunc;
        std::function<void()> mOnRequestExitCallbackFunc;
        std::function<void(const std::string)> mOnTitleChangeCallbackFunc;
        std::function<void(const std::string)> mOnTooltipCallbackFunc;
        std::function<void(const std::string, bool)> mOnPdfPrintFinishedCallbackFunc;
        std::function<void(int percent, bool complete)> mOnFileDownloadProgressCallbackFunc;
        std::function<const std::vector<std::string>(dullahan::EFileDialogType, const std::string, const std::string, const std::string, bool&)> mOnFileDialogCallbackFunc;
        std::function<bool(const std::string, const std::string, const std::string)> mOnJSDialogCallbackFunc;
        std::function<bool()> mOnJSBeforeUnloadCallbackFunc;
};

#endif //_DULLAHAN_CALLBACK_MANAGER
