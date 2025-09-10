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

#include <iostream>
#include <functional>
#include <filesystem>
#include <random>
#include <sstream>

#include "opengl-example.h"

#include "dullahan.h"

void errorCallback(int error, const char* description)
{
    std::cerr << "GLFW error: (" << error << ") - " << description << std::endl;
}

openglExample::openglExample() :
    mDullahan(nullptr),
    mTextureId(0),
    mPickTextureId(0),
    mWindow(nullptr),
    mShowAbout(false)
{
}

void openglExample::resizeCallback(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const double pi = 3.1415926;
    const double near_plane = 0.1f;
    const double far_plane = 100.0f;
    const double fov = 60.0;
    double frustum_height = tan(fov / 360.0 * pi) * near_plane;
    double frustum_width = frustum_height * (double)width / (double)height;
    glFrustum(-frustum_width, frustum_width, -frustum_height, frustum_height, near_plane, far_plane);

    glMatrixMode(GL_MODELVIEW);

    if (mDullahan)
    {
        mDullahan->setSize(width, height);
    }
}

void openglExample::handleKeyEvent(int key, int scancode, int action, int mods)
{
    // keep the ESC key to exit as well as File -> Quit since it's useful
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
            mDullahan->requestExit();
        }
    }
}

void openglExample::mouseButtonCallback(int button, int action, int mods)
{
    int width;
    int height;
    glfwGetWindowSize(mWindow, &width, &height);

    double xpos;
    double ypos;
    glfwGetCursorPos(mWindow, &xpos, &ypos);

    if (glfwGetKey(mWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        mMouseOffsetStartX = xpos / (double)width;

        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                mMouseOffsetStartY = ypos / (double)height;
                mXRotationStart = mXRotation;
                mYRotationStart = mYRotation;
            }
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS)
            {
                mMouseOffsetStartY = ((double)height - ypos) / (double)height;
                mXPanStart = mXPan;
                mYPanStart = mYPan;
            }
        }
    }
    else
    {
        int tx;
        int ty;
        // Draw pick texture, save mouse location then draw browser output
        if (draw(&tx, &ty))
        {
            mDullahan->mouseButton(
                dullahan::MB_MOUSE_BUTTON_LEFT,
                action == GLFW_PRESS ? dullahan::ME_MOUSE_DOWN : dullahan::ME_MOUSE_UP,
                tx, ty);
        }
    }
}

