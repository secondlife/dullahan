/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework
    @author Callum Prentice 2015

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

#ifndef _DULLAHAN_CALLBACK_MANAGER
#define _DULLAHAN_CALLBACK_MANAGER

#include <functional>

#include "dullahan.h"

class dullahan_callback_manager
{
    public:
        void setOnAddressChangeCallback(std::function<void(const std::string url)> callback);
        void onAddressChange(const std::string url);

        void setOnConsoleMessageCallback(std::function<void(const std::string message, const std::string source, int line)>
                                         callback);
        void onConsoleMessage(const std::string messgae, const std::string source, int line);

        void setOnCursorChangedCallback(std::function<void(const dullahan::ECursorType type)>
                                        callback);
        void onCursorChanged(const dullahan::ECursorType type);

        void setOnCustomSchemeURLCallback(std::function<void(const std::string url)> callback);
        void onCustomSchemeURL(const std::string url);

        void setOnHTTPAuthCallback(
            std::function<bool(const std::string host, const std::string realm, std::string& username, std::string& password)>
            callback);
        bool onHTTPAuth(const std::string host, const std::string realm, std::string& username, std::string& password);

        void setOnLoadEndCallback(std::function<void(int status)> callback);
        void onLoadEnd(int status);

        void setOnLoadErrorCallback(std::function<void(int status, const std::string error_text)> callback);
        void onLoadError(int status, const std::string error_text);

        void setOnLoadStartCallback(std::function<void()> callback);
        void onLoadStart();

        void setOnNavigateURLCallback(std::function<void(const std::string url, const std::string target)> callback);
        void onNavigateURL(const std::string url, const std::string target);

        void setOnPageChangedCallback(
            std::function<void(const unsigned char* pixels, int x, int y, int width, int height)> callback);
        void onPageChanged(const unsigned char* pixels, int x, int y, int width, int height);

        void setOnStatusMessageCallback(std::function<void(const std::string message)> callback);
        void onStatusMessage(const std::string message);

        void setOnRequestExitCallback(std::function<void()> callback);
        void onRequestExit();

        void setOnTitleChangeCallback(std::function<void(const std::string title)> callback);
        void onTitleChange(const std::string title);

        void setOnPdfPrintFinishedCallback(std::function<void(const std::string path, bool ok)> callback);
        void onPdfPrintFinished(const std::string path, bool ok);

        void setOnFileDownloadCallback(std::function<void(const std::string filename)> callback);
        void onFileDownload(const std::string filename);

        void setOnFileDialogCallback(std::function<const std::string()> callback);
        const std::string onFileDialog();

    private:
        std::function<void(const std::string)> mOnAddressChangeCallbackFunc;
        std::function<void(const std::string, const std::string, int)> mOnConsoleMessageCallbackFunc;
        std::function<void(const dullahan::ECursorType type)> mOnCursorChangedCallbackFunc;
        std::function<void(const std::string)> mOnCustomSchemeURLCallbackFunc;
        std::function<bool(const std::string, const std::string, std::string&, std::string&)> mOnHTTPAuthCallbackFunc;
        std::function<void(int)> mOnLoadEndCallbackFunc;
        std::function<void(int, const std::string error_text)> mOnLoadErrorCallbackFunc;
        std::function<void()> mOnLoadStartCallbackFunc;
        std::function<void(const std::string, const std::string)> mOnNavigateURLCallbackFunc;
        std::function<void(const unsigned char*, int, int, int, int)> mOnPageChangedCallbackFunc;
        std::function<void(const std::string)> mOnStatusMessageCallbackFunc;
        std::function<void()> mOnRequestExitCallbackFunc;
        std::function<void(const std::string)> mOnTitleChangeCallbackFunc;
        std::function<void(const std::string, bool)> mOnPdfPrintFinishedCallbackFunc;
        std::function<void(const std::string)> mOnFileDownloadCallbackFunc;
        std::function<const std::string()> mOnFileDialogCallbackFunc;
};

#endif //_DULLAHAN_CALLBACK_MANAGER
