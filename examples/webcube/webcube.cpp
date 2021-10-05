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

#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>
#include <string>
#include <ctime>

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>

#include <gl\gl.h>
#include <gl\glu.h>

#include "resource.h"

#include "webcube.h"

app* gApp = 0;

/////////////////////////////////////////////////////////////////////////////////
//
app::app()
{
    mAppWindowPosX = 800;
    mAppWindowPosY = 0;
    mAppWindowWidth = 1500;
    mAppWindowHeight = 1500;
    mTextureDepth = 4;
    mMouseOffsetX = 0.0;
    mMouseOffsetY = 0.0;
    mMouseOffsetStartX = 0.0;
    mMouseOffsetStartY = 0.0;
    mMouseDragCube = false;
    mXRotationStart = 0;
    mXRotation = -17.2f;
    mYRotationStart = 0;
    mYRotation = -21.6f;
    mCameraDist = -4.0;

    mTextureWidth = 1024;
    mTextureHeight = 1024;
    mAllFinished = false;

    // empty string means go load the page of test URLs
    // may be overridden by command line arg --homepage="URL"
    mHomePageURL = "";
}

/////////////////////////////////////////////////////////////////////////////////
//
const std::string app::get_title()
{
    return "Web Cube; a Dullahan example";
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::set_homepage_url(std::string url)
{
    mHomePageURL = url;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::init_dullahan()
{
    mDullahan = new dullahan();

    std::cout << "Version: " << mDullahan->composite_version() << std::endl;

    mDullahan->setOnAddressChangeCallback(std::bind(&app::onAddressChange, this, std::placeholders::_1));
    mDullahan->setOnConsoleMessageCallback(std::bind(&app::onConsoleMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    mDullahan->setOnCursorChangedCallback(std::bind(&app::onCursorChanged, this, std::placeholders::_1));
    mDullahan->setOnCustomSchemeURLCallback(std::bind(&app::onCustomSchemeURL, this, std::placeholders::_1));
    mDullahan->setOnFileDialogCallback(std::bind(&app::onFileDialog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    mDullahan->setOnFileDownloadProgressCallback(std::bind(&app::onFileDownloadProgress, this, std::placeholders::_1, std::placeholders::_2));
    mDullahan->setOnHTTPAuthCallback(std::bind(&app::onHTTPAuth, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    mDullahan->setOnLoadEndCallback(std::bind(&app::onLoadEnd, this, std::placeholders::_1, std::placeholders::_2));
    mDullahan->setOnLoadErrorCallback(std::bind(&app::onLoadError, this, std::placeholders::_1, std::placeholders::_2));
    mDullahan->setOnLoadStartCallback(std::bind(&app::onLoadStart, this));
    mDullahan->setOnOpenPopupCallback(std::bind(&app::onOpenPopup, this, std::placeholders::_1, std::placeholders::_2));
    mDullahan->setOnPageChangedCallback(std::bind(&app::onPageChangedCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    mDullahan->setOnPdfPrintFinishedCallback(std::bind(&app::onPdfPrintFinished, this, std::placeholders::_1, std::placeholders::_2));
    mDullahan->setOnRequestExitCallback(std::bind(&app::onRequestExitCallback, this));
    mDullahan->setOnStatusMessageCallback(std::bind(&app::onStatusMessage, this, std::placeholders::_1));
    mDullahan->setOnTitleChangeCallback(std::bind(&app::onTitleChange, this, std::placeholders::_1));
    mDullahan->setOnJSDialogCallback(std::bind(&app::onJSDialogCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    mDullahan->setOnJSBeforeUnloadCallback(std::bind(&app::onJSBeforeUnloadCallback, this));
    mDullahan->setOnTooltipCallback(std::bind(&app::onTooltip, this, std::placeholders::_1));

    std::vector<std::string> custom_schemes(1, "secondlife");
    mDullahan->setCustomSchemes(custom_schemes);

    dullahan::dullahan_settings settings;
    settings.host_process_path = "";  // implies host process is next to executable
    settings.accept_language_list = "en-US";
    settings.proxy_host_port = std::string();
    settings.background_color = 0xff666666;
    settings.cache_enabled = true;
    settings.locales_dir_path = "";
    settings.cache_path = ".\\webcube_cache";
    settings.cookies_enabled = true;
    settings.disable_gpu = false;
    settings.disable_network_service = false;
    settings.disable_web_security = false;
    settings.file_access_from_file_urls = false;
    settings.flash_enabled = false;
    settings.flip_mouse_y = false;
    settings.flip_pixels_y = false;
    settings.force_wave_audio = true;
    settings.frame_rate = 60;
    settings.initial_height = mTextureWidth;
    settings.initial_width = mTextureHeight;
    settings.java_enabled = false;
    settings.javascript_enabled = true;
    settings.media_stream_enabled = true;
    settings.plugins_enabled = true;
    settings.user_agent_substring = mDullahan->makeCompatibleUserAgentString(get_title());
    settings.webgl_enabled = true;
    settings.log_file = "webcube_cef_log.txt";
    settings.log_verbose = true;
    settings.fake_ui_for_media_stream = true;

    bool result = mDullahan->init(settings);
    if (result)
    {
        mDullahan->setPageZoom(1.0);
        navigateHome();
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::init()
{
    init_dullahan();

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // create the red/blue texture used in picking
    glGenTextures(mMaxFaces, &mPickTexture[0]);
    for (unsigned int f = 0; f < mMaxFaces; ++f)
    {
        for (unsigned int y = 0; y < mPickTextureHeight; ++y)
        {
            for (unsigned int x = 0; x < mPickTextureWidth; ++x)
            {
                unsigned long mask = (x << 14) | (y << 4) | f;

                mPickTexturePixels[f][y * mPickTextureWidth * mPickTextureDepth + x * mPickTextureDepth + 0] = mask & 0xff;
                mPickTexturePixels[f][y * mPickTextureWidth * mPickTextureDepth + x * mPickTextureDepth + 1] = (mask >> 8) & 0xff;
                mPickTexturePixels[f][y * mPickTextureWidth * mPickTextureDepth + x * mPickTextureDepth + 2] = (mask >> 16) & 0xff;
            };
        }

        glBindTexture(GL_TEXTURE_2D, mPickTexture[f]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mPickTextureWidth, mPickTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mPickTexturePixels[f]);
    }

    // texture used for application
    glGenTextures(mMaxFaces, &mAppTexture[0]);

    srand((unsigned int)time(0));

    resize_texture(mTextureWidth, mTextureHeight);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::resize_texture(int width, int height)
{
    for (int i = 0; i < mMaxFaces; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, mAppTexture[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mTextureWidth, mTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }

    mDullahan->setSize(width, height);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::resize_window(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    mAppWindowWidth = width;
    mAppWindowHeight = height;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::request_exit()
{
    mDullahan->requestExit();
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::shutdown()
{
    mDullahan->shutdown();
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::draw_geometry(bool pick_mode)
{
    static GLfloat textureCoords[mMaxFaces * 8] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f
    };

    static GLfloat vertexPos[mMaxFaces * 12] =
    {
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
        };

    const GLfloat delta = 1.0f;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int face = 0; face < mMaxFaces; ++face)
    {
        if (pick_mode)
        {
            glBindTexture(GL_TEXTURE_2D, mPickTexture[face]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, mAppTexture[face]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        glBegin(GL_QUADS);
        for (int corner = 0; corner < 4; ++corner)
        {
            int cur_tc = 4 * face + corner;
            glTexCoord2f(textureCoords[cur_tc * 2 + 0], textureCoords[cur_tc * 2 + 1]);

            int cur_vp = 4 * face + corner;
            glVertex3f(vertexPos[cur_vp * 3 + 0] * delta, vertexPos[cur_vp * 3 + 1] * delta, vertexPos[cur_vp * 3 + 2] * delta);
        };
        glEnd();

        if (pick_mode)
        {
            glReadPixels(mCurMouseX, mCurMouseY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, mPickPixelColor);
        }
    };

    if (! pick_mode)
    {
        glPolygonMode(GL_FRONT, GL_LINE);
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.3f, 0.9f, 0.2f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        for (int face = 0; face < mMaxFaces; ++face)
        {
            for (int corner = 0; corner < 4; ++corner)
            {
                int cur_vp = 4 * face + corner;
                glVertex3f(vertexPos[cur_vp * 3 + 0] * delta, vertexPos[cur_vp * 3 + 1] * delta, vertexPos[cur_vp * 3 + 2] * delta);
            };
        };
        glVertex3f(-1.0f * delta, -1.0f * delta, 1.0f * delta);
        glVertex3f(-1.0f * delta, 1.0f * delta, 1.0f * delta);
        glVertex3f(1.0f * delta, -1.0f * delta, 1.0f * delta);
        glVertex3f(1.0f * delta, 1.0f * delta, 1.0f * delta);
        glVertex3f(1.0f * delta, -1.0f * delta, -1.0f * delta);
        glVertex3f(1.0f * delta, 1.0f * delta, -1.0f * delta);
        glVertex3f(-1.0f * delta, -1.0f * delta, -1.0f * delta);
        glVertex3f(-1.0f * delta, 1.0f * delta, -1.0f * delta);
        glEnd();
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::draw(bool pick_only)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, mCameraDist);

    glRotatef(mXRotation, 1.0f, 0.0f, 0.0f);
    glRotatef(mYRotation, 0.0f, 1.0f, 0.0f);
    glRotatef(0.0f, 0.0f, 0.0f, 1.0f);

    bool pick_mode = true;
    draw_geometry(pick_mode);

    pick_mode = false;
    draw_geometry(pick_mode);

    if (mMouseDragCube)
    {
        mYRotation = mYRotationStart + (mMouseOffsetX - mMouseOffsetStartX) * 360.0f;
        mXRotation = mXRotationStart + (mMouseOffsetY - mMouseOffsetStartY) * 360.0f;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
void app::windowPosToTexturePos(int mx, int my, int& tx, int& ty, int& tf)
{
    mCurMouseX = mx;
    mCurMouseY = mAppWindowHeight - my;

    const bool pick_only = true;
    draw(pick_only);

    tx = (mPickPixelColor[2] << 2) | (mPickPixelColor[1] >> 6);
    ty = ((mPickPixelColor[1] & 0x3f) << 4) | (mPickPixelColor[0] >> 4);
    tf = mPickPixelColor[0] & 0x0f;

    // scale pick texture coordinates to dimensions of app texture
    tx = (int)((float)tx * (float)mTextureWidth / (float)mPickTextureWidth);
    ty = (int)((float)ty * (float)mTextureHeight / (float)mPickTextureHeight);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::update()
{
    mDullahan->update();

    const bool pick_only = false;
    draw(pick_only);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::navigate(const std::string url)
{
    mDullahan->navigate(url);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::navigateHome()
{
    mDullahan->navigate(getHomePageURL());
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::showDevTools()
{
    mDullahan->showDevTools();
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::printToPDF()
{
    const std::string filename = "page.pdf";

    mDullahan->printToPDF(filename);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::executeJavaScript()
{
    mDullahan->executeJavaScript("alert('hello world')");
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::setPageZoom(float val)
{
    mDullahan->setPageZoom(val);
}

/////////////////////////////////////////////////////////////////////////////////
// Volume is 0.0 to 1.0
void app::setPageVolume(float volume)
{
    DWORD left_channel = (DWORD)(volume * 65535.0f);
    DWORD right_channel = (DWORD)(volume * 65535.0f);
    DWORD hw_volume = left_channel << 16 | right_channel;

    ::waveOutSetVolume(nullptr, hw_volume);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::resizeBrowser(int width, int height)
{
    mTextureWidth = width;
    mTextureHeight = height;

    resize_texture(width, height);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::setACookie()
{
    const std::string url("https://id.callum.com");
    const std::string name("best_cookies");
    const std::string value("hobnobs");
    const std::string domain("id.callum.com");
    const std::string path("/");
    const bool httponly = false;
    const bool secure = false;

    std::cout << "Setting cookie called " << name << " to value " << value << std::endl;

    mDullahan->setCookie(url, name, value, domain, path, httponly, secure);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::listAllCookies()
{
    std::cout << "Listing all cookies" << std::endl;

    std::vector<std::string> cookies = mDullahan->getCookies();

    std::cout << "Number of cookies is " << cookies.size() << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::deleteAllCookies()
{
    std::cout << "Deleting all cookies" << std::endl;

    mDullahan->deleteAllCookies();
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onAddressChange(const std::string url)
{
    std::cout << "onAddressChange: " << url << std::endl;
    SendMessage(mURLEntryHandle, WM_SETTEXT, 0, (LPARAM)url.c_str());
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onConsoleMessage(const std::string message, const std::string source, int line)
{
    std::cout << "onConsoleMessage: " << message << " " << source << " #" << line << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onCursorChanged(dullahan::ECursorType type)
{
    std::cout << "onCursorChanged: " << type << std::endl;

    if (type == dullahan::CT_POINTER)
    {
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
    }
    else if (type == dullahan::CT_HAND)
    {
        SetCursor(LoadCursor(nullptr, IDC_HAND));
    }
    else if (type == dullahan::CT_IBEAM)
    {
        SetCursor(LoadCursor(nullptr, IDC_IBEAM));
    }
    else if (type == dullahan::CT_MOVE)
    {
        SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onCustomSchemeURL(const std::string url)
{
    std::cout << "onCustomSchemeURL triggered in app: " << url << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
const std::vector<std::string> app::onFileDialog(dullahan::EFileDialogType dialog_type, const std::string dialog_title, const std::string default_file, std::string dialog_accept_filter, bool& use_default)
{
    if (dialog_type == dullahan::FD_OPEN_FILE)
    {
        std::cout << "onFileDialog(..) open file: ";
    }
    else if (dialog_type == dullahan::FD_SAVE_FILE)
    {
        std::cout << "onFileDialog(..) save file ";
    }
    std::cout << "title: " << "\"" << dialog_title << "\"";
    std::cout << " | ";
    std::cout << "filename: " << "\"" << default_file << "\"";
    std::cout << " | ";
    std::cout << "filter: " << "\"" << dialog_accept_filter << "\"";
    std::cout << std::endl;

    // test directly download with no file dialog and user specified name
    const bool download_directly = false;
    if (download_directly)
    {
        use_default = false;
        char desktop_path[MAX_PATH + 1];
        SHGetFolderPath(nullptr, CSIDL_DESKTOP, nullptr, 0, desktop_path);

        std::vector<std::string> download_paths { std::string(desktop_path) + "\\dullahan_webcube.unknown" };

        std::cout << "Downloading directly to " << download_paths[0] << std::endl;

        return download_paths;
    }

    // test internal CEF file dialog and file dialog implemented here
    const bool use_default_file_dialog = false;
    use_default = use_default_file_dialog;

    if (use_default_file_dialog)
    {
        std::cout << "onFileDialog(..) - using default CEF dialog" << std::endl;
        return std::vector<std::string>();
    }

    std::cout << "onFileDialog(..) - using our own dialog" << std::endl;
    if (dialog_type == dullahan::FD_OPEN_FILE)
    {
        OPENFILENAME ofn;
        char szFile[MAX_PATH];
        ZeroMemory(szFile, MAX_PATH);
        memcpy(szFile, default_file.c_str(), default_file.length());
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetDesktopWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = (char*)dialog_accept_filter.c_str();
        ofn.nFilterIndex = (DWORD)dialog_accept_filter.length();
        ofn.lpstrFileTitle = (char*)dialog_title.c_str();
        ofn.nMaxFileTitle = (DWORD)dialog_title.length();
        ofn.lpstrInitialDir = nullptr;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

        if (GetOpenFileName(&ofn) == TRUE)
        {
            std::vector<std::string> download_paths{ std::string(ofn.lpstrFile) };
            return download_paths;
        }
        else
        {
            return std::vector<std::string>();
        }
    }
    if (dialog_type == dullahan::FD_OPEN_MULTIPLE_FILES)
    {
        OPENFILENAME ofn;
        char szFile[MAX_PATH  * 10];
        ZeroMemory(szFile, MAX_PATH * 10);
        memcpy(szFile, default_file.c_str(), default_file.length());
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetDesktopWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = (char*)dialog_accept_filter.c_str();
        ofn.nFilterIndex = (DWORD)dialog_accept_filter.length();
        ofn.lpstrFileTitle = (char*)dialog_title.c_str();
        ofn.nMaxFileTitle = (DWORD)dialog_title.length();
        ofn.lpstrInitialDir = nullptr;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

        if (GetOpenFileName(&ofn) == TRUE)
        {
            std::vector<std::string> download_paths;

            char* str = ofn.lpstrFile;
            std::string directory = str;
            str += (directory.length() + 1);
            while (*str)
            {
                std::string filename = str;
                download_paths.push_back(directory + "\\" + str);
                str += (filename.length() + 1);
            }

            return download_paths;
        }
        else
        {
            return std::vector<std::string>();
        }
    }
    else if (dialog_type == dullahan::FD_SAVE_FILE)
    {
        OPENFILENAME ofn;
        char szFile[MAX_PATH];
        ZeroMemory(szFile, MAX_PATH);
        memcpy(szFile, default_file.c_str(), default_file.length());
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetDesktopWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = (char*)dialog_accept_filter.c_str();
        ofn.nFilterIndex = 0;
        ofn.lpstrFileTitle = (char*)dialog_title.c_str();
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = nullptr;
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (GetSaveFileName(&ofn) == TRUE)
        {
            std::vector<std::string> download_paths{ std::string(ofn.lpstrFile) };
            return download_paths;
        }
        else
        {
            return std::vector<std::string>();
        }
    }
    else
    {
        return std::vector<std::string>();
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onFileDownloadProgress(int percent, bool complete)
{
    std::cout << "Download progress: " << percent << "%" << " --- complete: " << complete << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
bool app::onHTTPAuth(const std::string host, const std::string realm, std::string& username, std::string& password)
{
    std::cout << "onHTTPAuth: " << host << " " << realm << std::endl;
    username = "user";
    password = "passwd";
    return true;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onLoadEnd(int status, const std::string url)
{
    std::cout << "onLoadEnd: " << status << " --- URL: " << url << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onLoadError(int status, const std::string error_text)
{
    std::stringstream msg;

    msg << "<b>Loading error!</b>";
    msg << "<p>";
    msg << "Message: " << error_text;
    msg << "<br>";
    msg << "Code: " << status;

    mDullahan->showBrowserMessage(msg.str());
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onLoadStart()
{
    std::cout << "onLoadStart: " << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onOpenPopup(const std::string url, const std::string target)
{
    std::cout << "OnOpenPopup: " << url << " " << target << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onPageChangedCallback(const unsigned char* pixels, int x, int y,
                                const int width, const int height)

{
    for (int face = 0; face < mMaxFaces; ++face)
    {
        glBindTexture(GL_TEXTURE_2D, mAppTexture[face]);

        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        x, y,
                        width, height,
                        GL_BGRA_EXT,
                        GL_UNSIGNED_BYTE,
                        pixels);
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
bool app::onJSDialogCallback(const std::string origin_url, const std::string message_text, const std::string default_prompt_text)
{
    std::cout << "JavaScript dialog triggered:" << std::endl;
    std::cout << "    URL: " << origin_url << std::endl;
    std::cout << "    message: " << message_text << std::endl;
    std::cout << "    prompt: " << default_prompt_text << std::endl;

    // cancel the dialog automatically
    return true;
}

/////////////////////////////////////////////////////////////////////////////////
//
bool app::onJSBeforeUnloadCallback()
{
    std::cout << "JavaScript alert before page unloaded triggered:" << std::endl;

    // cancel the dialog automatically
    return true;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onPdfPrintFinished(const std::string path, bool ok)
{
    if (ok)
    {
        std::cout << "onPdfPrintFinished: " << path << "  " << ok << std::endl;
        ShellExecute(nullptr, "open", path.c_str(), nullptr, ".", SW_SHOWNORMAL);
    }
    else
    {
        std::cout << "Error: Unable to open PDF file" << std::endl;
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onRequestExitCallback()
{
    mAllFinished = true;

    // posted a quit message no longer works as of CEF 74. Not sure why - I
    // think it swallows the message before the app (like this one) gets to
    // see it. Now we need to use an explicit flag
    PostQuitMessage(0);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onStatusMessage(const std::string message)
{
    std::cout << "onStatusMessage: " << message << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onTitleChange(const std::string title)
{
    std::cout << "onTitleChange: " << title << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::onTooltip(const std::string tooltip)
{
    std::cout << "onTooltip: " << tooltip << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::on_left_mouse_down(int x, int y, int face)
{
    mDullahan->setFocus();
    mDullahan->mouseButton(dullahan::MB_MOUSE_BUTTON_LEFT, dullahan::ME_MOUSE_DOWN, x, y);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::on_left_mouse_up(int x, int y, int face)
{
    mDullahan->mouseButton(dullahan::MB_MOUSE_BUTTON_LEFT, dullahan::ME_MOUSE_UP, x, y);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::on_mouse_move(int x, int y, int face, bool left_button_down)
{
    mDullahan->mouseMove(x, y);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::on_mouse_wheel(int x, int y, int delta_x, int delta_y)
{
    mDullahan->mouseWheel(x, y, delta_x, delta_y);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::native_keyboard_event(uint32_t msg, uint64_t wparam, uint64_t lparam)
{
    mDullahan->nativeKeyboardEventWin(msg, (uint32_t)wparam, lparam);
}

/////////////////////////////////////////////////////////////////////////////////
//
LRESULT CALLBACK edit_sub_class_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_CHAR && wParam == VK_RETURN)
    {
        char buf[2048];
        GetWindowText(hWnd, buf, 1024);
        gApp->navigate(buf);
        return 0;
    }

    LRESULT lRes = DefSubclassProc(hWnd, uMsg, wParam, lParam);
    if (uMsg == WM_DESTROY)
    {
        RemoveWindowSubclass(hWnd, edit_sub_class_proc, 0);
    }
    return lRes;
}

/////////////////////////////////////////////////////////////////////////////////
//
void create_url_entry_ui(HWND parent)
{
    HINSTANCE instance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);

    HWND hwnd_control = CreateWindowEx(0, "EDIT", nullptr, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOVSCROLL, 0, 0, 0, 40, parent,
                                       (HMENU)nullptr, instance, nullptr);

    SendMessage(hwnd_control, EM_SETMARGINS, EC_LEFTMARGIN, MAKELPARAM(8, 0));
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                             CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("MS Sans Serif"));
    SendMessage(hwnd_control, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    gApp->setURLHandle(hwnd_control);

    SetWindowSubclass(hwnd_control, edit_sub_class_proc, 0, 0);

}

/////////////////////////////////////////////////////////////////////////////////
//
LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            create_url_entry_ui(hWnd);
        }
        break;

        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

            switch (wmId)
            {
                case ID_FEATURES_REQUEST_EXIT:
                    gApp->request_exit();
                    break;

                case ID_FEATURES_NAVIGATE_HOME:
                    gApp->navigateHome();
                    break;

                case ID_FEATURES_DEVELOPER_CONSOLE:
                    gApp->showDevTools();
                    break;

                case ID_FEATURES_PRINT_TO_PDF:
                    gApp->printToPDF();
                    break;

                case ID_FEATURES_EXECUTE_JAVASCRIPT:
                    gApp->executeJavaScript();
                    break;

                case ID_FEATURES_ZOOM_05X:
                    gApp->setPageZoom(0.1f);
                    break;

                case ID_FEATURES_ZOOM_1X:
                    gApp->setPageZoom(1.0f);
                    break;

                case ID_FEATURES_ZOOM_2X:
                    gApp->setPageZoom(2.0f);
                    break;

                case ID_FEATURES_ZOOM_5X:
                    gApp->setPageZoom(5.0f);
                    break;

                case ID_FEATURES_VOLUME_01X:
                    gApp->setPageVolume(0.1f);
                    break;

                case ID_FEATURES_VOLUME_00X:
                    gApp->setPageVolume(0.0f);
                    break;

                case ID_FEATURES_VOLUME_05X:
                    gApp->setPageVolume(0.5f);
                    break;

                case ID_FEATURES_VOLUME_10X:
                    gApp->setPageVolume(1.0f);
                    break;

                case ID_FEATURES_RESIZE_BROWSER_SMALL:
                    gApp->resizeBrowser(300, 300);
                    break;

                case ID_FEATURES_RESIZE_BROWSER_MEDIUM:
                    gApp->resizeBrowser(800, 800);
                    break;

                case ID_FEATURES_RESIZE_BROWSER_LARGE:
                    gApp->resizeBrowser(1400, 1400);
                    break;

                case ID_FEATURES_COOKIES_SET_A_COOKIE:
                    gApp->setACookie();
                    break;

                case ID_FEATURES_COOKIES_LIST_ALL_COOKIES:
                    gApp->listAllCookies();
                    break;

                case ID_FEATURES_COOKIES_DELETE_ALL_COOKIES:
                    gApp->deleteAllCookies();
                    break;

                default:
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
        }
        break;

        case WM_CHAR:
        {
            if (wParam == 27)
            {
                gApp->request_exit();
            }
            else
            {
                gApp->native_keyboard_event(uMsg, (uint32_t)wParam, lParam);
            }
            return 0;
        };

        case WM_KEYDOWN:
        {
            gApp->native_keyboard_event(uMsg, wParam, lParam);
            return 0;
        };

        case WM_KEYUP:
        {
            gApp->native_keyboard_event(uMsg, wParam, lParam);
            return 0;
        };

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        {
            int mouse_x = LOWORD(lParam);
            int mouse_y = HIWORD(lParam);

            gApp->mMouseOffsetStartX = ((float)mouse_x / (float)gApp->mAppWindowWidth);
            gApp->mMouseOffsetStartY = ((float)mouse_y / (float)gApp->mAppWindowHeight);
            gApp->mXRotationStart = gApp->mXRotation;
            gApp->mYRotationStart = gApp->mYRotation;

            if (!(wParam & MK_SHIFT))
            {
                SetFocus(hWnd);

                int texture_x, texture_y, texture_face;
                gApp->windowPosToTexturePos(mouse_x, mouse_y, texture_x, texture_y, texture_face);
                gApp->on_left_mouse_down(texture_x, texture_y, texture_face);
            }

            return 0;
        };

        case WM_LBUTTONUP:
        {
            int mouse_x = LOWORD(lParam);
            int mouse_y = HIWORD(lParam);

            gApp->mXRotation = gApp->mXRotationStart;
            gApp->mYRotation = gApp->mYRotationStart;

            const bool pick_only = false;
            gApp->draw(pick_only);

            if (!(wParam & MK_SHIFT))
            {
                int texture_x, texture_y, texture_face;
                gApp->windowPosToTexturePos(mouse_x, mouse_y, texture_x, texture_y, texture_face);
                gApp->on_left_mouse_up(texture_x, texture_y, texture_face);
            }

            return 0;
        };

        case WM_MOUSEMOVE:
        {
            int mouse_x = LOWORD(lParam);
            int mouse_y = HIWORD(lParam);

            gApp->mMouseDragCube = (wParam & MK_LBUTTON) && (wParam & MK_SHIFT);
            gApp->mMouseOffsetX = ((float)mouse_x / (float)gApp->mAppWindowWidth);
            gApp->mMouseOffsetY = ((float)mouse_y / (float)gApp->mAppWindowHeight);

            if (!gApp->mMouseDragCube)
            {
                int texture_x, texture_y, texture_face;
                gApp->windowPosToTexturePos(mouse_x, mouse_y, texture_x, texture_y, texture_face);
                gApp->on_mouse_move(texture_x, texture_y, texture_face, (bool)(wParam & MK_LBUTTON));
            }

            return 0;
        };

        case WM_MOUSEWHEEL:
        {
            if (wParam & MK_SHIFT)
            {
                gApp->mCameraDist += (GLfloat)((short)HIWORD(wParam) / 120) / 10.0f;
            }
            else
            {
                // location of mouse cursor when wheel was moved
                int mouse_x = LOWORD(lParam);
                int mouse_y = HIWORD(lParam);

                // use the mouse position to get the position on a face in texture
                int texture_x, texture_y, texture_face;
                gApp->windowPosToTexturePos(mouse_x, mouse_y, texture_x, texture_y, texture_face);

                // synthesize a virtual mouse wheel event (clamp X delta for now)
                int delta_x = 0;
                int delta_y = GET_WHEEL_DELTA_WPARAM(wParam);
                gApp->on_mouse_wheel(texture_x, texture_y, delta_x, delta_y);
            }
            return 0;
        };

        case WM_CLOSE:
        {
            gApp->request_exit();
            return 0;
        }

        case WM_SIZE:
        {
            int new_width = LOWORD(lParam);
            int new_height = HIWORD(lParam);
            gApp->resize_window(new_width, new_height);

            MoveWindow(gApp->getURLHandle(), 0, 0, LOWORD(lParam), 16, TRUE);

            return 0;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////
//
const std::string app::getHomePageURL()
{
    // You can override default home page by adding --homepage="http://example.com" to command line
    if (mHomePageURL.length() != 0)
    {
        return mHomePageURL;
    }

    const std::string default_homepage_url("https://sl-viewer-media-system.s3.amazonaws.com/index.html");

    return default_homepage_url;
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::initConsole()
{
    AllocConsole();
    freopen_s(&mConsole, "CON", "w", stdout);
    freopen_s(&mConsole, "CON", "w", stderr);
}

/////////////////////////////////////////////////////////////////////////////////
//
void app::closeConsole()
{
    fclose(mConsole);
    FreeConsole();
}

/////////////////////////////////////////////////////////////////////////////////
//
bool app::isAllFinished()
{
    return mAllFinished;
}

/////////////////////////////////////////////////////////////////////////////////
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    gApp = new app();

    gApp->initConsole();

    // set from command line args
    if (__argc == 2)
    {
        std::string argv1 = std::string(__argv[1]);
        std::string opt("--homepage");
        if (argv1.find(opt) != std::string::npos)
        {
            std::string url = argv1.substr(opt.length() + 1, argv1.length() - opt.length() - 1);

            std::cout << "URL is " << url << std::endl;
            gApp->set_homepage_url(url);
        }
    }

    std::cout << gApp->get_title() << std::endl;

    std::cout << "Instructions" << std::endl << std::endl;
    std::cout << "- Interact with the web page normally" << std::endl;
    std::cout << "- Hold the shift key and drag mouse to move cube" << std::endl;
    std::cout << "- Hold the shift key and use mouse wheel to zoom cube" << std::endl;
    std::cout << "- Select features to try from the menu" << std::endl;
    std::cout << "- Press ESC key to exit or close normally" << std::endl;

    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wc.lpfnWndProc = (WNDPROC)window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    wc.lpszClassName = gApp->gClassName;
    RegisterClass(&wc);


    RECT window_rect;
    SetRect(&window_rect, 0, 50, gApp->mAppWindowWidth, gApp->mAppWindowHeight + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYMENU));

    DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    AdjustWindowRectEx(&window_rect, style, FALSE, ex_style);

    HWND hWnd = CreateWindowEx(ex_style,
                               gApp->gClassName,
                               gApp->get_title().c_str(),
                               style,
                               gApp->mAppWindowPosX, gApp->mAppWindowPosY,
                               window_rect.right - window_rect.left,
                               window_rect.bottom - window_rect.top,
                               nullptr, nullptr, hInstance, nullptr);

    static  PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
        32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
    };

    HDC hDC = GetDC(hWnd);

    GLuint pixel_format = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pixel_format, &pfd);

    HGLRC hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);

    gApp->init();

    gApp->resize_window(gApp->mAppWindowWidth, gApp->mAppWindowHeight);

    MSG msg;

    SetCursor(LoadCursor(nullptr, IDC_ARROW));

    do
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            gApp->update();
            SwapBuffers(hDC);
        }
    }
    while (gApp->isAllFinished() == false);

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
    DestroyWindow(hWnd);
    UnregisterClass(gApp->gClassName, hInstance);

    gApp->closeConsole();

    gApp->shutdown();

    return 0;
}
