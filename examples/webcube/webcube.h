/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

           Example: render web on a cube

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

#ifndef _WEBCUBE_EXAMPLE
#define _WEBCUBE_EXAMPLE

#include <iostream>
#include <windows.h>
#include <gl\gl.h>

#include "resource.h"

#include "dullahan.h"

class app
{
    public:
        app();

        void init();
        void resize_texture(int width, int height);
        void resize_window(int width, int height);
        void draw_geometry(bool pick_mode);
        void draw(bool pick_only);
        void windowPosToTexturePos(int mx, int my, int& tx, int& ty, int& tf);
        void update();
        void navigate(const std::string url);
        void navigateHome();
        void showDevTools();
        void printToPDF();
        void executeJavaScript();
        void setPageZoom(float val);
        void setPageVolume(float volume);
        void resizeBrowser(int width, int height);
        void setACookie();
        void listAllCookies();
        void deleteAllCookies();
        void initConsole();
        void closeConsole();
        void request_exit();
        void shutdown();

        HWND getURLHandle()
        {
            return mURLEntryHandle;
        }
        void setURLHandle(HWND handle)
        {
            mURLEntryHandle = handle;
        }

        void on_left_mouse_down(int x, int y, int face);
        void on_left_mouse_up(int x, int y, int face);
        void on_mouse_move(int x, int y, int face, bool left_button_down);
        void on_mouse_wheel(int x, int y, int  delta_x, int delta_y);
        void native_keyboard_event(uint32_t msg, uint64_t wparam, uint64_t lparam);

    public:
        const char* gClassName = "WebCubeClass";
        bool mMouseDragCube;
        float mMouseOffsetStartX;
        float mMouseOffsetStartY;
        float mMouseOffsetX;
        float mMouseOffsetY;
        GLfloat mCameraDist;
        GLfloat mXRotation;
        GLfloat mXRotationStart;
        GLfloat mYRotation;
        GLfloat mYRotationStart;
        int mAppWindowHeight;
        int mAppWindowPosX;
        int mAppWindowPosY;
        int mAppWindowWidth;

        static const int mMaxFaces = 6;
        int mTextureDepth;
        int mTextureHeight;
        int mTextureWidth;
        unsigned char* mTexturePixels[mMaxFaces];

        const std::string get_title();
        void set_homepage_url(std::string url);
        bool isAllFinished();

    private:
        FILE* mConsole;
        static const int mPickTextureDepth = 3;
        static const int mPickTextureHeight = 1024;
        static const int mPickTextureWidth = 1024;
        GLuint mAppTexture[mMaxFaces];
        GLuint mPickTexture[mMaxFaces];
        int mCurMouseX;
        int mCurMouseY;
        unsigned char mPickPixelColor[3];
        unsigned char mPickTexturePixels[mMaxFaces][mPickTextureWidth * mPickTextureWidth * mPickTextureDepth];
        HWND mURLEntryHandle;
        std::string mHomePageURL;
        dullahan* mDullahan;
        const std::string getHomePageURL();
        bool mAllFinished;

        void init_dullahan();

        void onAddressChange(const std::string url);
        void onConsoleMessage(const std::string message, const std::string source, int line);
        void onCursorChanged(dullahan::ECursorType type);
        void onCustomSchemeURL(const std::string url);
        const std::vector<std::string> onFileDialog(dullahan::EFileDialogType dialog_type, const std::string dialog_title, const std::string default_file, const std::string dialog_accept_filter, bool& use_default);
        void onFileDownloadProgress(int percent, bool complete);
        bool onHTTPAuth(const std::string host, const std::string realm, std::string& username, std::string& password);
        bool onJSDialogCallback(const std::string origin_url, const std::string message_text, const std::string default_prompt_text);
        bool onJSBeforeUnloadCallback();
        void onLoadEnd(int status, const std::string url);
        void onLoadError(int status, const std::string error_text);
        void onLoadStart();
        void onOpenPopup(const std::string url, const std::string target);
        void onPageChangedCallback(const unsigned char* pixels, int x, int y, const int width, const int height);
        void onPdfPrintFinished(const std::string path, bool ok);
        void onRequestExitCallback();
        void onStatusMessage(const std::string message);
        void onTitleChange(const std::string title);
        void onTooltip(const std::string title);
};

#endif // _WEBCUBE_EXAMPLE
