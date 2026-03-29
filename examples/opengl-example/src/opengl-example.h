/**
    @brief  Dullahan OpenGL Example application

            Cross platform example for illustration and standalone testing
            of Dullahan features. Renders output to an OpenGL 2.1 quad
            and allows interaction using the mouse.

    @author Callum Prentice - August 2025

    Copyright (c) 2025, Linden Research, Inc.

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
**/

#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include <glad/glad.h>
#if defined(WIN32)
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <commctrl.h>
#include <d3d11_1.h>
#else
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include "dullahan.h"

#ifdef WIN32
// WGL_NV_DX_interop2 function typedefs
typedef HANDLE (WINAPI *PFNWGLDXOPENDEVICENVPROC)(void*);
typedef BOOL (WINAPI *PFNWGLDXCLOSEDEVICENVPROC)(HANDLE);
typedef HANDLE (WINAPI *PFNWGLDXREGISTEROBJECTNVPROC)(HANDLE, void*, GLuint, GLenum, GLenum);
typedef BOOL (WINAPI *PFNWGLDXUNREGISTEROBJECTNVPROC)(HANDLE, HANDLE);
typedef BOOL (WINAPI *PFNWGLDXLOCKOBJECTSNVPROC)(HANDLE, GLint, HANDLE*);
typedef BOOL (WINAPI *PFNWGLDXUNLOCKOBJECTSNVPROC)(HANDLE, GLint, HANDLE*);

#define WGL_ACCESS_READ_ONLY_NV 0x0000
#endif

class openglExample
{
    public:
        openglExample();

        bool init();
        bool run();
        bool draw(int* tx, int* ty);
        void resizeCallback(int width, int height);
        void handleKeyEvent(int key, int scancode, int action, int mods);
        void mouseButtonCallback(int button, int action, int mods);
        void mouseMoveCallback(double xpos, double ypos);
        void mouseScrollCallback(double xoffset, double yoffset);
        void initUI();
        void updateUI();
        void resetUI();
        bool reset();

        // callbacks
#ifdef WIN32
        void onAcceleratedPageChanged(void* handle, const std::vector<dullahan::dullahan_rect>& dirty_rects);
#else
        void onPageChanged(const unsigned char* pixels, int x, int y, const int width, const int height);
#endif
        void onRequestExitCallback();
        std::string onJStoCPPMsgCallback(const std::string id, const std::string msg);

    private:
        GLFWwindow* mWindow;
        const std::string mWindowTitle = "Dullahan OpenGL Example";
        const std::string mAppVersionStr = "0.0.1";
        const std::string mHomeUrl = "https://sl-viewer-media-system.s3.amazonaws.com/bookmarks/index.html";
        const int mWindowWidth = 1280;
        const int mWindowHeight = 1280;
        bool mShowAbout;
        double mCameraDist = -2.1;
        double mMouseOffsetX = 0.0;
        double mMouseOffsetY = 0.0;
        double mMouseOffsetStartX = 0.0;
        double mMouseOffsetStartY = 0.0;
        double mXRotationStart = 0;
        double mXRotation = 0;
        double mYRotationStart = 0;
        double mYRotation = 0;
        double mXPanStart = 0;
        double mXPan = 0;
        double mYPanStart = 0;
        double mYPan = 0;
        const double mZoomSensitivity = 10.0;
        const double mZoomMin = -20.0;
        const double mZoomMax = -0.2;
        GLuint mPickTextureId;
        const int mPickTextureWidth = 1024;
        const int mPickTextureHeight = 1024;
        const int mPickTextureDepth = 4;
        GLuint mTextureId;
        int mTextureWidth = 1024;
        int mTextureHeight = 1024;
        const int mTextureDepth = 4;
        const unsigned char mBrowserId = 23;
        dullahan* mDullahan;

#ifdef WIN32
        // D3D11/GL interop for accelerated paint
        ID3D11Device1* mD3DDevice = nullptr;
        ID3D11DeviceContext* mD3DContext = nullptr;
        HANDLE mInteropDevice = nullptr;
        HANDLE mInteropObject = nullptr;
        ID3D11Texture2D* mSharedTexture = nullptr;
        ID3D11Texture2D* mLocalTexture = nullptr;
        int mLocalTextureWidth = 0;
        int mLocalTextureHeight = 0;

        PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV = nullptr;
        PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV = nullptr;
        PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV = nullptr;
        PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV = nullptr;
        PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV = nullptr;
        PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV = nullptr;
#endif

        void generatePickTexture();
        bool mousePosToTexturePos(int* tx, int* ty);
        void resizeBrowser(int width, int height);

        // Used to marshall static function callbacks to a instance of the app class
        static void resizeCallbackStatic(GLFWwindow* window, int width, int height)
        {
            static_cast<openglExample*>(glfwGetWindowUserPointer(window))->resizeCallback(width, height);
        }
        static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            static_cast<openglExample*>(glfwGetWindowUserPointer(window))->handleKeyEvent(key, scancode, action, mods);
        }
        static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods)
        {
            static_cast<openglExample*>(glfwGetWindowUserPointer(window))->mouseButtonCallback(button, action, mods);
        }
        static void mouseMoveCallbackStatic(GLFWwindow* window, double xpos, double ypos)
        {
            static_cast<openglExample*>(glfwGetWindowUserPointer(window))->mouseMoveCallback(xpos, ypos);
        }
        static void mouseScrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset)
        {
            static_cast<openglExample*>(glfwGetWindowUserPointer(window))->mouseScrollCallback(xoffset, yoffset);
        }

        #if defined(WIN32)
        static LRESULT CALLBACK keyEventSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
        #endif
};