void openglExample::mouseMoveCallback(double xpos, double ypos)
{
    int width;
    int height;
    glfwGetWindowSize(mWindow, &width, &height);

    if (glfwGetKey(mWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        mMouseOffsetX = xpos / (double)width;

        if (glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            mMouseOffsetY = ypos / (double)height;
            mYRotation = mYRotationStart + (mMouseOffsetX - mMouseOffsetStartX) * 360.0f;
            mXRotation = mXRotationStart + (mMouseOffsetY - mMouseOffsetStartY) * 360.0f;
        }
        if (glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            mMouseOffsetY = ((double)height - ypos) / (double)height;
            mXPan = mXPanStart + (mMouseOffsetX - mMouseOffsetStartX) * 5.0f;
            mYPan = mYPanStart + (mMouseOffsetY - mMouseOffsetStartY) * 5.0f;
        }
    }
    else
    {
        int tx;
        int ty;
        // Draw pick texture, save mouse location then draw browser output
        if (draw(&tx, &ty))
        {
            mDullahan->mouseMove(tx, ty);
        }
    }
}

void openglExample::mouseScrollCallback(double xoffset, double yoffset)
{
    if (glfwGetKey(mWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        mCameraDist += yoffset / mZoomSensitivity;

        if (mCameraDist < mZoomMin)
        {
            mCameraDist = mZoomMin;
        }
        if (mCameraDist > mZoomMax)
        {
            mCameraDist = mZoomMax;
        }
    }
    else
    {
        int tx;
        int ty;
        // Draw pick texture, save mouse location then draw browser output
        if (draw(&tx, &ty))
        {
            mDullahan->mouseWheel(tx, ty, (int)xoffset, (int)(yoffset * 20));
        }
    }

}

#if defined(WIN32)
// Windows subclass procedure for handling keyboard events using native
// Windows messages and parameters which is what Dullahan requires.
LRESULT CALLBACK openglExample::keyEventSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_CHAR || uMsg == WM_KEYDOWN || uMsg == WM_KEYUP)
    {
        openglExample* parent = (openglExample*)dwRefData;
        parent->mDullahan->nativeKeyboardEventWin(uMsg, (uint32_t)wParam, lParam);
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
#endif

bool openglExample::init()
{
    if (! glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, mWindowTitle.c_str(), nullptr, nullptr);
    if (! mWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // store this to so the static callbacks can get to an instance
    glfwSetWindowUserPointer(mWindow, this);

    // Create a Windows subclass procedure for handling keyboard events using
    // native Windows messages and parameters which is what Dullahan requires.
    #if defined(WIN32)
    HWND hwnd = glfwGetWin32Window(mWindow);
    SetWindowSubclass(hwnd, keyEventSubClassProc, 0x01, (DWORD_PTR)this);
    #endif

    glfwSetErrorCallback(errorCallback);
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glfwSwapInterval(1);

    glfwSetKeyCallback(mWindow, keyCallbackStatic);
    glfwSetMouseButtonCallback(mWindow, mouseButtonCallbackStatic);
    glfwSetCursorPosCallback(mWindow, mouseMoveCallbackStatic);
    glfwSetScrollCallback(mWindow, mouseScrollCallbackStatic);

    int width, height;
    glfwSetFramebufferSizeCallback(mWindow, resizeCallbackStatic);
    glfwGetFramebufferSize(mWindow, &width, &height);
    resizeCallback(width, height);

    // Texture used to display browser output on the quad
    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);

    // Generates the picking texture - each pixel in the texture
    // holds the coordinates of its location for mouse picking
    generatePickTexture();

    initUI();

    mDullahan = new dullahan();

    // Modern way of generating random numbers - need this for making the CEF root_cache_folder unique
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(100000, 999999);
    int random_number = distrib(gen);

    // As of CEF 139, the root cache folder must be unique and
    // an absolute path - std::Filesystem to the rescue
    std::filesystem::path root_cache_path = std::filesystem::absolute("./opengl-example-profile") / std::to_string(random_number);
    std::filesystem::path log_path = root_cache_path / "opengl-example-cef.log";

    dullahan::dullahan_settings settings;
    settings.log_file = log_path.string();
    settings.root_cache_path = root_cache_path.string();
    settings.initial_height = mTextureWidth;
    settings.initial_width = mTextureHeight;
    settings.disable_gpu = false;
#ifdef __APPLE__
    settings.use_mock_keychain = true;
#endif

    bool result = mDullahan->init(settings);
    if (result)
    {
        mDullahan->setOnPageChangedCallback(std::bind(&openglExample::onPageChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        mDullahan->setOnRequestExitCallback(std::bind(&openglExample::onRequestExitCallback, this));

        mDullahan->navigate(mHomeUrl);
    }

    return true;
}

void openglExample::generatePickTexture()
{
    unsigned char* pick_texture_pixels = new unsigned char[mTextureWidth * mTextureHeight * mTextureDepth];

    glGenTextures(1, (GLuint*)(&mPickTextureId));
    for (size_t y = 0; y < mTextureHeight; ++y)
    {
        for (size_t x = 0; x < mTextureWidth; ++x)
        {
            // use use 12 bits for x and y position and some 'random' value for the ID
            unsigned long mask = ((unsigned long)x << 12) | ((unsigned long)y);
            pick_texture_pixels[y * mTextureWidth * mTextureDepth + x * mTextureDepth + 0] = mask & 0xff;
            pick_texture_pixels[y * mTextureWidth * mTextureDepth + x * mTextureDepth + 1] = (mask >> 8) & 0xff;
            pick_texture_pixels[y * mTextureWidth * mTextureDepth + x * mTextureDepth + 2] = (mask >> 16) & 0xff;
            pick_texture_pixels[y * mTextureWidth * mTextureDepth + x * mTextureDepth + 3] = mBrowserId;
        };
    }

    glBindTexture(GL_TEXTURE_2D, (GLuint)mPickTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)mTextureWidth, (GLsizei)mTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pick_texture_pixels);

    delete [] pick_texture_pixels;
}

// Converts the position of the mouse on the screen into
// the position within the quad - return true if hit and
// location X, Y (0..texture size) is passed back
bool openglExample::mousePosToTexturePos(int* tx, int* ty)
{
    int width;
    int height;
    glfwGetWindowSize(mWindow, &width, &height);

    double xpos;
    double ypos;
    glfwGetCursorPos(mWindow, &xpos, &ypos);

    unsigned char pick_pixel_color[4];
    glBindTexture(GL_TEXTURE_2D, (GLuint)mPickTextureId);
    glReadPixels((int)xpos, height - (int)ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pick_pixel_color);

    if (pick_pixel_color[3] != mBrowserId)
    {
        *tx = -1;
        *ty = -1;
        return false;
    }

    *tx = (pick_pixel_color[2] << 4) | (pick_pixel_color[1] >> 4);
    *ty = ((pick_pixel_color[1] & 0x0f) << 8) | (pick_pixel_color[0]);

    return true;
}

bool openglExample::draw(int* tx, int* ty)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    glTranslatef((GLfloat)mXPan, (GLfloat)mYPan, (GLfloat)mCameraDist);

    glRotatef((GLfloat)mXRotation, 1.0f, 0.0f, 0.0f);
    glRotatef((GLfloat)mYRotation, 0.0f, 1.0f, 0.0f);
    glRotatef(0.0f, 0.0f, 0.0f, 1.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);

    bool hit_browser = false;

    // Draw the quad for both picking and displaying
    // browser output - only difference is texture in play
    auto draw_quad = []()
    {
        GLfloat pos_x = 0;
        GLfloat pos_y = 0;
        GLfloat pos_z = 0;

        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(pos_x - (GLfloat)1.0, pos_y - (GLfloat)1.0, pos_z);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(pos_x + (GLfloat)1.0, pos_y - (GLfloat)1.0, pos_z);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(pos_x + (GLfloat)1.0, pos_y + (GLfloat)1.0, pos_z);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(pos_x - (GLfloat)1.0, pos_y + (GLfloat)1.0, pos_z);
        glEnd();

    };

    // Only send back texture X/Y of mouse when picking
    if (tx != nullptr && ty != nullptr)
    {
        // draw picking texture
        glBindTexture(GL_TEXTURE_2D, (GLuint)mPickTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        draw_quad();

        // determine location of mouse cursor
        hit_browser = mousePosToTexturePos(tx, ty);
    }

    // Now draw again with browser output texture
    glBindTexture(GL_TEXTURE_2D, (GLuint)mTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    draw_quad();


    return hit_browser;
}

// Triggered when browser page content changes
void openglExample::onPageChanged(const unsigned char* pixels, int x, int y, const int width, const int height)
{
    // This should always be true but test just in case
    if (width == mTextureWidth && height == mTextureHeight)
    {
        glBindTexture(GL_TEXTURE_2D, (GLuint)mTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)mTextureWidth, (GLsizei)mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
    }
}

// Triggered by Dullahan when cleanup is complete and it's okay to exit
void openglExample::onRequestExitCallback()
{
    glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
}

void openglExample::initUI()
{
    IMGUI_CHECKVERSION();
    ImGui:: CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    io.FontGlobalScale = 1.2f;
    ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
    ImGui_ImplOpenGL2_Init();
}

void openglExample::updateUI()
{
    // main host for UI - URL and bookmarks drop-down
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Turn off window decoaration - we don't want
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, main_viewport->WorkPos.y), 0);
    ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, ImGui::GetFrameHeight() * 3), 0);

    // Write menu bar and associated actions
    ImGui::Begin("##ui", NULL, window_flags);
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Quit"))
            {
                glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Actions"))
        {
            if (ImGui::MenuItem("Go Home"))
            {
                mDullahan->navigate(mHomeUrl);
            }
            if (ImGui::MenuItem("Dev Console"))
            {
                mDullahan->showDevTools();
            }

            if (ImGui::BeginMenu("Zoom"))
            {
                if (ImGui::MenuItem("0.5"))
                {
                    mDullahan->setPageZoom(0.5);
                }
                if (ImGui::MenuItem("1.0"))
                {
                    mDullahan->setPageZoom(1.0);
                }
                if (ImGui::MenuItem("2.0"))
                {
                    mDullahan->setPageZoom(2.0);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                mShowAbout = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (mShowAbout)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
        if (ImGui::Begin(mWindowTitle.c_str(), &mShowAbout, ImGuiWindowFlags_NoResize))
        {
            std::ostringstream ss;

            ss << "Instructions:";
            ss << std::endl << std::endl;
            ss << "Hold Control key and move/pan/zoom using the";
            ss << std::endl;
            ss << "mouse left, right buttons and scroll wheel.";
            ss << std::endl << std::endl;
            ss << "Interact with the page using the mouse";
            ss << std::endl;
            ss << "left mouse button and scroll wheel.";
            ss << std::endl << std::endl;
            ss << "App version: " << mAppVersionStr;
            ss << std::endl << std::endl;
            ss << "Dullahan version: " << mDullahan->dullahan_version(false);
            ss << std::endl << std::endl;
            ss << "CEF version: " << mDullahan->dullahan_cef_version(false);
            ss << std::endl << std::endl;

            ImGui::Text(ss.str().c_str());
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    // Write freeform URL entry bar
    ImGui::SetNextItemWidth(main_viewport->Size.x);
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(64, 64, 100, 255));
    static char url_buffer[4096];
    if (ImGui::InputTextWithHint("UrlInout", "Enter a URL",
                                 url_buffer,
                                 IM_ARRAYSIZE(url_buffer),
                                 ImGuiInputTextFlags_EnterReturnsTrue))
    {
        mDullahan->navigate(url_buffer);
    }
    ImGui::PopStyleColor();

    // Write bookmarks bar - a good place to add useful or interesting bookmarks
    const char* items[] =
    {
        "chrome://version",
        "https://sl-viewer-media-system.s3.amazonaws.com/bookmarks/index.html",
        "https://viewer-login.agni.lindenlab.com/",
        "https://secondlife.com"
    };
    static const char* current_item = "Select a bookmark";
    ImGui::SetNextItemWidth(main_viewport->Size.x);
    ImGui::SetCursorPos(ImVec2(0, ImGui::GetFrameHeight()));
    if (ImGui::BeginCombo("##Bookmarks", current_item))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (current_item == items[n]);
            if (ImGui::Selectable(items[n], is_selected))
            {
                current_item = items[n];
                mDullahan->navigate(current_item);
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void openglExample::resetUI()
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool openglExample::run()
{
    while (! glfwWindowShouldClose(mWindow))
    {
        if (mDullahan)
        {
            mDullahan->update();
        }

        // Drawe browser output but do not pick
        draw(nullptr, nullptr);

        updateUI();

        glfwSwapBuffers(mWindow);

        glfwPollEvents();
    }
    return true;
}

bool openglExample::reset()
{
    resetUI();

    glfwDestroyWindow(mWindow);

    glfwTerminate();

    return true;
}

int main(int argc, char* argv[])
{
    openglExample* app = new openglExample;

    app->init();

    app->run();

    app->reset();

    exit(EXIT_SUCCESS);
}
