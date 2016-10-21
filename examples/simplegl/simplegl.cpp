/*
@brief Dullahan - a headless browser rendering engine
based around the Chromium Embedded Framework

Example: a very (very) simple OpenGL app using GLUT

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
#include <string>
#include <functional>

#include "freeglut.h"
#include "dullahan.h"

dullahan* gDullahan;
const GLuint gTextureWidth = 1024;
const GLuint gTextureHeight = 1024;
GLuint gAppWindowWidth = gTextureWidth;
GLuint gAppWindowHeight = gTextureHeight;

////////////////////////////////////////////////////////////////////////////////
//
void glutResize(int width, int height)
{
    gAppWindowWidth = width;
    gAppWindowHeight = height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, width, height);
    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

////////////////////////////////////////////////////////////////////////////////
//
void glutDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2d(gAppWindowWidth, 0);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2d(0, 0);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2d(0, gAppWindowHeight);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2d(gAppWindowWidth, gAppWindowHeight);
    glEnd();

    glutSwapBuffers();
}

////////////////////////////////////////////////////////////////////////////////
//
void glutIdle()
{
    gDullahan->update();

    glutPostRedisplay();
}

////////////////////////////////////////////////////////////////////////////////
//
void glutMouseButton(int button, int state, int x, int y)
{
    int scaled_x = x * gTextureWidth / gAppWindowWidth;
    int scaled_y = y * gTextureHeight / gAppWindowHeight;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            gDullahan->setFocus();
            gDullahan->mouseButton(dullahan::MB_MOUSE_BUTTON_LEFT, dullahan::ME_MOUSE_DOWN, scaled_x, scaled_y);
        }
        else if (state == GLUT_UP)
        {
            gDullahan->setFocus();
            gDullahan->mouseButton(dullahan::MB_MOUSE_BUTTON_LEFT, dullahan::ME_MOUSE_UP, scaled_x, scaled_y);
        }
    }

    glutPostRedisplay();
}

////////////////////////////////////////////////////////////////////////////////
//
void glutMouseMove(int x, int y)
{
    int scaled_x = x * gTextureWidth / gAppWindowWidth;
    int scaled_y = y * gTextureHeight / gAppWindowHeight;

    gDullahan->mouseMove(scaled_x, scaled_y);
    glutPostRedisplay();
}

/////////////////////////////////////////////////////////////////////////////////
//
void onPageChangedCallback(const unsigned char* pixels, int x, int y, const int width, const int height, bool is_popup)
{
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    x, y,
                    width, height,
                    GL_BGRA_EXT,
                    GL_UNSIGNED_BYTE,
                    pixels);
}

////////////////////////////////////////////////////////////////////////////////
//
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(80, 0);
    glutInitWindowSize(gAppWindowWidth, gAppWindowHeight);

    glutCreateWindow("SimpleGL");

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutIdle);
    glutMouseFunc(glutMouseButton);
    glutPassiveMotionFunc(glutMouseMove);
    glutMotionFunc(glutMouseMove);
    glutReshapeFunc(glutResize);

    GLuint app_texture;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &app_texture);
    glBindTexture(GL_TEXTURE_2D, app_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gTextureWidth, gTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glViewport(0, 0, gTextureWidth, gTextureHeight);
    glOrtho(0.0f, gTextureWidth, gTextureHeight, 0.0f, -1.0f, 1.0f);

    gDullahan = new dullahan();

    std::cout << "Version: " << gDullahan->composite_version() << std::endl;

    gDullahan->setOnPageChangedCallback(std::bind(&onPageChangedCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

    dullahan::dullahan_settings settings;
    settings.cache_enabled = true;
    settings.cache_path = ".\\cache";
    settings.cookie_store_path = ".\\cookies";
    settings.cookies_enabled = true;
    settings.initial_width = gTextureWidth;
    settings.initial_height = gTextureHeight;
    settings.user_agent_substring = gDullahan->makeCompatibleUserAgentString("console");

    bool result = gDullahan->init(settings);
    if (result)
    {
        std::string url = "file:///dullahan_test_urls.html?simple=y";
        if (2 == argc)
        {
            url = std::string(argv[1]);
        }

        gDullahan->navigate(url);
    }

    glutMainLoop();

    gDullahan->shutdown();

    std::cout << "Exiting cleanly" << std::endl;
}
