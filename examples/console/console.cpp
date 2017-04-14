/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

           Example: capture a web page to image file and a PDF with a console app

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
#include <functional>
#include <string>
#include <ctime>

#include <fstream>
#include <windows.h>


#include "dullahan.h"

dullahan* headless_browser;

unsigned char* gPixels = 0;
int gWidth = 0;
int gHeight = 0;
time_t gPageFinishLoadTime = time(nullptr);
time_t gLastChangeTime = time(nullptr);

void writeBMPImage(const std::string& filename,
                   unsigned char* pixels,
                   int image_width, int image_height)
{
    std::cout << std::endl << "Writing output image (BMP) (" << image_width << " x " << image_height << ") to " << filename << std::endl;

    std::ofstream img_stream(filename.c_str(), std::ios::binary | std::ios::out);
    if (img_stream)
    {
        unsigned char file[14] =
        {
            'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 40 + 14, 0, 0, 0
        };
        unsigned char info[40] =
        {
            40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x13, 0x0B, 0, 0, 0x13, 0x0B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        };
        int size_data = image_width * image_height * 3;
        int size_all = size_data + sizeof(file) + sizeof(info);

        file[2] = (unsigned char)(size_all);
        file[3] = (unsigned char)(size_all >> 8);
        file[4] = (unsigned char)(size_all >> 16);
        file[5] = (unsigned char)(size_all >> 24);

        info[4] = (unsigned char)(image_width);
        info[5] = (unsigned char)(image_width >> 8);
        info[6] = (unsigned char)(image_width >> 16);
        info[7] = (unsigned char)(image_width >> 24);

        info[8] = (unsigned char)(-image_height);
        info[9] = (unsigned char)(-image_height >> 8);
        info[10] = (unsigned char)(-image_height >> 16);
        info[11] = (unsigned char)(-image_height >> 24);

        info[20] = (unsigned char)(size_data);
        info[21] = (unsigned char)(size_data >> 8);
        info[22] = (unsigned char)(size_data >> 16);
        info[23] = (unsigned char)(size_data >> 24);

        img_stream.write((char*)file, sizeof(file));
        img_stream.write((char*)info, sizeof(info));

        const int image_depth = 4;
        for (int i = 0; i < image_width * image_height * image_depth; i += image_depth)
        {
            const unsigned char red = *(pixels + i + 2);
            const unsigned char green = *(pixels + i + 1);
            const unsigned char blue = *(pixels + i + 0);

            img_stream << blue;
            img_stream << green;
            img_stream << red;
        }

        img_stream.close();
    }
}

void writePDF(std::string filename)
{
    static bool written = false;

    if (!written)
    {
        std::cout << std::endl << "Writing output PDF: " << filename << std::endl;
        headless_browser->printToPDF(filename);
        headless_browser->update();
    }

    written = true;
}

void onPageChanged(const unsigned char* pixels, int x, int y, int width, int height)
{
    std::cout << "# ";

    if (width != gWidth || height != gHeight)
    {
        delete gPixels;
        gPixels = new unsigned char[width * height * 4];
        gWidth = width;
        gHeight = height;
    }

    memcpy(gPixels, (unsigned char*)pixels, gWidth * 4 * gHeight);

    gLastChangeTime = time(nullptr);
}

void onLoadStart()
{
    std::cout << std::endl << "Page load started" << std::endl;
    gLastChangeTime = time(nullptr);
}

void onLoadEnd(int code)
{
    std::cout << std::endl << "Page load ended with code " << code << std::endl;
    gLastChangeTime = time(nullptr);
    gPageFinishLoadTime = time(nullptr);
}

void onRequestExit()
{
    std::cout << std::endl << "Exit requested - shutting down and exiting" << std::endl;

    PostQuitMessage(0L);
}

int main(int argc, char* argv[])
{
    std::string url = "https://news.google.com";
    if (argc == 2)
    {
        url = std::string(argv[1]);
    }

    headless_browser = new dullahan();

    std::cout << "Dullahan console test" << std::endl << std::endl;
    std::cout << "Capturing: " << url << std::endl << std::endl;
    std::cout << "CEF Version: " << headless_browser->dullahan_cef_version(true) << std::endl;
    std::cout << "Chrome Version: " << headless_browser->dullahan_chrome_version(true) << std::endl;
    std::cout << "Dullahan version: " << headless_browser->dullahan_version(true) << std::endl << std::endl;

    headless_browser->setOnPageChangedCallback(std::bind(onPageChanged,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));
    headless_browser->setOnLoadStartCallback(std::bind(onLoadStart));
    headless_browser->setOnLoadEndCallback(std::bind(onLoadEnd,
                                           std::placeholders::_1));
    headless_browser->setOnRequestExitCallback(std::bind(onRequestExit));

    dullahan::dullahan_settings settings;
    settings.initial_width = 2048;
    settings.initial_height = 2048;
    settings.javascript_enabled = true;
    settings.cookies_enabled = true;
    settings.user_agent_substring = "Console Test";
    settings.accept_language_list = "en-us";

    headless_browser->init(settings);

    headless_browser->navigate(url);

    MSG msg;
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            headless_browser->update();

            const time_t max_elapsed_change_seconds = 2;
            if (time(nullptr) - gLastChangeTime > max_elapsed_change_seconds)
            {
                writePDF("output.pdf");

                headless_browser->requestExit();
            }

            const time_t max_elapsed_since_loaded_seconds = 10;
            if (time(nullptr) - gPageFinishLoadTime > max_elapsed_since_loaded_seconds)
            {
                writePDF("output.pdf");

                headless_browser->requestExit();
            }
        }
    }
    while (msg.message != WM_QUIT);

    writeBMPImage("output.bmp", gPixels, gWidth, gHeight);

    headless_browser->shutdown();
}
